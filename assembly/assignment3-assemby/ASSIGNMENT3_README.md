# Assignment 3: LCD Display and Interrupt-Driven I/O

**CSC 230: Computer Architecture and Assembly Language**  
**University of Victoria - Spring 2025**

## Overview

This assignment implements interactive display systems using the Arduino Mega 2560's 16x2 LCD panel, AVR timers, and interrupt handling. Projects range from simple flashing message signs to complex scrolling displays with keypad interaction, demonstrating real-time embedded systems programming.

##  Learning Objectives

- Configure and use AVR hardware timers
- Write interrupt service routines (ISRs)
- Interface with LCD displays using the HD44780 controller
- Process analog keypad input through ADC
- Implement real-time display updates
- Design modular, interrupt-driven embedded systems
- Work with peripheral devices on Arduino Mega 2560

##  Hardware Requirements

- **Board**: Arduino Mega 2560 (ATmega2560 microcontroller)
- **Display**: 16x2 character LCD with HD44780 controller
- **Input**: DFRobot LCD Keypad Shield (5 buttons: Up, Down, Left, Right, Select)
- **Development Environment**: Microchip Studio 7
- **Testing Location**: ECS 249 lab (UVic)

## 📂 Project Structure

```
assignment-3/
├── display_partA.asm          # Flashing message display
├── display_partB.asm          # Scrolling message display
├── display_partC.asm          # Keypad-controlled scrolling
├── display_partD.asm          # Keypad position display
├── lcd.asm                    # LCD library (provided)
├── LCDdefs.inc               # LCD definitions (provided)
├── lcd_example.asm           # Example code (provided)
└── README.md                 # This file
```

**Note**: Only submit the four `display_part*.asm` files. Do NOT submit the provided LCD library files.

##  LCD Display System

### Hardware Specifications

**Display**: 16 columns × 2 rows character LCD  
**Controller**: Hitachi HD44780 compatible  
**Interface**: 4-bit parallel communication  
**Character Set**: ASCII + custom characters

### LCD Addressing

```
Position (x,y):
Row 1: (0,0) (1,0) (2,0) ... (15,0)
Row 2: (0,1) (1,1) (2,1) ... (15,1)
```

### Provided LCD Library Functions

| Function | Parameters | Description |
|----------|-----------|-------------|
| `lcd_init` | None | Initialize LCD. **Must be called first** |
| `lcd_gotoxy` | X (byte), Y (byte) on stack | Move cursor to position (x,y) |
| `lcd_puts` | Address (2 bytes) on stack | Display null-terminated string |
| `lcd_clr` | None | Clear entire LCD screen |
| `str_init` | Source addr (2 bytes), Dest addr (2 bytes) on stack | Copy string from program to data memory |

### LCD Usage Example

```assembly
; Initialize the LCD
call lcd_init

; Move cursor to top-left (0,0)
ldi r16, 0x00    ; Y position
push r16
ldi r16, 0x00    ; X position
push r16
call lcd_gotoxy
pop r16
pop r16

; Display a message
ldi r16, high(msg1)
push r16
ldi r16, low(msg1)
push r16
call lcd_puts
pop r16
pop r16
```

---

##  Part A: LCD Flashing Message Display

**File**: `display_partA.asm`

### Requirements

Create a program that alternates between displaying two messages on the LCD:

1. **Message 1**: Your name (e.g., "John Smith")
2. **Message 2**: "CSC 230: Spring 2025"

### Display Sequence

```
Time 0s:   [YourName         ]    (Row 1)
           [CSC 230: Spr 2025]    (Row 2)

Time 1s:   [YourName         ]    (Row 1 only)
           [                 ]    (Row 2 clear)

Time 2s:   [                 ]    (Row 1 clear)
           [CSC 230: Spr 2025]    (Row 2 only)

Time 3s:   [YourName         ]    (Both rows again)
           [CSC 230: Spr 2025]

... repeats continuously
```

### Technical Requirements

- Use a **timer** to generate 1-second delays
- No busy-wait loops for timing (must use timer interrupts or polling)
- Clearly document how the main program detects 1-second intervals
- Messages stored in program memory, copied to data memory via `str_init`

### Implementation Approach

**Option 1: Timer Compare Match Interrupt**
```assembly
; Configure Timer1 for 1-second intervals
; Set CTC mode with appropriate prescaler
; Enable Timer1 Compare Match A interrupt
; ISR increments a counter visible to main program
```

