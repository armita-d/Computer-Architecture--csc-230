# Assignment 1: Bit Manipulation and Arithmetic Operations

**CSC 230: Computer Architecture and Assembly Language**  
**University of Victoria - Spring 2025**

##  Overview

This assignment explores fundamental operations in AVR assembly language, focusing on bit-level manipulation and multi-byte arithmetic. All programs operate directly on registers without using AVR library functions, demonstrating low-level control of the ATmega2560 processor.

##  Objectives

- Implement multi-byte arithmetic using 8-bit registers
- Design algorithms for bit pattern manipulation
- Apply two's complement representation for signed numbers
- Work within AVR architectural constraints
- Create flowcharts documenting algorithm logic

##  Files

```
assignment-1/
├── sixteen-bit-addition.asm      # Q1: 16-bit addition using 8-bit registers
├── reset-rightmost.asm            # Q2: Clear rightmost contiguous bits
├── twos-complement.asm            # Q3: Convert positive to negative (signed)
└── reset-rightmost-flowchart.pdf  # Algorithm flowchart for Q2
```

##  Problems Solved

### Problem 1: 16-Bit Addition

**Challenge**: AVR's ADD and ADC instructions only work with 8-bit values. How do we add two 16-bit numbers?

**Solution**: Use two 8-bit registers per 16-bit number and leverage the carry flag.

**Input**:
- First number: `(R17:R16)` - R17 = high byte, R16 = low byte
- Second number: `(R19:R18)` - R19 = high byte, R18 = low byte

**Output**:
- Result: `(R5:R4)` - R5 = high byte, R4 = low byte

**Example**:
```
  0x0994  (R17=0x09, R16=0x94)
+ 0xFD35  (R19=0xFD, R18=0x35)
---------
  0x06C9  (R5=0x06, R4=0xC9)
```

**Key Concept**: The ADD instruction sets the carry flag if overflow occurs. The ADC (Add with Carry) instruction includes this carry when adding the high bytes.

---

### Problem 2: Reset Rightmost Contiguous Bits

**Challenge**: Identify and clear the rightmost sequence of consecutive '1' bits in a byte.

**Solution**: Bit scanning algorithm that detects contiguous sequences.

**Input**: 8-bit value in R16  
**Output**: Modified value in R1

**Examples**:
```
Input:  0b01011100  →  Output: 0b01000000
Input:  0b10110110  →  Output: 0b10110000
Input:  0b11111111  →  Output: 0b00000000
Input:  0b00000000  →  Output: 0b00000000
```

**Algorithm**:
1. Scan from right to left to find first '1' bit
2. Continue scanning while bits remain '1'
3. Clear all bits in the contiguous sequence
4. Preserve bits to the left

**Flowchart**: See `reset-rightmost-flowchart.pdf`

---

### Problem 3: Two's Complement Conversion

**Challenge**: Convert a positive number to its negative representation using two's complement.

**Solution**: Find rightmost set bit, keep it and everything to the right intact, flip all bits to the left.

**Input**: Positive 8-bit value in R16  
**Output**: Two's complement (negative) in R17  
**Constraint**: R16 must remain unchanged

**Examples**:
```
+12:  0b00001100  →  -12: 0b11110100
+54:  0b00110110  →  -54: 0b11001010
```

**Algorithm**:
1. Copy value from R16 to R17
2. Find rightmost '1' bit (bit position n)
3. Keep bits [n:0] unchanged
4. Flip all bits [7:n+1]

**Why This Works**: Two's complement is mathematically equivalent to:
- Invert all bits and add 1, OR
- Find rightmost 1, keep it and bits to the right, flip everything to the left

---

##  Running the Code

### Setup
1. Open Microchip Studio 7
2. Create new project: **File → New → Project**
3. Select **Assembler → AVR Assembler Project**
4. Choose device: **ATmega2560**

### Testing
1. Copy one `.asm` file into your project
2. **Build**: F7 or Build → Build Solution
3. **Start Debugging**: F5 or Debug → Start Debugging and Break
4. **Step Through**: F10 to execute line-by-line
5. **Watch Registers**: Debug → Windows → Processor → Registers

### Verification
- **Q1**: Check R4 and R5 for the sum
- **Q2**: Check R1 for the reset bit pattern
- **Q3**: Check R17 for the two's complement (R16 should be unchanged)

##  Key Concepts Learned

### AVR Instructions Used
- `LDI` - Load immediate value into register
- `ADD` - Add without carry
- `ADC` - Add with carry
- `MOV` - Copy register to register
- `LSR` - Logical shift right
- `LSL` - Logical shift left
- `ANDI` - Bitwise AND with immediate
- `ORI` - Bitwise OR with immediate
- `EOR` - Bitwise XOR (exclusive OR)
- `COM` - One's complement (flip all bits)
- `BRCC/BRCS` - Branch if carry clear/set
- `BREQ/BRNE` - Branch if equal/not equal

### Programming Techniques
- **Multi-precision arithmetic**: Extending operations beyond register width
- **Bit masking**: Isolating specific bits using AND/OR operations
- **Carry flag usage**: Propagating overflow between bytes
- **Loop-free algorithms**: Minimizing execution time
- **Edge case handling**: All zeros, all ones, single bit patterns

##  Complexity Analysis

| Problem | Time Complexity | Space Complexity |
|---------|----------------|------------------|
| 16-bit Addition | O(1) - 2 instructions | O(1) - 4 registers |
| Reset Rightmost | O(n) - n = bit width | O(1) - constant registers |
| Two's Complement | O(n) - n = bit width | O(1) - 2 registers |

##  Common Pitfalls

1. **Forgetting ADC in 16-bit addition**: Using ADD for both bytes loses the carry
2. **Modifying input registers**: Q3 requires R16 to remain unchanged
3. **Not handling edge cases**: All zeros or all ones can break naive algorithms
4. **Incorrect bit numbering**: AVR uses b7-b0, with b0 as LSB
5. **Infinite loops**: Forgetting the terminating `RJMP` causes runaway execution

##  Testing Strategy

Each solution was tested with:
- Provided test cases from assignment specification
- Edge cases (0x00, 0xFF)
- Random values
- Boundary conditions
- Values that could cause overflow/underflow


##  Skills Demonstrated

- Understanding of binary arithmetic at the hardware level
- Ability to work within architectural constraints (8-bit registers, limited instruction set)
- Algorithm design for bit manipulation
- Clear code documentation and commenting
- Flowchart creation for algorithm visualization
- Systematic testing and debugging

## References

- AVR Instruction Set Manual
- ATmega2560 Datasheet
- CSC 230 Lecture Notes (Numeration Systems, AVR Architecture)

---

**Author**: Armita Darbandi
**Course**: CSC 230 - Spring 2025  
**Institution**: University of Victoria
