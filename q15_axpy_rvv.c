#include <stdint.h>
#include <riscv_vector.h>

void q15_axpy_rvv(const int16_t *a,
                  const int16_t *b,
                  int16_t *y,
                  int n,
                  int16_t alpha)
{
    while (n > 0) {
        size_t vl = __riscv_vsetvl_e16m1(n);
        vint16m1_t va = __riscv_vle16_v_i16m1(a, vl);
        vint16m1_t vb = __riscv_vle16_v_i16m1(b, vl);
        vint32m2_t vprod = __riscv_vwmul_vx_i32m2(vb, alpha, vl);
        vint32m2_t va_wide = __riscv_vwcvt_x_x_v_i32m2(va, vl);
        vint32m2_t va_q30 = __riscv_vsll_vx_i32m2(va_wide, 15, vl);
        vint32m2_t vsum = __riscv_vadd_vv_i32m2(va_q30, vprod, vl);
        vint16m1_t vy = __riscv_vnclip_wx_i16m1(vsum, 15, __RISCV_VXRM_RTZ, vl);
        __riscv_vse16_v_i16m1(y, vy, vl);
        a += vl;
        b += vl;
        y += vl;
        n -= vl;
    }
}
