# CSC 230: Computer Architecture and Assembly Language

**University of Victoria - Spring 2025**

This repository contains assembly programming projects completed for CSC 230: Introduction to Computer Architecture. All projects are written in AVR assembly language and designed to run on the ATmega2560 microcontroller using Microchip Studio 7.

## Course Overview

Computer architecture fundamentals and low-level programming using AVR assembly language. Topics include:
- Binary arithmetic and number representation
- Bit manipulation techniques
- Register-level operations
- AVR instruction set architecture
- Algorithm implementation in assembly

##  Repository Structure

```
├── assignment-1/          # Bit manipulation and arithmetic operations
├── assignment-2/          # LED signaling with function calls
├── assignment-3/          # LCD display with timer interrupts
└── assignment-4/          # Digital clock in C programming
```

##  Development Environment

- **Platform**: AVR ATmega2560
- **IDE**: Microchip Studio 7 (AVR Studio)
- **Simulator**: Built-in Microchip Studio simulator
- **Architecture**: 8-bit RISC

##  Projects

### Assignment 1: Bit Manipulation and Arithmetic
Low-level bit operations and multi-byte arithmetic
- **16-bit Addition**: Extended precision arithmetic using 8-bit registers
- **Rightmost Bit Reset**: Identifying and clearing contiguous bit sequences
- **Two's Complement**: Sign conversion using bit manipulation

[View Assignment 1 Details →](./assignment-1/)

### Assignment 2: LED Signaling Display
Hardware interfacing and function implementation
- **LED Control**: Direct I/O port manipulation for 6-LED array
- **Parameter Passing**: Register-based and stack-based conventions
- **Pattern Encoding**: Letter-to-LED encoding with lookup tables
- **Message Display**: String processing from program memory

[View Assignment 2 Details →](./assignment-2/)

### Assignment 3: LCD Display and Interrupt-Driven I/O
Real-time embedded systems with timers and peripherals
- **LCD Interface**: HD44780 controller communication
- **Timer Interrupts**: Precise timing without busy-wait loops
- **Scrolling Display**: Text animation for long messages
- **Keypad Input**: ADC-based button detection and control

[View Assignment 3 Details →](./assignment-3/)

### Assignment 4: Digital Clock and Stopwatch in C
Transitioning from assembly to C for embedded systems
- **C Programming**: Same hardware, high-level language
- **Real-Time Clock**: Timer-based timekeeping with subsecond precision
- **Stopwatch Function**: Pause/resume functionality with button control
- **Assembly to C**: Understanding the abstraction layer

[View Assignment 4 Details →](./assignment-4/)

##  Skills Demonstrated

- **Low-level programming**: Direct hardware manipulation without high-level abstractions
- **C for embedded systems**: AVR C programming with interrupts and peripherals
- **Hardware interfacing**: I/O port control, LCD displays, and LED arrays
- **Interrupt handling**: Timer interrupts and ISR implementation
- **Real-time systems**: Event-driven architecture and precise timing
- **Function design**: Modular code with clear calling conventions
- **Parameter passing**: Register-based and stack-based techniques
- **Peripheral programming**: ADC, timers, and external devices
- **Algorithm design**: Implementing solutions with limited instruction sets
- **Bit manipulation**: Efficient operations on binary data
- **Memory management**: Stack operations and program memory access
- **Code optimization**: Working within architectural constraints
- **Documentation**: Clear code comments and technical explanations
- **Language progression**: Assembly to C translation and abstraction understanding

##  Getting Started

### Prerequisites
- Microchip Studio 7 ([Download here](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio))
- Windows OS (required for Microchip Studio)

### Running the Projects
1. Clone this repository
2. Open Microchip Studio 7
3. Create a new AVR Assembler project
4. Copy the `.asm` file contents into your project
5. Build and run in the simulator (Debug > Start Debugging and Break)
6. Step through code and observe register values

## Learning Outcomes

Through these projects, I gained hands-on experience with:
- Understanding how high-level operations map to machine instructions
- Working directly with processor registers and flags
- Designing algorithms for resource-constrained environments
- Debugging at the hardware level
- Reading and understanding assembly language code

## Author

Armita Darbandi
Computer Science Student, University of Victoria  
CSC 230 - Spring 2025

##  License

These projects are academic assignments completed as coursework. Code is provided for educational and portfolio purposes.

---

*Note: This repository showcases individual work completed for CSC 230. All code follows UVic's academic integrity policies.*
