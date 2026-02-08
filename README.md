# RISC-V RVV Q15 AXPY (Saturating Fixed-Point)

A vectorized implementation of a **Q15 fixed-point AXPY** operation using the **RISC-V Vector Extension (RVV)**.  
This project was implemented as part of the **Audiomark / LFX Mentorship Coding Challenge**.

The implementation is:
- ✅ Bit-exact with a scalar reference
- ✅ Vector-length agnostic
- ✅ Portable across RV32 and RV64
- ✅ Uses RVV v1.0 intrinsics only

---

## Problem Statement

For vectors `a`, `b` and scalar `alpha` (all in **Q15 fixed-point**):

y[i] = sat_q15(a[i] + alpha × b[i])


Where:
- `a`, `b`, `y` are `int16_t` arrays
- `alpha` is a Q15 scalar (`int16_t`)
- `sat_q15` saturates to `[-32768, 32767]`

---

## Function Signature

```c
void q15_axpy_rvv(const int16_t *a,
                  const int16_t *b,
                  int16_t *y,
                  int n,
                  int16_t alpha);
Key Design Choices
Widening arithmetic
Multiplication is performed in 32-bit (Q30) to avoid overflow before saturation.

Correct Q-format scaling
Products are shifted right by 15 bits to return to Q15.

Saturating narrowing
Final results are clipped to the valid Q15 range using RVV narrowing intrinsics.

Vector-length agnostic loop
Uses vsetvl so the same binary runs efficiently on any RVV hardware.

Implementation Overview
High-level steps per iteration:

Load Q15 input vectors

Widen and multiply (int16 × int16 → int32)

Shift to restore Q15 scale

Widen and add a

Saturate and narrow back to int16

Store result

All arithmetic matches the scalar reference bit-for-bit.

Build Instructions
RV64
riscv64-unknown-elf-gcc \
  -O2 -march=rv64gcv -mabi=lp64d \
  src/q15_axpy_rvv.c src/test_harness.c \
  -o q15_axpy.elf
RV32
riscv32-unknown-elf-gcc \
  -O2 -march=rv32gcv -mabi=ilp32 \
  src/q15_axpy_rvv.c src/test_harness.c \
  -o q15_axpy.elf
Running (Simulator)
Spike
spike --isa=rv64gcv pk q15_axpy.elf
QEMU
qemu-riscv64 -cpu rv64,v=true,vlen=128 q15_axpy.elf
Testing
The test harness verifies:

Basic correctness

Saturation behavior

Edge cases (n=0, n=1, alpha=0)

Non-power-of-two lengths

Random stress testing

The vector implementation is validated against a scalar reference.
Notes
The implementation is portable and does not assume a fixed vector length.

No architecture-specific tuning is applied; correctness and clarity were prioritized.

The code follows RVV best practices for fixed-point DSP workloads.

Author
Varchas H V
GitHub: https://github.com/BitWiseBrain

License
MIT License
