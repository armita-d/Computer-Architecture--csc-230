# Assignment 4: Digital Clock and Stopwatch in C

**CSC 230: Computer Architecture and Assembly Language**  
**University of Victoria - Spring 2025**

## Overview

This final assignment transitions from assembly language to **C programming** while maintaining the same hardware platform. The project implements a digital clock with stopwatch functionality, demonstrating timer interrupts, LCD display control, and button input handling - all in C.

**Key Transition**: This assignment shows how high-level C code maps to the low-level assembly concepts mastered in Assignments 1-3.

##  Learning Objectives

- Transition from assembly to C for embedded systems
- Configure AVR timers using C
- Write interrupt service routines (ISRs) in C
- Interface with LCD display using C libraries
- Process button input in C
- Implement real-time clock and stopwatch logic
- Understand C-to-assembly compilation for AVR

##  Hardware Requirements

- **Board**: Arduino Mega 2560 (ATmega2560 microcontroller)
- **Display**: 16x2 character LCD with HD44780 controller
- **Input**: DFRobot LCD Keypad Shield (Select button used)
- **Development Environment**: Microchip Studio 7 (C mode)
- **Testing Location**: ECS 249 lab (UVic)

##  Project Structure

```
assignment-4/
├── main.c                    # Your implementation (SUBMIT THIS)
├── lcd_drv.c                 # LCD driver (provided)
├── lcd_drv.h                 # LCD driver header (provided)
├── main.h                    # Main header with LCD config (provided)
├── mydefs.h                  # Helper definitions (provided)
├── timer_interrupt.c         # Timer example (provided, reference)
├── button.c                  # Button example (provided, reference)
└── README.md                 # This file
```

**Submit ONLY**: `main.c` file

##  Project Requirements

### Digital Clock Display

Implement a clock that displays time in the format:

```
Line 1: [HH:MM:SS.mmm]  ← Running clock
Line 2: [HH:MM:SS.mmm]  ← Stopwatch (pauses/resumes)
```

**Format Breakdown**:
- `HH`: Hours (00-23, 2 digits)
- `MM`: Minutes (00-59, 2 digits)
- `SS`: Seconds (00-59, 2 digits)
- `mmm`: Milliseconds/subseconds (000-999, 3 digits)

**Example Display**:
```
[01:23:45.678]
[01:23:45.678]
```

### Stopwatch Functionality

**Button Behavior**:
1. **Select button pressed (1st time)**: Line 2 **pauses** (freezes current time)
2. **Select button pressed (2nd time)**: Line 2 **resumes** (syncs with Line 1)
3. Line 1 **always runs** continuously

**Example Sequence**:
```
Time = 0s (both lines running):
[00:00:05.230]
[00:00:05.230]

User presses Select at 00:00:10.500:
[00:00:10.500]
[00:00:10.500]  ← Line 2 freezes here

Time continues (Line 1 keeps going):
[00:00:15.820]
[00:00:10.500]  ← Line 2 still frozen

User presses Select again at 00:00:20.100:
[00:00:20.100]
[00:00:20.100]  ← Line 2 resumes, synced with Line 1
```

---

##  Implementation Guide

### Timer Configuration

**Goal**: Generate interrupts every **10ms** (0.01 seconds) for 100Hz update rate.

**Timer1 Setup** (16-bit timer, recommended):

```c
void setup_timer1(void) {
    // Disable interrupts during setup
    cli();
    
    // Set Timer1 to CTC mode (Clear Timer on Compare Match)
    // WGM12 = 1 (CTC mode, TOP = OCR1A)
    TCCR1B |= (1 << WGM12);
    
    // Set prescaler to 64
    // CS11 = 1, CS10 = 1 → Prescaler = 64
    TCCR1B |= (1 << CS11) | (1 << CS10);
    
    // Calculate compare value for 10ms interrupt
    // F_CPU = 16,000,000 Hz
    // Prescaler = 64
    // Timer frequency = 16,000,000 / 64 = 250,000 Hz
    // For 10ms: 250,000 * 0.01 = 2,500 ticks
    OCR1A = 2500;
    
    // Enable Timer1 Compare Match A interrupt
    TIMSK1 |= (1 << OCIE1A);
    
    // Enable global interrupts
    sei();
}
```

