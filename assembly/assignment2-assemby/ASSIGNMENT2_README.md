# Assignment 2: LED Signaling Display

**CSC 230: Computer Architecture and Assembly Language**  
**University of Victoria - Spring 2025**

## Overview

This assignment implements a visual communication system using the Arduino Mega 2560's six-LED array to display encoded messages. Each letter of the alphabet is represented by a unique pattern of lights with variable duration, similar to naval flag signaling or morse code.

**Demo Video**: [Watch the assignment demonstration](https://youtu.be/_tRcKbYSZlY)

## Learning Objectives

- Write and call functions in AVR assembly
- Implement parameter passing using registers and stack
- Use stack frames for function parameters
- Work with return values through registers
- Interface with physical hardware (LED array on Arduino Mega 2560)
- Read data from program memory (pattern lookup tables)
- Understand calling conventions and stack management

## Hardware Requirements

- **Board**: Arduino Mega 2560 (ATmega2560 microcontroller)
- **Components**: 6 LED array on the board
- **Development Environment**: Microchip Studio 7
- **Testing Location**: ECS 249 lab (UVic)

## Files

```
assignment-2/
├── a2-signalling.asm          # Main implementation file
└── README.md                  # This file
```

## Signaling System Design

### Encoding Scheme

Each letter is encoded with two properties:

1. **LED Pattern** (6 bits): Which LEDs are on/off
   - Bit 5 → LED 01 (leftmost)
   - Bit 4 → LED 02
   - Bit 3 → LED 03
   - Bit 2 → LED 04
   - Bit 1 → LED 05
   - Bit 0 → LED 06 (rightmost)

2. **Duration** (2 bits):
   - Bits 7-6 = `11` → Long duration (~1 second)
   - Bits 7-6 = `00` → Short duration (~0.25 seconds)

### Example: "HELLO"

```
Letter  Pattern     Duration  Binary      Hex
------  ----------  --------  ----------  ----
H       ..oo..      Short     00001100    0x0C
E       oooooo      Long      11111111    0xFF
L       o.o.o.      Long      11010101    0xD5
L       o.o.o.      Long      11010101    0xD5
O       .oooo.      Long      11011110    0xDE
```

Legend: `o` = LED on, `.` = LED off

## Implementation Parts

### Part A: `configure_leds`

**Function Signature**:
```assembly
configure_leds:
    ; Parameters: r16 (LED pattern, pass-by-value)
    ; Return value: none
```

**Purpose**: Set the hardware LEDs on/off based on bit pattern in r16.

**Implementation Requirements**:
- Control 6 LEDs using bits 5-0 of r16
- Ignore bits 7-6
- LEDs remain in state until changed by another call
- Directly manipulate hardware I/O ports

**Example**:
```assembly
ldi r16, 0b00100001  ; Turn on LED 01 and LED 06
rcall configure_leds
```

**Hardware Details**:
- Uses PORT and DDR registers for LED control
- Requires initialization of data direction registers
- Maps bit pattern to physical pin addresses

---

### Part B: `fast_leds` and `slow_leds`

**Function Signatures**:
```assembly
fast_leds:
    ; Parameters: r17 (LED pattern, pass-by-value)
    ; Return value: none

slow_leds:
    ; Parameters: r17 (LED pattern, pass-by-value)
    ; Return value: none
```

**Purpose**: Turn on LEDs for a fixed duration, then turn off.

**Implementation Requirements**:
- `fast_leds`: Display pattern for ~0.25 seconds
- `slow_leds`: Display pattern for ~1 second
- Must call `configure_leds` to set LED state
- Must call `delay_short` or `delay_long` for timing
- Turn LEDs off after delay

**Example**:
```assembly
ldi r17, 0b00101010  ; Alternating pattern
rcall slow_leds      ; Show for 1 second, then off
```

---

### Part C: `leds_with_speed`

**Function Signature**:
```assembly
leds_with_speed:
    ; Parameters: 1 byte on stack (pattern + duration, pass-by-value)
    ; Return value: none
```

**Purpose**: Unified LED control using 8-bit encoding (pattern + duration).

**Implementation Requirements**:
- Read parameter from stack
- Bits 7-6: Determine duration
  - `11` → Call `slow_leds`
  - `00` → Call `fast_leds`
- Bits 5-0: LED pattern
- Extract pattern bits and pass to appropriate function

**Stack Parameter Passing**:
```assembly
ldi r16, 0xFF        ; All LEDs on, slow
push r16
rcall leds_with_speed
pop r16              ; Clean up stack
```

**Key Concept**: This function demonstrates stack-based parameter passing, a critical skill for more complex assembly programs.

---

### Part D: `encode_letter`

**Function Signature**:
```assembly
encode_letter:
    ; Parameters: 1 byte on stack (uppercase letter, pass-by-value)
    ; Return value: r25 (encoded byte for leds_with_speed)
```

**Purpose**: Convert an uppercase letter to its LED encoding.

**Implementation Requirements**:
- Read letter from stack
- Search `PATTERNS` table in program memory
- Extract LED pattern (6 characters: "o" or ".")
- Extract duration flag (1 or 2)
- Construct 8-bit encoding in r25

**Pattern Table Format**:
```assembly
PATTERNS:
    .db "A", "..oo..", 1  ; Letter, pattern string, duration
    .db "B", ".o..o.", 2
    ; ... etc
```

**Encoding Algorithm**:
1. Find letter in PATTERNS table
2. Parse pattern string:
   - "o" at position N → Set bit (5-N)
   - "." at position N → Clear bit (5-N)
3. Set bits 7-6 based on duration:
   - Duration = 1 → Set bits 7-6 (`11`)
   - Duration = 2 → Clear bits 7-6 (`00`)
4. Return encoding in r25

**Example**:
```assembly
ldi r21, 'A'
push r21
rcall encode_letter
pop r21
; r25 now contains 0xCC (0b11001100)
```

**Challenge**: Reading from program memory requires `lpm` instruction and Z-pointer manipulation.

---

### Part E: `display_message_signal`

**Function Signature**:
```assembly
display_message_signal:
    ; Parameters: 
    ;   r25 = High byte of message address in program memory
    ;   r24 = Low byte of message address in program memory
    ; Return value: none
```

**Purpose**: Display an entire message by encoding and showing each letter.

**Implementation Requirements**:
- Message address is in program memory (not data memory)
- Messages contain only uppercase letters
- Messages are null-terminated (ASCII 0)
- For each letter:
  1. Read from program memory
  2. Push onto stack
  3. Call `encode_letter`
  4. Push r25 onto stack
  5. Call `leds_with_speed`
  6. Move to next letter
- Stop when null terminator is reached

**Message Format**:
```assembly
WORD05: .db "UVIC", 0, 0
```

**Calling Convention**:
```assembly
ldi r25, HIGH(WORD05 << 1)  ; High byte of address
ldi r24, LOW(WORD05 << 1)   ; Low byte of address
rcall display_message_signal
```

**Note**: The `<< 1` shifts the address because program memory is word-addressed but byte-addressed in the LPM instruction context.

**Algorithm Flow**:
```
1. Load Z-pointer with message address (r25:r24)
2. Loop:
   a. Read byte from program memory using lpm
   b. Check if null (0)
   c. If null, exit
   d. Encode letter
   e. Display encoded pattern
   f. Increment Z-pointer
   g. Repeat
```

---

## Testing Strategy

Each part has a corresponding test routine:

### Test Part A
```assembly
rjmp test_part_a
```
- Tests direct LED control
- Verifies bit-to-LED mapping
- Shows various patterns with delays

### Test Part B
```assembly
rjmp test_part_b
```
- Tests timed LED patterns
- Alternates between fast and slow
- Verifies timing accuracy

### Test Part C
```assembly
rjmp test_part_c
```
- Tests stack parameter passing
- Verifies duration bit decoding
- Shows multiple patterns

### Test Part D
```assembly
rjmp test_part_d
```
- Tests letter encoding
- Displays A, B, C with correct patterns
- Verifies lookup table search

### Test Part E
```assembly
rjmp test_part_e
```
- Tests full message display
- Shows "UVIC" as LED signals
- Integrates all previous functions

**To change tests**: Modify line 48 in `a2-signalling.asm`

---

## AVR Assembly Concepts

### Stack Operations
```assembly
; Pushing parameter
ldi r16, 'A'
push r16

; Inside function: reading parameter
; Stack grows downward, SP points to last pushed byte
in r16, SPL
in r17, SPH
; Add offset to access parameter
```

### Program Memory Access
```assembly
; Load program memory byte to register
ldi ZL, LOW(PATTERNS << 1)
ldi ZH, HIGH(PATTERNS << 1)
lpm r16, Z+  ; Load and post-increment
```

### Function Calling Convention
```assembly
; Caller's responsibility:
1. Push parameters onto stack
2. Call function with rcall
3. Pop parameters after return (clean up stack)

; Callee's responsibility:
1. Preserve registers (push/pop)
2. Access parameters from stack
3. Return value in designated register
4. Return with ret
```

### I/O Port Manipulation
```assembly
; Set data direction (1 = output)
ldi r16, 0xFF
out DDRL, r16

; Write to port
ldi r16, 0b00100001
out PORTL, r16
```

---

## Performance Characteristics

| Function | Time Complexity | Stack Usage |
|----------|----------------|-------------|
| configure_leds | O(1) | 0 bytes |
| fast_leds | O(1) + delay | 0 bytes |
| slow_leds | O(1) + delay | 0 bytes |
| leds_with_speed | O(1) + delay | 1 byte |
| encode_letter | O(n) | 1 byte |
| display_message_signal | O(m×n) | 2 bytes |

Where:
- n = number of letters in alphabet (26)
- m = message length

---

## Common Pitfalls & Debugging Tips

### Stack Management
**Wrong**: Forgetting to pop parameters
```assembly
ldi r16, 'A'
push r16
rcall encode_letter
; Missing: pop r16
```

**Correct**: Always balance pushes and pops
```assembly
ldi r16, 'A'
push r16
rcall encode_letter
pop r16
```

### Program Memory Access
 **Wrong**: Using wrong address
```assembly
ldi ZL, LOW(WORD05)      ; Missing << 1
```

 **Correct**: Shift for word addressing
```assembly
ldi ZL, LOW(WORD05 << 1)
```

### Register Preservation
 **Wrong**: Modifying without saving
```assembly
encode_letter:
    ldi r16, 26   ; Overwrites caller's r16!
    ret
```

 **Correct**: Push/pop working registers
```assembly
encode_letter:
    push r16
    ldi r16, 26
    ; ... work ...
    pop r16
    ret
```

### Hardware Initialization
 **Wrong**: Forgetting DDR setup
```assembly
configure_leds:
    out PORTL, r16  ; LEDs won't work!
    ret
```

 **Correct**: Set data direction first
```assembly
; In initialization section:
ldi r16, 0xFF
out DDRL, r16
```

---

##  Pattern Encoding Reference

Full alphabet encoding (from PATTERNS table):

```
Letter  Pattern    Duration  Binary      Hex
A       ..oo..     Long      11001100    0xCC
B       .o..o.     Short     00010010    0x12
C       o.o...     Long      11100000    0xE0
D       .....o     Long      11000001    0xC1
E       oooooo     Long      11111111    0xFF
F       .oooo.     Short     00011110    0x1E
G       oo..oo     Short     00110011    0x33
H       ..oo..     Short     00001100    0x0C
I       .o..o.     Long      11010010    0xD2
J       .....o     Short     00000001    0x01
K       ....oo     Short     00000011    0x03
L       o.o.o.     Long      11010101    0xD5
M       oooooo     Short     00111111    0x3F
N       oo....     Long      11110000    0xF0
O       .oooo.     Long      11011110    0xDE
P       o.oo.o     Long      11101101    0xED
Q       o.oo.o     Short     00101101    0x2D
R       oo..oo     Long      11110011    0xF3
S       ....oo     Long      11000011    0xC3
T       ..oo..     Short     00001100    0x0C
U       o.....     Long      11100000    0xE0
V       o.o.o.     Short     00010101    0x15
W       o.o...     Short     00101000    0x28
X       oo....     Short     00110000    0x30
Y       ..oo..     Short     00001100    0x0C
Z       o.....     Short     00100000    0x20
```


## Skills Demonstrated

### Technical Skills
- **Function design**: Modular code with clear interfaces
- **Parameter passing**: Both register-based and stack-based
- **Hardware interfacing**: Direct I/O port manipulation
- **Memory management**: Stack operations and program memory access
- **Table lookup**: Searching data structures in program memory
- **String processing**: Null-terminated string iteration

### Problem-Solving Skills
- Breaking complex tasks into smaller functions
- Designing reusable code components
- Debugging hardware-software integration
- Managing timing constraints with delays
- Encoding information compactly in bytes

### Assembly Programming Techniques
- Calling conventions and stack frames
- Register allocation and preservation
- Bit manipulation for encoding/decoding
- Pointer arithmetic with Z-register
- Program memory vs data memory access

---

##Key AVR Instructions Used

| Instruction | Purpose | Example |
|-------------|---------|---------|
| `ldi` | Load immediate | `ldi r16, 0xFF` |
| `out` | Output to I/O port | `out PORTL, r16` |
| `in` | Input from I/O port | `in r16, SPL` |
| `push` | Push onto stack | `push r16` |
| `pop` | Pop from stack | `pop r16` |
| `rcall` | Call subroutine | `rcall configure_leds` |
| `ret` | Return from subroutine | `ret` |
| `lpm` | Load program memory | `lpm r16, Z+` |
| `sbrc/sbrs` | Skip if bit clear/set | `sbrc r16, 7` |
| `brne/breq` | Branch if not equal/equal | `brne loop` |

---

##  Running the Project

### Hardware Setup
1. Connect Arduino Mega 2560 to computer via USB
2. Open Microchip Studio 7
3. Create new project for ATmega2560
4. Copy code into project
5. Build (F7)

### Programming the Board
1. **Tools → Device Programming**
2. Select **AVRISP mkII** (or appropriate programmer)
3. Select **ATmega2560**
4. **Read** device signature to verify connection
5. **Program** → Flash → Select your .hex file
6. Click **Program**

### Observing the LEDs
- LEDs are on the Arduino board itself
- Watch the 6-LED array as the program runs
- Each pattern represents a letter
- Duration varies per letter encoding

### Debugging
- Use **Debug → Start Debugging and Break**
- Step through code with F10
- Watch registers in Debug windows
- Set breakpoints at function entries
- Monitor stack pointer (SP) for stack operations

---

##  References

- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [AVR Instruction Set Manual](http://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf)
- CSC 230 Lab #3 Materials (LED control)
- CSC 230 Lecture Notes (Functions, Stack, I/O)

---

**Author**: Armita Darbandi
**Course**: CSC 230 - Spring 2025  
**Institution**: University of Victoria  
**Lab**: ECS 249