**Option 2: Timer Overflow Polling**
```assembly
; Configure Timer1 with prescaler
; Main loop polls overflow flag
; When flag set, update display and reset timer
```

### Example Timer Setup (Conceptual)

```assembly
; Timer1 CTC mode, 1 second at 16MHz
; Prescaler = 1024, OCR1A = 15624
ldi r16, high(15624)
sts OCR1AH, r16
ldi r16, low(15624)
sts OCR1AL, r16

; Set CTC mode (WGM12 = 1)
ldi r16, (1 << WGM12) | (1 << CS12) | (1 << CS10)
sts TCCR1B, r16

; Enable interrupt
ldi r16, (1 << OCIE1A)
sts TIMSK1, r16
sei
```

### Evaluation Criteria

- Correct 1-second timing (±10%)
- Proper display sequencing
- Clean transitions (no flicker)
- Well-documented timer configuration

---

## Part B: LCD Scrolling Message Display

**File**: `display_partB.asm`

### Requirements

Extend Part A to support **scrolling** for messages longer than 16 characters.

### Scrolling Behavior

```
Message: "This is a long scrolling message"

Display at t=0:    [This is a long s]
Display at t=0.5s: [his is a long sc]
Display at t=1.0s: [is is a long scr]
Display at t=1.5s: [s is a long scro]
... continues scrolling left
```

### Technical Requirements

- Support messages **longer than 16 characters**
- Scroll text **left** continuously
- Use a **second timer** for scroll speed control
- Scrolling should be smooth and readable
- Both rows can scroll independently
- After message fully scrolls, wrap around and repeat

### Implementation Strategy

**Dual-Timer System**:
- **Timer1**: 1-second intervals for message alternation (from Part A)
- **Timer2**: Shorter intervals (200-500ms) for scroll updates

**Scrolling Algorithm**:
```
1. Keep track of scroll offset (start position in message)
2. On scroll timer interrupt:
   a. Increment scroll offset
   b. Extract 16-character window starting at offset
   c. Display the window on LCD
   d. If offset >= message_length, reset to 0 (wrap around)
```

### String Windowing Example

```assembly
; Assume message in data memory at msg_addr
; Scroll offset in scroll_pos variable

extract_window:
    ldi r16, 16              ; Characters to extract
    ldi XL, low(msg_addr)
    ldi XH, high(msg_addr)
    
    ; Add scroll offset to start position
    lds r17, scroll_pos
    add XL, r17
    brcc no_carry
    inc XH
no_carry:
    
    ; Copy 16 chars to display buffer
    ldi r18, 16
copy_loop:
    ld r19, X+
    ; Store to display buffer
    dec r18
    brne copy_loop
```

### Evaluation Criteria

- Smooth scrolling (no jitter)
- Correct scroll speed (adjustable via timer)
- Proper wrap-around at message end
- Messages longer than 16 chars fully visible
- Clean code with clear timer separation

---

## Part C: Keypad-Controlled Scrolling

**File**: `display_partC.asm`

### Requirements

Extend Part B to allow **user control** of scroll direction using keypad buttons.

### Button Behavior

| Button | Action |
|--------|--------|
| **Left** | Scroll message to the left |
| **Right** | Scroll message to the right |
| **Up, Down, Select** | Ignored (no action) |
| **No button** | Pause scrolling (optional) |

### Keypad Interface

The DFRobot keypad uses a **single analog input** (ADC0) with different voltage levels for each button.

**ADC Value Ranges**:
```
Button   ADC Range (approx)
------   ------------------
Right    0 - 50
Up       50 - 176
Down     176 - 352
Left     352 - 555
Select   555 - 800
None     > 900
```

### ADC Reading Example

```assembly
read_button:
    ; Start ADC conversion on channel 0
    ldi r16, (1 << REFS0)    ; AVCC reference
    sts ADMUX, r16
    
    ldi r16, (1 << ADEN) | (1 << ADSC) | 7  ; Enable, start, prescaler 128
    sts ADCSRA, r16
    
wait_adc:
    lds r16, ADCSRA
    sbrc r16, ADSC           ; Wait for conversion
    rjmp wait_adc
    
    ; Read result (10-bit value)
    lds r16, ADCL            ; Read low byte first
    lds r17, ADCH            ; Then high byte
    ret
```

### Button Detection Logic