**Alternative: Timer0 (8-bit)**:
```c
void setup_timer0(void) {
    cli();
    
    // Set prescaler to 256
    TCCR0B |= (1 << CS02);
    
    // Enable overflow interrupt
    TIMSK0 |= (1 << TOIE0);
    
    // Set initial value for ~10ms overflow
    // With prescaler 256: 16MHz / 256 = 62.5 kHz
    // For 10ms: 62,500 * 0.01 = 625 ticks
    // TCNT0 starts at: 256 - 625 = -369 (overflow)
    // Use multiple overflows or adjust logic
    
    sei();
}
```

### Interrupt Service Routine (ISR)

**Structure**:
```c
#include <avr/interrupt.h>

// Global time counters (volatile for ISR access)
volatile uint16_t subseconds = 0;  // 0-999 (milliseconds)
volatile uint8_t seconds = 0;      // 0-59
volatile uint8_t minutes = 0;      // 0-59
volatile uint8_t hours = 0;        // 0-23

// Stopwatch counters
volatile uint16_t sw_subseconds = 0;
volatile uint8_t sw_seconds = 0;
volatile uint8_t sw_minutes = 0;
volatile uint8_t sw_hours = 0;

// Stopwatch state
volatile uint8_t sw_paused = 0;  // 0 = running, 1 = paused

// Timer1 Compare Match A ISR
ISR(TIMER1_COMPA_vect) {
    // Increment subseconds (each interrupt = 10ms = 0.01s)
    subseconds += 10;  // Add 10 milliseconds
    
    if (subseconds >= 1000) {
        subseconds = 0;
        seconds++;
        
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                
                if (hours >= 24) {
                    hours = 0;  // Reset to 00:00:00.000
                }
            }
        }
    }
    
    // Update stopwatch if not paused
    if (!sw_paused) {
        sw_subseconds = subseconds;
        sw_seconds = seconds;
        sw_minutes = minutes;
        sw_hours = hours;
    }
}
```

### Button Detection (ADC)

**Reading the Select Button**:

```c
#include <avr/io.h>

// Initialize ADC for button reading
void init_adc(void) {
    // ADMUX: AVcc reference, ADC0 channel
    ADMUX = (1 << REFS0);
    
    // ADCSRA: Enable ADC, prescaler = 128
    // ADC Clock = 16MHz / 128 = 125kHz (ideal: 50-200kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Read ADC value from button input
uint16_t read_button_adc(void) {
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));
    
    // Read result (10-bit value, 0-1023)
    uint16_t result = ADCL;          // Read low byte first!
    result |= (ADCH << 8);           // Then high byte
    
    return result;
}

// Determine which button is pressed
typedef enum {
    BTN_NONE = 0,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_SELECT
} button_t;

button_t get_button(void) {
    uint16_t adc_val = read_button_adc();
    
    if (adc_val > 1000) return BTN_NONE;
    if (adc_val < 50)   return BTN_RIGHT;
    if (adc_val < 195)  return BTN_UP;
    if (adc_val < 380)  return BTN_DOWN;
    if (adc_val < 555)  return BTN_LEFT;
    if (adc_val < 790)  return BTN_SELECT;
    
    return BTN_NONE;
}
```

**Debouncing** (simple approach):
```c
button_t last_button = BTN_NONE;

button_t get_button_debounced(void) {
    button_t current = get_button();
    
    // Only register button if it's newly pressed
    if (current != BTN_NONE && last_button == BTN_NONE) {
        last_button = current;
        return current;
    }
    
    // Update last button state
    if (current == BTN_NONE) {
        last_button = BTN_NONE;
    }
    
    return BTN_NONE;
}
```

