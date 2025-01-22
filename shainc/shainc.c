#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SHA_SIZE (512 / 8)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   usize;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct Array {
    usize size;
    usize cap;
    u8*  array;
} Array;

void resize(Array*);
/* always 512 size array behind pointer; */
void insert(Array*, u8*, usize);
void btle(u32*);
void btlel(u64*);

u32 rotate(u32, u32);
int main(int, char**);
void fill_arr(FILE*, Array*);
void padd_arr(Array*);
void sha256(Array*);

u32 k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

u32 h[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
};

u32 rotate(u32 a, u32 b) {
    __asm__("rorl %%cl, %0" : "+r" (a) : "c" (b));
    return a;
}

void resize(Array *self) {
    /* I'm doing it in bytes now so I'm amoritzing it */
    usize size = self->cap * 2;
    u8 *new = malloc(sizeof(u8) * size);
    memcpy(new, self->array, self->cap);
    free(self->array);
    self->cap = size;
    self->array = new;
}

/* Insert `size` bytes from `data` into the array */
void insert(Array* self, u8* data, usize size) {
    if (self->size + size > self->cap) {
        resize(self);
    }

    /* copy 512 bytes of data, even if it's not 512 long. */
    memcpy((self->array + self->size), data, size);
    /* Add the size of the data to the size of the array (512, unless last chunk); */
    self->size += size;
}

void fill_arr(FILE* input, Array *output) {
    u8 temp[SHA_SIZE] = { 0 };
    usize in_size;
    /* I hope this stupid for loop works */
    for (in_size = fread(temp, 1, SHA_SIZE, input); in_size > 0; in_size = fread(temp, 1, SHA_SIZE, input)) {
        insert(output, temp, in_size);
    }
}

/* NOTE(CryptoStackExchange[https://crypto.stackexchange.com/questions/79734/how-to-pad-a-448-bit-message-for-sha256]):
 * The padding method for SHA-256 is (assuming you're byte oriented - it appears you are)
 * is 'append an 0x80 byte, and then add 0x00 byte's until the length modulo 64 bytes is 56.
 * This implies that if the original message length modulo 64 is 56 or larger,
 * you'll need to do one more hash compression operation. (Apr 6, 2020)
 * */
/* NOTE(Wikipedia):
 * begin with the original message of length L bits
 * append a single '1' bit
 * append K '0' bits, where K is the minimum number >= 0 such that (L + 1 + K + 64) is a multiple of 512
 * append L as a 64-bit big-endian integer, making the total post-processed length a multiple of 512 bits
 * such that the bits in the message are: <original message of length L> 1 <K zeros> <L as 64 bit integer> , (the number of bits will be a multiple of 512)
 * */
void padd_arr(Array *self) {
    u8 one = 0x80;
    u8 zero = 0x00;
    usize i;
    usize size;
    usize *ptr_size;
    size = self->size * 8;
    ptr_size = &size;

    insert(self, &one, 1);
    while (self->size % 64 != 56) {
        insert(self, &zero, 1);
    }
    /* gotta insert the value in big endian */

    for (i = 0; i < 8; i++) {
        insert(self, (((u8*)(ptr_size)) + 7 - i), 1);
    }
    assert(self->size % 64 == 0);
}

