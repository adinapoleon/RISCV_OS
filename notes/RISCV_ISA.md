# RISC-V 32-bit Instruction Set (RV32I)

This document serves as a reference for the base integer instruction set used in this project.

## Register ABI Convention

| Register | ABI Name | Description | Saver |
| :--- | :--- | :--- | :--- |
| x0 | zero | Hard-wired zero | -- |
| x1 | ra | Return address | Caller |
| x2 | sp | Stack pointer | Callee |
| x3 | gp | Global pointer | -- |
| x4 | tp | Thread pointer | -- |
| x5-x7 | t0-t2 | Temporaries | Caller |
| x8 | s0 / fp | Saved register / Frame pointer | Callee |
| x9 | s1 | Saved register | Callee |
| x10-x11 | a0-a1 | Function arguments / Return values | Caller |
| x12-x17 | a2-a7 | Function arguments | Caller |
| x18-x27 | s2-s11 | Saved registers | Callee |
| x28-x31 | t3-t6 | Temporaries | Caller |

## Common Instructions

### Arithmetic & Logic
- `add rd, rs1, rs2`: rd = rs1 + rs2
- `sub rd, rs1, rs2`: rd = rs1 - rs2
- `addi rd, rs1, imm`: rd = rs1 + sign-extended immediate
- `and rd, rs1, rs2`: rd = rs1 & rs2
- `or rd, rs1, rs2`: rd = rs1 | rs2
- `xor rd, rs1, rs2`: rd = rs1 ^ rs2
- `li rd, imm`: Load immediate (pseudo-instruction)

### Memory Access
- `lw rd, offset(rs1)`: Load 32-bit word from memory at `rs1 + offset`
- `sw rs2, offset(rs1)`: Store 32-bit word from `rs2` to memory at `rs1 + offset`

### Branch & Jump
- `beq rs1, rs2, label`: Branch if rs1 == rs2
- `bne rs1, rs2, label`: Branch if rs1 != rs2
- `jal rd, label`: Jump and link (save PC+4 to rd, jump to label)
- `jalr rd, offset(rs1)`: Jump and link register
- `call label`: Pseudo-instruction for function calls
- `ret`: Pseudo-instruction for function return (jumps to `ra`)

### CSR (Control and Status Register) Instructions
- `csrr rd, csr`: Read CSR into rd
- `csrw csr, rs1`: Write rs1 to CSR
- `csrs csr, rs1`: Set bits in CSR (CSR = CSR | rs1)
- `csrc csr, rs1`: Clear bits in CSR (CSR = CSR & ~rs1)
- `csrrw rd, csr, rs1`: Atomic Read and Write CSR
- `mret`: Return from Machine-mode trap

## Instruction Formats
RV32I instructions are all 32 bits long and follow several fixed formats (R, I, S, B, U, J) to simplify hardware decoding.

- **R-type**: Register-Register operations.
- **I-type**: Register-Immediate operations, Loads, JALR.
- **S-type**: Stores.
- **B-type**: Conditional branches.
- **U-type**: Large immediates (LUI, AUIPC).
- **J-type**: Unconditional jumps (JAL).