```assembly
check_button:
    rcall read_button        ; r17:r16 = ADC value
    
    ; Check if > 900 (no button)
    cpi r17, 3               ; High byte > 3 means > 900
    brsh no_button
    
    ; Check for Left (352-555)
    cpi r17, 1               
    brlo not_left            ; < 256
    cpi r16, 96              
    brlo not_left            ; < 352
    cpi r17, 2
    brsh not_left            ; >= 555
    ; It's Left button!
    rjmp handle_left
    
not_left:
    ; Check for Right (0-50)
    cpi r16, 51
    brsh not_right
    ; It's Right button!
    rjmp handle_right
    
not_right:
    ; Other buttons ignored
    ret
```

### Implementation Approach

**Option 1: Polling**
- Main loop periodically reads ADC
- Updates scroll direction based on button

**Option 2: Interrupt-Driven**
- Timer interrupt reads ADC
- Sets flags for main loop to process

### Scroll Direction Control

```assembly
; Global variables in data memory
scroll_direction:  .byte 1   ; 0=stopped, 1=left, 2=right
scroll_pos:        .byte 1   ; Current offset in message

scroll_update:
    lds r16, scroll_direction
    cpi r16, 1
    breq scroll_left
    cpi r16, 2
    breq scroll_right
    ret                      ; Direction = 0, no scroll

scroll_left:
    lds r16, scroll_pos
    inc r16
    ; Check if past message end, wrap to 0
    sts scroll_pos, r16
    ret

scroll_right:
    lds r16, scroll_pos
    dec r16
    ; Check if before message start, wrap to end
    sts scroll_pos, r16
    ret
```

### Evaluation Criteria

- Correct button detection (Left/Right only)
- Smooth direction changes
- Other buttons properly ignored
- Responsive to button presses
- No accidental double-triggering (debouncing optional but recommended)

---

## Part D: Keypad Position Display

**File**: `display_partD.asm`

### Requirements

Display button names at specific LCD positions when pressed.

### Button-to-Position Mapping

```
LCD Layout:
Row 1: [Up              Right]
Row 2: [Left             Down]

Position mapping:
┌────────────────┬──────────────────┐
│ Button         │ Position (x, y)  │
├────────────────┼──────────────────┤
│ Up             │ (0, 0) - top-left    │
│ Right          │ (11, 0) - top-right  │
│ Left           │ (0, 1) - bottom-left │
│ Down           │ (11, 1) - bottom-right│
└────────────────┴──────────────────┘
```

### Display Examples

**When Up is pressed:**
```
[Up              ]  ← "Up" appears at position (0,0)
[                ]
```

**When Right is pressed:**
```
[          Right ]  ← "Right" appears at position (11,0)
[                ]
```

**When Left is pressed:**
```
[                ]
[Left            ]  ← "Left" appears at position (0,1)
```

**When Down is pressed:**
```
[                ]
[            Down]  ← "Down" appears at position (11,1)
```

### Implementation Structure

```assembly
main_loop:
    call read_button
    call check_which_button
    ; r16 now contains button code (0=none, 1=up, 2=down, 3=left, 4=right)
    
    cpi r16, 0
    breq main_loop           ; No button, loop again
    
    cpi r16, 1
    breq display_up
    cpi r16, 2
    breq display_down
    cpi r16, 3
    breq display_left
    cpi r16, 4
    breq display_right
    rjmp main_loop

display_up:
    call lcd_clr
    ldi r16, 0x00            ; Y = 0
    push r16
    ldi r16, 0x00            ; X = 0
    push r16
    call lcd_gotoxy
    pop r16
    pop r16
    
    ldi r16, high(str_up)
    push r16
    ldi r16, low(str_up)
    push r16
    call lcd_puts
    pop r16
    pop r16
    rjmp main_loop

; Similar for other buttons...

.dseg
str_up:    .byte 10
str_down:  .byte 10
str_left:  .byte 10
str_right: .byte 10

.cseg
str_up_p:    .db "Up", 0
str_down_p:  .db "Down", 0
str_left_p:  .db "Left", 0
str_right_p: .db "Right", 0
```

### ADC Button Detection (Refined)