### Number to ASCII Conversion

**Method 1: Manual Conversion**
```c
void uint8_to_string(uint8_t value, char *buffer) {
    // Extract digits
    buffer[0] = '0' + (value / 10);       // Tens digit
    buffer[1] = '0' + (value % 10);       // Ones digit
    buffer[2] = '\0';                     // Null terminator
}

void uint16_to_string3(uint16_t value, char *buffer) {
    // For 3-digit subseconds (000-999)
    buffer[0] = '0' + (value / 100);           // Hundreds
    buffer[1] = '0' + ((value / 10) % 10);     // Tens
    buffer[2] = '0' + (value % 10);            // Ones
    buffer[3] = '\0';                          // Null terminator
}
```

**Method 2: Using sprintf**
```c
#include <stdio.h>

char time_str[20];

void format_time(uint8_t h, uint8_t m, uint8_t s, uint16_t ss) {
    // Format: "HH:MM:SS.mmm"
    sprintf(time_str, "%02d:%02d:%02d.%03d", h, m, s, ss);
}
```

**Method 3: Using itoa**
```c
#include <stdlib.h>

char buffer[4];
itoa(subseconds, buffer, 10);  // Convert to base-10 string
```

### LCD Display Functions

**Provided Functions**:

```c
#include "lcd_drv.h"

// Initialize LCD (call once at startup)
lcd_init();

// Move cursor to position (x, y)
// x = 0-15 (column), y = 0-1 (row)
lcd_xy(0, 0);  // Top-left
lcd_xy(0, 1);  // Bottom-left

// Display a null-terminated string
lcd_puts("Hello");

// Display a single character
lcd_putchar('A');

// Blank n characters (erase)
lcd_blank(5);  // Erase 5 characters

// Send raw command to LCD
lcd_command(0x01);  // Clear display
```

**Usage Example**:
```c
// Display time on Line 1
lcd_xy(0, 0);
lcd_puts("01:23:45.678");

// Display time on Line 2
lcd_xy(0, 1);
lcd_puts("01:23:45.678");
```

### Complete Main Program Structure

```c
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "lcd_drv.h"
#include "main.h"

// Global time variables (volatile for ISR)
volatile uint16_t subseconds = 0;
volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0;

// Stopwatch variables
volatile uint16_t sw_subseconds = 0;
volatile uint8_t sw_seconds = 0;
volatile uint8_t sw_minutes = 0;
volatile uint8_t sw_hours = 0;
volatile uint8_t sw_paused = 0;

// ISR for timer
ISR(TIMER1_COMPA_vect) {
    // Increment main clock
    subseconds += 10;
    if (subseconds >= 1000) {
        subseconds = 0;
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                if (hours >= 24) hours = 0;
            }
        }
    }
    
    // Update stopwatch if running
    if (!sw_paused) {
        sw_subseconds = subseconds;
        sw_seconds = seconds;
        sw_minutes = minutes;
        sw_hours = hours;
    }
}

void setup_timer(void) {
    cli();
    TCCR1B |= (1 << WGM12);                    // CTC mode
    TCCR1B |= (1 << CS11) | (1 << CS10);       // Prescaler 64
    OCR1A = 2500;                              // 10ms
    TIMSK1 |= (1 << OCIE1A);                   // Enable interrupt
    sei();
}

void init_adc(void) {
    ADMUX = (1 << REFS0);                      // AVcc reference
    ADCSRA = 0x87;                             // Enable, prescaler 128
}

uint16_t read_adc(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADCL | (ADCH << 8);
}

int is_select_pressed(void) {
    uint16_t val = read_adc();
    return (val >= 555 && val < 790);
}

void display_time(uint8_t h, uint8_t m, uint8_t s, uint16_t ss, uint8_t line) {
    char buffer[17];
    sprintf(buffer, "%02d:%02d:%02d.%03d", h, m, s, ss);
    lcd_xy(0, line);
    lcd_puts(buffer);
}

int main(void) {
    lcd_init();
    setup_timer();
    init_adc();
    
    uint8_t last_button_state = 0;
    
    while (1) {
        // Read button
        uint8_t button_pressed = is_select_pressed();
        
        // Detect button press (rising edge)
        if (button_pressed && !last_button_state) {
            // Toggle stopwatch pause state
            sw_paused = !sw_paused;
            _delay_ms(200);  // Simple debounce
        }
        last_button_state = button_pressed;
        
        // Display times (copy volatile vars to avoid race conditions)
        cli();
        uint8_t h = hours, m = minutes, s = seconds;
        uint16_t ss = subseconds;
        uint8_t sw_h = sw_hours, sw_m = sw_minutes, sw_s = sw_seconds;
        uint16_t sw_ss = sw_subseconds;
        sei();
        
        display_time(h, m, s, ss, 0);           // Line 1: running clock
        display_time(sw_h, sw_m, sw_s, sw_ss, 1);  // Line 2: stopwatch
        
        _delay_ms(50);  // Update display ~20 times/second
    }
    
    return 0;
}
```

