#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

void q15_axpy_scalar(const int16_t *a, const int16_t *b, int16_t *y, 
                     int n, int16_t alpha) {
    for (int i = 0; i < n; i++) {
        int32_t prod = ((int32_t)alpha * (int32_t)b[i]) >> 15;
        int32_t sum = (int32_t)a[i] + prod;
        
        if (sum > 32767) sum = 32767;
        if (sum < -32768) sum = -32768;
        
        y[i] = (int16_t)sum;
    }
}

void q15_axpy_rvv(const int16_t *a, const int16_t *b, int16_t *y, 
                  int n, int16_t alpha);

int verify_results(const int16_t *expected, const int16_t *actual, int n) {
    for (int i = 0; i < n; i++) {
        if (expected[i] != actual[i]) {
            printf("MISMATCH at index %d: expected %d, got %d\n", 
                   i, expected[i], actual[i]);
            return 0;
        }
    }
    return 1;
}

void test_basic() {
    printf("Test 1: Basic functionality\n");
    int n = 16;
    int16_t a[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int16_t b[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int16_t alpha = 100;
    int16_t y_expected[16], y_actual[16];
    
    q15_axpy_scalar(a, b, y_expected, n, alpha);
    q15_axpy_rvv(a, b, y_actual, n, alpha);
    
    if (verify_results(y_expected, y_actual, n)) {
        printf("done PASSED\n\n");
    } else {
        printf("error FAILED\n\n");
    }
}

void test_saturation() {
    printf("Test 2: Saturation (overflow)\n");
    int n = 8;
    int16_t a[8] = {32767, -32768, 30000, -30000, 32000, -32000, 0, 0};
    int16_t b[8] = {32767, -32768, 32767, -32768, 1000, -1000, 32767, -32768};
    int16_t alpha = 32767; 
    int16_t y_expected[8], y_actual[8];
    
    q15_axpy_scalar(a, b, y_expected, n, alpha);
    q15_axpy_rvv(a, b, y_actual, n, alpha);
    
    if (verify_results(y_expected, y_actual, n)) {
        printf("done PASSED\n\n");
    } else {
        printf("error FAILED\n\n");
    }
}

void test_edge_cases() {
    printf("Test 3: Edge cases\n");
    
    int16_t dummy[1];
    q15_axpy_rvv(dummy, dummy, dummy, 0, 100);
    printf("  n=0: done\n");
    
    int16_t a1[1] = {100}, b1[1] = {200}, y1[1], y1_exp[1];
    q15_axpy_scalar(a1, b1, y1_exp, 1, 300);
    q15_axpy_rvv(a1, b1, y1, 1, 300);
    if (y1[0] == y1_exp[0]) {
        printf("  n=1: done\n");
    } else {
        printf("  n=1: error (expected %d, got %d)\n", y1_exp[0], y1[0]);
    }
    
    int16_t a0[4] = {1, 2, 3, 4}, b0[4] = {5, 6, 7, 8}, y0[4], y0_exp[4];
    q15_axpy_scalar(a0, b0, y0_exp, 4, 0);
    q15_axpy_rvv(a0, b0, y0, 4, 0);
    if (verify_results(y0_exp, y0, 4)) {
        printf("  alpha=0: done\n");
    } else {
        printf("  alpha=0: error\n");
    }
    
    printf("\n");
}

void test_non_power_of_two() {
    printf("Test 4: Non-power-of-two sizes (VL-agnostic)\n");
    int sizes[] = {7, 13, 31, 63, 100, 127, 255};
    
    for (int s = 0; s < 7; s++) {
        int n = sizes[s];
        int16_t *a = malloc(n * sizeof(int16_t));
        int16_t *b = malloc(n * sizeof(int16_t));
        int16_t *y_expected = malloc(n * sizeof(int16_t));
        int16_t *y_actual = malloc(n * sizeof(int16_t));
        
        for (int i = 0; i < n; i++) {
            a[i] = rand() % 10000 - 5000;
            b[i] = rand() % 10000 - 5000;
        }
        int16_t alpha = rand() % 32768;
        
        q15_axpy_scalar(a, b, y_expected, n, alpha);
        q15_axpy_rvv(a, b, y_actual, n, alpha);
        
        if (verify_results(y_expected, y_actual, n)) {
            printf("  n=%d: done\n", n);
        } else {
            printf("  n=%d: error\n", n);
        }
        
        free(a); free(b); free(y_expected); free(y_actual);
    }
    printf("\n");
}

void test_random_stress() {
    printf("Test 5: Random stress test (1000 iterations)\n");
    srand(time(NULL));
    
    int failures = 0;
    for (int iter = 0; iter < 1000; iter++) {
        int n = (rand() % 256) + 1;
        int16_t *a = malloc(n * sizeof(int16_t));
        int16_t *b = malloc(n * sizeof(int16_t));
        int16_t *y_expected = malloc(n * sizeof(int16_t));
        int16_t *y_actual = malloc(n * sizeof(int16_t));
        
        for (int i = 0; i < n; i++) {
            a[i] = rand() % 65536 - 32768;
            b[i] = rand() % 65536 - 32768;
        }
        int16_t alpha = rand() % 65536 - 32768;
        
        q15_axpy_scalar(a, b, y_expected, n, alpha);
        q15_axpy_rvv(a, b, y_actual, n, alpha);
        
        if (!verify_results(y_expected, y_actual, n)) {
            failures++;
            if (failures <= 5) {
                printf("  Failure %d: n=%d, alpha=%d\n", failures, n, alpha);
            }
        }
        
        free(a); free(b); free(y_expected); free(y_actual);
    }
    
    if (failures == 0) {
        printf("done All 1000 iterations PASSED\n\n");
    } else {
        printf("error %d/%d iterations FAILED\n\n", failures, 1000);
    }
}

int main() {
    printf("======================================\n");
    printf("Q15 AXPY RVV Implementation Tests\n");
    printf("======================================\n\n");
    
    test_basic();
    test_saturation();
    test_edge_cases();
    test_non_power_of_two();
    test_random_stress();
    
    printf("======================================\n");
    printf("Testing complete\n");
    printf("======================================\n");
    
    return 0;
}
