#include "4096_t.h"
#include <assert.h>
#include <stdio.h>

/* print the big value as a string */
void seebig(u64 *a) {
    usize i;
    for (i = SIZE-1; i < SIZE ; i--) {
        printf("%016lx", a[i]); 
        if ((i % 8 == 0 && i)) {
            printf("\n");
        }       
    }
    printf("\n\n");
    return;
}

u64 bigsub(u64 *min, u64 *sub, u64 *dif) {
    usize i;
    u64 carry = 0, tmp;
    for (i = 0; i < SIZE; i++) {
        tmp = min[i] - sub[i] - carry;
        carry = min[i] < sub[i];
        dif[i] = tmp;
    }
    return carry;
}

u64 bigadd(u64 *in0, u64 *in1, u64 *sum) {
    usize i;
    u64 carry = 0, tmp;
    for (i = 0; i < SIZE; i++) {
        tmp = in0[i] + in1[i] + carry;
        carry = (tmp < in0[i] || tmp < in1[i]);
        /* carry = in0[i] < in1[i]; */
        sum[i] = tmp;
    }
    return carry;
}

u64 bigmul(u64 *in0, u64 *in1, u64 *out) {
    usize i, j;
    u64 wrk[SIZE*2 + 1]; /* 1 for overflow */
    u32 *al0 = (u32 *)in0, *al1 = (u32 *)in1 , *alw = (u32 *)wrk, *alo = (u32 *)out;
    u64 carry = 0, tmp;



    return carry;
}

int main(int argc, char **argv) {
    u64 min[SIZE], sub[SIZE], dif[SIZE];
    usize i;
    memset(min, 0x22, BYTES);
    memset(sub, 0x11, BYTES);
    bigsub(min, sub, dif);

    seebig(min);
    seebig(sub);
    seebig(dif);

    u64 a[SIZE], b[SIZE], c[SIZE];
    memset(a, 0, BYTES);
    memset(b, 0, BYTES);

    for (i = 0; i < SIZE; i++) {
        a[i] = i * 3;
        b[i] = i * 2;
    }

    bigadd(a, b, c);
    seebig(a);
    seebig(b);
    /* You may have written `bigeqs` or */
    for (i = 0; i < SIZE; i++) {
        printf("c[%02lx] = %02lx\n", i, c[i]);
        assert(c[i] == ((i * 3) + (i * 2)));
    }

    memset(a, 0x66, BYTES);
    memset(b, 0xaa, BYTES);
    memset(c, 0, BYTES);

    u64 carry = bigadd(a, b, c);

    seebig(a);
    seebig(b);
    seebig(c);
    printf("Carry: %lx\n\n", carry);

    memset(a, 0x00, BYTES);
    memset(b, 0xba, BYTES);
    memset(c, 0, BYTES);
    a[0] = 1;
    bigmul(a, b, c);
    seebig(c);

    return 0;
}