---

##  Key C vs Assembly Concepts

### Comparison Table

| Concept | Assembly (A1-A3) | C (A4) |
|---------|------------------|--------|
| **Variables** | Registers (r16, r17...) | `int`, `uint8_t` variables |
| **Functions** | `rcall`, `ret` | `void func(void) { }` |
| **Loops** | `label: ... rjmp label` | `while(1) { }`, `for` |
| **Conditions** | `brne`, `breq`, `brlo` | `if`, `switch` |
| **ISR** | `.org` vector, `reti` | `ISR(TIMER1_COMPA_vect)` |
| **I/O** | `out PORTL, r16` | `PORTL = value;` |
| **Memory** | `.dseg`, `.cseg` | `volatile uint8_t var;` |

### Under the Hood

When you write this C code:
```c
PORTL = 0xFF;
```

The compiler generates assembly similar to:
```assembly
ldi r16, 0xFF
out PORTL, r16
```

Your knowledge of assembly helps you understand:
- Why `volatile` is needed for ISR variables
- How function calls use the stack
- Why order matters in ADC register reads
- Performance implications of different code patterns

---

## Common Issues & Solutions

### Timer Issues

 **Problem**: Clock runs too fast/slow  
 **Solutions**:
- Verify F_CPU is 16000000UL
- Check prescaler calculation
- Confirm OCR1A value: `F_CPU / (prescaler * desired_frequency)`
- For 10ms at prescaler 64: `16,000,000 / (64 * 100) = 2,500`

 **Problem**: ISR never executes  
 **Solutions**:
```c
// Ensure all of these are present:
TCCR1B |= (1 << WGM12);           // CTC mode
TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler
OCR1A = 2500;                      // Compare value
TIMSK1 |= (1 << OCIE1A);          // Enable interrupt
sei();                             // Global interrupts ON
```

### Display Issues

 **Problem**: Numbers don't display correctly  
 **Solutions**:
```c
// WRONG: Trying to display raw number
lcd_putchar(45);  // Displays '-' character (ASCII 45), not "45"

// CORRECT: Convert to ASCII string
char buffer[3];
buffer[0] = '0' + (45 / 10);  // '4'
buffer[1] = '0' + (45 % 10);  // '5'
buffer[2] = '\0';
lcd_puts(buffer);

// OR use sprintf:
sprintf(buffer, "%02d", 45);  // "45"
lcd_puts(buffer);
```

 **Problem**: Display flickers  
 **Solutions**:
- Don't call `lcd_init()` in main loop (only once at startup)
- Limit update rate (e.g., update every 50-100ms, not every loop)
- Use buffering: only update when value changes