```assembly
; More robust button detection with ranges
check_which_button:
    call read_button         ; r17:r16 = ADC value
    
    ; No button (> 900)
    cpi r17, 3
    brsh button_none
    cpi r16, 132             ; Low byte check for > 900
    brsh button_none
    
    ; Right (0-50)
    cpi r16, 51
    brlo button_right
    
    ; Up (50-176)
    cpi r16, 177
    brlo button_up
    
    ; Down (176-352)
    cpi r17, 1
    brlo button_down         ; < 256
    cpi r16, 97
    brlo button_down         ; < 352
    
    ; Left (352-555)
    cpi r17, 2
    brlo button_left         ; < 512
    cpi r16, 44
    brlo button_left         ; < 555
    
    ; Select (555-800) - ignored for this part
    rjmp button_none

button_right:
    ldi r16, 4
    ret
button_up:
    ldi r16, 1
    ret
button_down:
    ldi r16, 2
    ret
button_left:
    ldi r16, 3
    ret
button_none:
    ldi r16, 0
    ret
```

### Evaluation Criteria

- Correct button detection for all 4 directions
- Text appears at correct positions
- Screen clears between button presses (optional)
- Responsive to button presses
- No Select button interference

---

## ⏱️ Timer Configuration Reference

### Timer1 (16-bit) - For 1-Second Delays

**CTC Mode with Interrupts**:
```assembly
; F_CPU = 16,000,000 Hz
; Prescaler = 1024
; Target = 1 second
; OCR1A = (F_CPU / Prescaler) - 1 = 15624

setup_timer1:
    ; Set compare value
    ldi r16, high(15624)
    sts OCR1AH, r16
    ldi r16, low(15624)
    sts OCR1AL, r16
    
    ; CTC mode (WGM12=1), Prescaler=1024 (CS12=1, CS10=1)
    ldi r16, (1 << WGM12) | (1 << CS12) | (1 << CS10)
    sts TCCR1B, r16
    
    ; Enable Compare Match A interrupt
    ldi r16, (1 << OCIE1A)
    sts TIMSK1, r16
    
    ; Clear any pending interrupts
    ldi r16, (1 << OCF1A)
    sts TIFR1, r16
    
    sei                      ; Enable global interrupts
    ret

; Interrupt vector
.org 0x0020                  ; Timer1 Compare Match A vector
    rjmp TIMER1_COMPA_ISR

TIMER1_COMPA_ISR:
    push r16
    in r16, SREG
    push r16
    
    ; Increment second counter
    lds r16, seconds
    inc r16
    sts seconds, r16
    
    pop r16
    out SREG, r16
    pop r16
    reti
```

### Timer2 (8-bit) - For Scroll Speed

**Overflow Mode for ~250ms**:
```assembly
; F_CPU = 16,000,000 Hz
; Prescaler = 1024
; Overflow every: 256 * 1024 / 16,000,000 = 0.016384 s
; For 250ms: Need to count 15-16 overflows

setup_timer2:
    ; Normal mode, Prescaler=1024
    ldi r16, (1 << CS22) | (1 << CS21) | (1 << CS20)
    sts TCCR2B, r16
    
    ; Enable overflow interrupt
    ldi r16, (1 << TOIE2)
    sts TIMSK2, r16
    
    sei
    ret

.org 0x001C                  ; Timer2 Overflow vector
    rjmp TIMER2_OVF_ISR

TIMER2_OVF_ISR:
    push r16
    in r16, SREG
    push r16
    
    ; Increment overflow counter
    lds r16, ovf_count
    inc r16
    cpi r16, 15              ; 15 overflows ≈ 250ms
    brlo skip_scroll
    
    ; Reset counter and set scroll flag
    clr r16
    ldi r17, 1
    sts scroll_flag, r17
    
skip_scroll:
    sts ovf_count, r16
    
    pop r16
    out SREG, r16
    pop r16
    reti
```

---

## Common Pitfalls & Debugging

### LCD Issues

**Problem**: Nothing appears on LCD  
**Solutions**:
- Ensure `lcd_init` is called before any LCD functions
- Check that LCD library files are properly included
- Verify LCD is physically connected and powered

**Problem**: Garbled characters  
**Solutions**:
- Strings must be null-terminated
- Use `str_init` to copy from program to data memory
- Don't exceed 16 characters per line without scrolling

### Timer Issues

**Problem**: Timing is way off  
**Solutions**:
- Double-check prescaler and compare/top values
- Verify F_CPU matches actual clock (16MHz for Mega 2560)
- Use correct timer mode (CTC vs Normal vs PWM)

**Problem**: ISR never executes  
**Solutions**:
- Enable global interrupts with `sei`
- Verify interrupt vector address is correct
- Check that specific timer interrupt is enabled in TIMSKx

