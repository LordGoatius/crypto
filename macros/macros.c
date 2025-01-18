// #IF is true if nonzero
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#define BOOL_CHOICE(a, b, c) ((a && b) || (!a && c)) 
#define CHOICE(a, b, c) ((a &  b) |  (~a &  c))

#define BOOL_NAND(a, b) !(a && b)
#define BOOL_THREE_NAND(a, b, c) !(a && b && c)
#define BOOL_MEDIAN(a, b, c) BOOL_THREE_NAND(BOOL_NAND(a, b), BOOL_NAND(b, c), BOOL_NAND(a, c))

#define BITT_NAND(a, b) ~(a & b)
#define BITT_THREE_NAND(a, b, c) ~(a & b & c)
#define MEDIAN(a, b, c) ~(~(a & b) & ~(b & c) & ~(a & c))

// #define BITT_ROTATE_LEFT(a, b) ((a << b) | (a >> ((8*sizeof(a)) - b)))
#define LEFT_ROTATE(a, b) ((a << b) | (a >> ((8*sizeof(a)) - b)))
#define ROTATE(a, b) ((a >> b) | (a << ((8*sizeof(a)) - b)))

int main() {
    return 0;
}