### Button Issues

 **Problem**: Button triggers multiple times  
 **Solutions**:
```c
// Implement debouncing and edge detection
static uint8_t last_state = 0;

uint8_t current = is_select_pressed();
if (current && !last_state) {
    // Button just pressed (rising edge)
    sw_paused = !sw_paused;
    _delay_ms(200);  // Debounce delay
}
last_state = current;
```

 **Problem**: Wrong button detected  
 **Solutions**:
- Always read ADCL before ADCH (hardware requirement)
- Calibrate ADC ranges for your specific board
- Add margin to ranges (e.g., 555-790 for Select)

### Volatile Variables

 **Problem**: Variables don't update from ISR  
 **Solutions**:
```c
// WRONG:
uint8_t seconds = 0;

// CORRECT:
volatile uint8_t seconds = 0;

// Volatile tells compiler:
// "This variable can change outside normal program flow"
```

### Race Conditions

 **Problem**: Display shows corrupted time (e.g., 99:99:99)  
 **Solutions**:
```c
// WRONG: Reading multi-byte value without protection
display_time(hours, minutes, seconds, subseconds, 0);
// ISR might change hours mid-read!

// CORRECT: Atomic copy
cli();  // Disable interrupts
uint8_t h = hours, m = minutes, s = seconds;
uint16_t ss = subseconds;
sei();  // Re-enable interrupts
display_time(h, m, s, ss, 0);
```


## 🎓 Skills Demonstrated

### C Programming for Embedded Systems
- Writing interrupt service routines in C
- Using volatile qualifier for shared variables
- Bit manipulation in C (`|=`, `&=`, `<<`, `>>`)
- Working with AVR-specific registers and types

### Hardware Abstraction
- Timer configuration through C register manipulation
- ADC control and data acquisition
- LCD driver library usage
- Button input processing

### Software Engineering
- Modular code design (setup functions, helper functions)
- State management (stopwatch pause/resume)
- Race condition handling (cli/sei protection)
- Code documentation and comments

### Real-Time Programming
- Interrupt-driven time keeping
- Event detection (button presses)
- Display refresh management
- Debouncing strategies

---

## Assembly to C Translation Guide

### Assignment 1-3 Concepts in C

**Timer Setup** (from A2/A3):
```assembly
; Assembly (A3)
ldi r16, high(15624)
sts OCR1AH, r16
ldi r16, low(15624)
sts OCR1AL, r16
```
```c
// C (A4)
OCR1A = 15624;  // Compiler handles high/low bytes
```

**I/O Port Control** (from A2):
```assembly
; Assembly (A2)
ldi r16, 0xFF
out PORTL, r16
```
```c
// C (A4)
PORTL = 0xFF;
```

**Button Reading** (from A3):
```assembly
; Assembly (A3)
lds r16, ADCL
lds r17, ADCH
```
```c
// C (A4)
uint16_t val = ADCL | (ADCH << 8);
```

**ISR** (from A3):
```assembly
; Assembly (A3)
.org 0x0020
    rjmp TIMER1_ISR
TIMER1_ISR:
    push r16
    in r16, SREG
    push r16
    ; ... code ...
    pop r16
    out SREG, r16
    pop r16
    reti
```
```c
// C (A4)
ISR(TIMER1_COMPA_vect) {
    // Compiler handles register saving
    // ... code ...
    // Compiler handles reti
}
```

---

## Testing Strategy

### Unit Testing

**Timer Accuracy**:
1. Use stopwatch to time 60 seconds
2. Clock should show exactly 01:00:00.000 (±0.1s acceptable)
3. Test for 5+ minutes to check drift

**Subseconds Accuracy**:
1. Observe subseconds incrementing
2. Should go 000 → 010 → 020 → ... → 990 → 000
3. Each step should be ~10ms