### ADC/Keypad Issues

**Problem**: Wrong buttons detected  
**Solutions**:
- ADC ranges vary between boards - calibrate yours
- Read ADCL before ADCH (order matters!)
- Add hysteresis to prevent bouncing between ranges
- Use delays or debouncing for cleaner button reads

### Memory Issues

**Problem**: Strings not displaying correctly  
**Solutions**:
- Reserve enough space in `.dseg` (.byte 200 is safe)
- Always null-terminate strings
- Use `str_init` correctly: source in program memory, destination in data memory

---


### Code Demo Evaluation

**Important**: Assignment 3 includes a **live code demonstration** during the week of March 24th in your lab section.

**Expect questions about**:
- How your timer configuration works
- Why you chose specific prescaler values
- How interrupt service routines interact with main loop
- Your button detection algorithm
- Scrolling implementation details

**Preparation tips**:
- Understand every line of your code
- Be able to explain design decisions
- Know your timer calculations
- Test thoroughly before the demo

---

## Skills Demonstrated

### Embedded Systems Programming
- Hardware timer configuration and management
- Interrupt-driven architecture
- Real-time display updates
- Peripheral device interfacing

### Low-Level I/O
- LCD protocol communication (HD44780)
- Analog-to-Digital Conversion (ADC)
- Button matrix reading through voltage dividers
- Memory-mapped I/O register manipulation

### Software Engineering
- Modular code design with clear function separation
- State machine implementation (scroll direction, display mode)
- Event handling (timer interrupts, button presses)
- Resource management (timers, memory buffers)

---

## Key Concepts

### Interrupt Service Routines (ISRs)
- **Purpose**: Handle time-critical events without polling
- **Requirements**: 
  - Fast execution (minimize ISR code)
  - Save/restore registers and SREG
  - Set flags for main loop to process
  - End with `reti`, not `ret`

### Timer Modes
- **Normal**: Count to max, overflow
- **CTC (Clear Timer on Compare)**: Count to OCRx, reset
- **PWM**: Generate pulse-width modulated signals

### LCD Addressing
- **DDRAM**: Display Data RAM (what's shown on screen)
- **CGRAM**: Character Generator RAM (custom characters)
- Position formula: Row 1 starts at 0x00, Row 2 at 0x40

---

## Testing Strategy

### Part A Testing
1. Verify initial display (both messages)
2. Time the alternation with a stopwatch
3. Check for clean transitions (no flicker)
4. Let run for several minutes (stability test)

### Part B Testing
1. Create messages of varying lengths (10, 20, 30+ chars)
2. Verify smooth scrolling (no jumps)
3. Check wrap-around behavior
4. Test both rows independently

### Part C Testing
1. Test each button individually
2. Change direction mid-scroll
3. Press multiple buttons rapidly
4. Verify ignored buttons have no effect

### Part D Testing
1. Press each button and verify position
2. Press rapidly to check responsiveness
3. Hold button down (should not retrigger)
4. Test all four buttons multiple times

---

## Running the Projects

### Setup
1. Open Microchip Studio 7
2. Create new AVR Assembler project
3. Select ATmega2560 device
4. Add LCD library files to project folder
5. Copy your display_partX.asm code

### Building
```
Build → Build Solution (F7)
```

### Programming the Board
1. Connect Arduino Mega 2560 via USB
2. **Tools → Device Programming**
3. **Tool**: AVRISP mkII or Arduino bootloader
4. **Device**: ATmega2560
5. **Interface**: ISP or UART
6. **Program** → Flash → Select .hex file

### Testing on Hardware
- Watch LCD display for expected behavior
- Press keypad buttons for Parts C and D
- Use serial monitor for debugging (if needed)
- Measure timing with stopwatch for verification

---

## eferences

### Datasheets
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [HD44780 LCD Controller](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)

### Course Materials
- CSC 230 Lab 7: Timers and Interrupts
- CSC 230 Lab 8: LCD Display
- CSC 230 Lab 4: ADC and Keypad

### AVR Resources
- [AVR Instruction Set Manual](http://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf)
- [AVR Timer Tutorial](http://maxembedded.com/2011/06/avr-timers/)

---

**Author**: Armita Darbandi
**Course**: CSC 230 - Spring 2025  
**Institution**: University of Victoria  
**Lab**: ECS 249