void sha256(Array *self) {
    /* Pseudocode taken from the Wikipedia Article on SHA-2 */
    usize total;

    for (total = 0; total < self->cap; total += SHA_SIZE) {
        u32 w[64] = { 0 };
        u32 chunk[16];
        usize i;
        u32 a1;
        u32 b1;
        u32 c1;
        u32 d1;
        u32 e1;
        u32 f1;
        u32 g1;
        u32 h1;
        memcpy(chunk, self->array + total, SHA_SIZE);
        for (i = 0; i < 16; i++) {
            btle(&chunk[i]);
            w[i] = chunk[i];
        }
        for (i = 16; i < 64; i++) {
            u32 s0 = (rotate(w[i-15], 7) ^ rotate(w[i-15], 18) ^ (w[i-15] >> 3));
            /* s0 := (w[i-15] rightrotate 7) xor (w[i-15] rightrotate 18) xor (w[i-15] rightshift 3) */
            u32 s1 = (rotate(w[i-2], 17) ^ rotate(w[i-2], 19) ^ (w[i-2] >> 10));
            /* s1 := (w[i-2] rightrotate 17) xor (w[i-2] rightrotate 19) xor (w[i-2] rightshift 10) */
            w[i] = w[i-16] + s0 + w[i-7] + s1;
            /* w[i] := w[i-16] + s0 + w[i-7] + s1 */
        }

        /* a := h0 */
        /* b := h1 */
        /* c := h2 */
        /* d := h3 */
        /* e := h4 */
        /* f := h5 */
        /* g := h6 */
        /* h := h7 */

        /* NOTE: Big endian */
        a1 = h[0];
        b1 = h[1];
        c1 = h[2];
        d1 = h[3];
        e1 = h[4];
        f1 = h[5];
        g1 = h[6];
        h1 = h[7];
        
        /* for i from 0 to 63 */
        for (i = 0; i < 64; i++) {
            u32 s1;
            u32 ch;
            u32 tmp0;
            u32 s0;
            u32 maj;
            u32 tmp1;

            /* NOTE: Curr in little endian */
            /* S1 := (e rightrotate 6) xor (e rightrotate 11) xor (e rightrotate 25) */
            s1 = (rotate(e1, 6) ^ rotate(e1, 11) ^ rotate(e1, 25)); /* NOTE: Should stay in big endian */
            /* ch := (e and f) xor ((not e) and g) */
            ch = (e1 & f1) ^ ((~e1) & g1);
            /* temp1 := h + S1 + ch + k[i] + w[i] */
            tmp0 = (h1 + s1 + ch + k[i] + w[i]);
            /* S0 := (a rightrotate 2) xor (a rightrotate 13) xor (a rightrotate 22) */
            s0 = (rotate(a1, 2) ^ rotate(a1, 13) ^ rotate(a1, 22));
            /* maj := (a and b) xor (a and c) xor (b and c) */
            maj = (a1 & b1) ^ (a1 & c1) ^ (b1 & c1);
            /* temp2 := S0 + maj */
            tmp1 = s0 + maj;

            /* h := g */
            /* g := f */
            /* f := e */
            /* e := d + temp1 */
            /* d := c */
            /* c := b */
            /* b := a */
            /* a := temp1 + temp2  */

            h1 = g1;
            g1 = f1;
            f1 = e1;
            e1 = d1 + tmp0;
            d1 = c1;
            c1 = b1;
            b1 = a1;
            a1 = tmp0 + tmp1;
        }

        h[0] += a1;
        h[1] += b1;
        h[2] += c1;
        h[3] += d1;
        h[4] += e1;
        h[5] += f1;
        h[6] += g1;
        h[7] += h1;
    }
    /* it says this is  "big endian" 
     * I don't know if that means big endian all the h's or the whole thing 😐 
     * I hope the emoji breaks the c89 compliance
     * digest := hash := h0 append h1 append h2 append h3 append h4 append h5 append h6 append h7 */
    /* NOTE: Already big endia */
}

/* Cstyle function name (to save bytes) */
void btle(u32 *num) {
    u8 *arr = (u8*)num;
    arr[0] ^= arr[3];
    arr[3] ^= arr[0];
    arr[0] ^= arr[3];

    arr[1] ^= arr[2];
    arr[2] ^= arr[1];
    arr[1] ^= arr[2];
}

void btlel(u64* num) {
    u8 *arr = (u8*)num;
    arr[0] ^= arr[7];
    arr[7] ^= arr[0];
    arr[0] ^= arr[7];

    arr[1] ^= arr[6];
    arr[6] ^= arr[1];
    arr[1] ^= arr[6];

    arr[2] ^= arr[5];
    arr[5] ^= arr[2];
    arr[2] ^= arr[5];

    arr[3] ^= arr[4];
    arr[4] ^= arr[3];
    arr[3] ^= arr[4];
}

int main(int argc, char **argv) {
    FILE* input;
    Array data;
    usize i;

    if (argc != 2) {
        puts("No arguments provided");
        return 1;
    }
    input = fopen(argv[1], "rb");

    data.size = 0;
    data.cap = SHA_SIZE;
    data.array = malloc(SHA_SIZE);

    fill_arr(input, &data);
    padd_arr(&data);

    sha256(&data);

    for (i = 0; i < 8; i++) {
        printf("%08x", h[i]);
    }

    printf("  %s\n", argv[1]);

    free(data.array);
    fclose(input);
    return 0;
}