**Rollover Testing**:
```c
// Manually set time near rollover
hours = 23;
minutes = 59;
seconds = 55;
subseconds = 0;
// Wait and verify proper 24-hour rollover
```

### Integration Testing

**Stopwatch Pause**:
1. Start program
2. Let run to 00:00:10.000
3. Press Select → Line 2 freezes
4. Wait until Line 1 reaches 00:00:20.000
5. Verify Line 2 still shows 00:00:10.000

**Stopwatch Resume**:
1. Continue from above
2. Press Select again
3. Both lines should immediately show same time
4. Both should continue incrementing together

**Rapid Button Presses**:
1. Press Select rapidly 10 times
2. Stopwatch should toggle correctly
3. No stuck states or crashes

---

## C Library Functions Used

### Standard C Functions

```c
#include <stdio.h>
sprintf(buffer, "%02d:%02d:%02d.%03d", h, m, s, ss);
// Format string to buffer

#include <stdlib.h>
itoa(value, buffer, 10);
// Integer to ASCII (base 10)

#include <string.h>
strlen(str);     // String length
strcpy(dest, src);  // String copy
```

### AVR-Specific

```c
#include <avr/io.h>
// Register definitions (PORTL, DDRB, TIMSK1, etc.)

#include <avr/interrupt.h>
ISR(TIMER1_COMPA_vect) { }  // Define ISR
sei();  // Enable global interrupts
cli();  // Disable global interrupts

#include <util/delay.h>
_delay_ms(100);  // Delay milliseconds
_delay_us(50);   // Delay microseconds
```

---

##  Optimization Tips

### Memory Efficiency

```c
// Use smallest appropriate type
uint8_t seconds;      // 0-59 fits in 8 bits
uint16_t subseconds;  // 0-999 fits in 16 bits

// Not this:
int seconds;          // Wastes memory (int is 16-bit on AVR)
```

### Display Efficiency

```c
// Only update when values change
static uint8_t last_seconds = 255;
if (seconds != last_seconds) {
    display_time(...);
    last_seconds = seconds;
}
```

### ISR Efficiency

```c
// Keep ISRs SHORT and FAST
ISR(TIMER1_COMPA_vect) {
    subseconds += 10;
    // Don't put LCD updates in ISR!
    // Just update counters
}
```

---

##  References

### Datasheets & Manuals
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [AVR Libc Reference](https://www.nongnu.org/avr-libc/user-manual/)

### C Programming for AVR
- [AVR GCC Tutorial](https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial)
- [Interrupt Handling in C](http://efundies.com/avr-timer-interrupts-in-c/)

### Course Materials
- CSC 230 Lab materials on timers
- CSC 230 Lab materials on LCD in C
- Provided example files: `timer_interrupt.c`, `button.c`, `main.c`

---

## Why Assignment 4 Matters

### Course Progression Summary

**A1**: Pure computation (bit manipulation) in assembly  
**A2**: Simple I/O (LEDs) + functions in assembly  
**A3**: Complex I/O (LCD, keypad) + interrupts in assembly  
**A4**: All of the above, but in **C**

### Skills Integration

By completing all 4 assignments, you've demonstrated:

1. **Low-level understanding**: Bit manipulation, registers, instruction sets
2. **Hardware control**: I/O ports, timers, ADC, LCD protocol
3. **Systems programming**: Interrupts, real-time constraints, event handling
4. **Language progression**: Assembly → C (understanding the abstraction)
5. **Embedded development**: Complete real-world embedded system

### Career Relevance

This progression mirrors real embedded systems development:
- Start with understanding the hardware (assembly)
- Build up to system-level programming (C)
- Work within constraints (timing, memory, peripherals)
- Debug across hardware-software boundaries

---

**Author**: Armita Darbandi
**Course**: CSC 230 - Spring 2025  
**Institution**: University of Victoria  
**Lab**: ECS 249

---

*This assignment completes your journey from bit-level assembly programming to high-level embedded C development on the AVR platform.*
