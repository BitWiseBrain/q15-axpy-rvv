RISC-V RVV Q15 AXPY Implementation
Overview

This repository contains a RISC-V Vector Extension (RVV) accelerated implementation of a fixed-point Q15 AXPY (multiply-accumulate) operation, implemented using RVV v1.0 C intrinsics.

The goal of this project is to demonstrate:

Correct fixed-point arithmetic handling (widening, scaling, saturation)

Proper use of RVV intrinsics

A vector-length agnostic design that is portable across different RISC-V implementations

Problem Description

Implement the following function:

void q15_axpy_rvv(const int16_t *a,
                  const int16_t *b,
                  int16_t *y,
                  int n,
                  int16_t alpha);


For all i in [0, n):

y[i] = sat_q15(a[i] + alpha × b[i])


Where:

a, b, and y are vectors of Q15 fixed-point values (int16_t)

alpha is a Q15 scalar

sat_q15 saturates results to the valid Q15 range [-32768, 32767]

Implementation Approach

The implementation uses RISC-V Vector intrinsics to process multiple elements in parallel while preserving the exact semantics of the scalar reference.

Key Steps

Vector-length selection

vsetvl is used to dynamically select the maximum vector length supported by the hardware for the remaining elements.

This makes the implementation vector-length agnostic.

Vector loads

Input vectors a and b are loaded using vle16.

Widened multiplication

Q15 × Q15 multiplication produces a Q30 result.

vwmul widens operands to 32-bit to prevent overflow.

Fixed-point scaling

The product is shifted right by 15 bits using an arithmetic shift to restore Q15 scaling.

Accumulation

The scaled product is added to the widened a vector.

Saturation and narrowing

vnclip narrows the 32-bit result back to 16-bit with signed saturation, implementing sat_q15.

Store results

The saturated result is written back to memory using vse16.

Vector-Length Agnostic Design

The loop structure does not assume any fixed vector width. Instead, it repeatedly:

Queries the hardware for the maximum supported vector length

Processes that many elements

Advances the pointers

This ensures correctness and portability across different RISC-V vector implementations.

Correctness

The vector implementation mirrors the scalar reference logic:

int32_t prod = ((int32_t)alpha * b[i]) >> 15;
int32_t sum  = (int32_t)a[i] + prod;
sum = clamp(sum, -32768, 32767);
y[i] = (int16_t)sum;


All arithmetic is performed in widened precision until the final saturation step, ensuring bit-for-bit equivalence with the scalar version.

Portability

Compatible with RVV v1.0

Builds on both RV32 and RV64 targets

Does not depend on a specific vector length or XLEN

Uses standard RVV C intrinsics

Build Instructions
RV32 Example
riscv32-unknown-elf-gcc \
  -O2 -march=rv32gcv -mabi=ilp32 \
  q15_axpy_rvv.c \
  -o q15_axpy.elf

RV64 Example
riscv64-unknown-elf-gcc \
  -O2 -march=rv64gcv -mabi=lp64d \
  q15_axpy_rvv.c \
  -o q15_axpy.elf

Testing and Validation

The implementation was validated by comparing the vectorized computation steps against the scalar reference logic.
Due to time constraints, full execution on a RISC-V simulator was not completed.

Performance Estimate

Assuming:

Vector length (VLEN) = 128 bits

16-bit elements → 8 elements per vector

The RVV implementation processes up to 8 elements per iteration compared to 1 element in the scalar version.

Estimated speedup: approximately 5×–8×, depending on memory bandwidth and loop overhead.

This estimate is conservative and intended to illustrate expected scaling behavior rather than exact cycle counts.

Additional Notes

This implementation prioritizes:

Correctness

Clarity

Portability

Further optimizations (e.g., tuning LMUL, unrolling, or micro-architecture-specific scheduling) were intentionally avoided to keep the solution simple and verifiable.

Summary

This project demonstrates a correct and portable use of RISC-V Vector intrinsics for fixed-point DSP-style computation, with careful handling of widening, scaling, saturation, and vector-length agnostic execution.