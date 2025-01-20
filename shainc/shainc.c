#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    u32 h0;
    u32 h1;
    u32 h2;
    u32 h3;
    u32 h4;
    u32 h5;
    u32 h6;
    u32 h7;
} Digest;

void resize(Array*);
// always 512 size array behind pointer;
void insert(Array*, u8*, usize);
void btle(u32*);

u32 rotate(u32, u32);
int main(int, char**);
void fill_arr(FILE*, Array*);
void padd_arr(Array*);
Digest sha256(Array*);

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
    // ugh not amoratized ik but it's gotta be exact
    // resizing is expensive but it can't be *that* bad
    // Not how I would do it if I wasn't rushing this out 4 weeks in advance
    // Fancy math with Array size would be better probably 
    // but this avoids the problem completely
    usize size = self->cap + 512;
    u8 *new = malloc(sizeof(u8) * size);
    memcpy(new, self->array, self->cap);
    free(self->array);
    self->cap = size;
    self->array = new;
}

void insert(Array* self, u8* data, usize size) {
    if (self->size + 512 > self->cap) {
        resize(self);
    }

    // copy 512 bytes of data, even if it's not 512 long.
    memcpy((self->array + self->size), data, 512);
    // Add the size of the data to the size of the array (512, unless last chunk);
    self->size += size;
}

void fill_arr(FILE* input, Array *output) {
    u8 temp[512] = { 0 };
    usize in_size;
    // I hope this stupid for loop works
    for (in_size = fread(temp, 1, 512, input); in_size > 0; in_size = fread(temp, 1, 512, input)) {
        insert(output, temp, in_size);
    }
}

// Wikipedia:
// begin with the original message of length L bits
// append a single '1' bit
// append K '0' bits, where K is the minimum number >= 0 such that (L + 1 + K + 64) is a multiple of 512
// append L as a 64-bit big-endian integer, making the total post-processed length a multiple of 512 bits
// such that the bits in the message are: <original message of length L> 1 <K zeros> <L as 64 bit integer> , (the number of bits will be a multiple of 512)
void padd_arr(Array *self) {
    // FIXME
    usize offset = (self->cap - self->size);
    if (offset != 0) {
        memset(self->array + self->size, 0, offset);
        self->array[self->size] = 1;
    }
}

Digest sha256(Array *self) {
    // Pseudocode taken from the Wikipedia Article on SHA-2
    usize total;
    for (total = 0; total < self->cap; total += 512) {
        u32 w[64] = { 0 };
        u32 chunk[16];
        usize i;
        // FIXME: Annoying warning
        memcpy(w, self->array + total, 512);
        for (i = 0; i < 16; i++) {
            w[i] = chunk[i];
        }
        for (i = 16; i < 64; i++) {
            u32 s0 = (rotate(w[i-15], 7) ^ rotate(w[i-15], 18) ^ (w[i-15] >> 3));
            // s0 := (w[i-15] rightrotate 7) xor (w[i-15] rightrotate 18) xor (w[i-15] rightshift  3)
            u32 s1 = (rotate(w[i-2], 17) ^ rotate(w[i-2], 19) ^ (w[i-2] >> 10));
            // s1 := (w[i-2] rightrotate 17) xor (w[i-2] rightrotate 19) xor (w[i-2] rightshift 10)
            w[i] = w [i-16] + s0 + w[i-7] + s1;
            // w[i] := w[i-16] + s0 + w[i-7] + s1
        }

        // a := h0
        // b := h1
        // c := h2
        // d := h3
        // e := h4
        // f := h5
        // g := h6
        // h := h7
        
        u32 a1 = h[0];
        u32 b1 = h[1];
        u32 c1 = h[2];
        u32 d1 = h[3];
        u32 e1 = h[4];
        u32 f1 = h[5];
        u32 g1 = h[6];
        u32 h1 = h[7];
        
        // for i from 0 to 63
        for (i = 0; i < 64; i++) {
            // S1 := (e rightrotate 6) xor (e rightrotate 11) xor (e rightrotate 25)
            u32 s1 = (rotate(e1, 6) ^ rotate(e1, 11) ^ rotate(e1, 25));
            // ch := (e and f) xor ((not e) and g)
            u32 ch = (e1 & f1) ^ ((~e1) & g1);
            // temp1 := h + S1 + ch + k[i] + w[i]
            u32 tmp0 = (h1 + s1 + ch + k[i] + w[i]);
            // S0 := (a rightrotate 2) xor (a rightrotate 13) xor (a rightrotate 22)
            u32 s0 = (rotate(a1, 2) ^ rotate(a1, 13) ^ rotate(a1, 22));
            // maj := (a and b) xor (a and c) xor (b and c)
            u32 maj = (a1 & b1) ^ (a1 & c1) ^ (b1 & c1);
            // temp2 := S0 + maj
            u32 tmp1 = s0 + maj;

            // h := g
            // g := f
            // f := e
            // e := d + temp1
            // d := c
            // c := b
            // b := a
            // a := temp1 + temp2 

            h1 = g1;
            g1 = f1;
            f1 = e1;
            e1 = d1 + tmp0;
            d1 = c1;
            c1 = b1;
            b1 = a1;
            a1 = tmp0 + tmp1;
        }
 

        h[0] = h[0] + a1;
        h[1] = h[1] + b1;
        h[2] = h[2] + c1;
        h[3] = h[3] + d1;
        h[4] = h[4] + e1;
        h[5] = h[5] + f1;
        h[6] = h[6] + g1;
        h[7] = h[7] + h1;
    }
    // it says this is  "big endian"
    // I don't know if that means big endian all the h's or the whole thing 😐
    // I hope the emoji breaks the c89 compliance
    // digest := hash := h0 append h1 append h2 append h3 append h4 append h5 append h6 append h7
    btle(&h[0]);
    btle(&h[1]);
    btle(&h[2]);
    btle(&h[3]);
    btle(&h[4]);
    btle(&h[5]);
    btle(&h[6]);
    btle(&h[7]);

    Digest dig = {
        .h0 = h[0],
        .h1 = h[1],
        .h2 = h[2],
        .h3 = h[3],
        .h4 = h[4],
        .h5 = h[5],
        .h6 = h[6],
        .h7 = h[7],
    };

    return dig;
}

// C style function name (to save bytes)
void btle(u32 *num) {
    u8 *arr = (u8*)num;
    arr[0] ^= arr[3];
    arr[3] ^= arr[0];
    arr[0] ^= arr[3];

    arr[1] ^= arr[2];
    arr[2] ^= arr[1];
    arr[1] ^= arr[2];
}

int main(int argc, char **argv) {
    FILE* input;
    if (argc != 2) {
        puts("No arguments provided");
        return 1;
    }
    input = fopen(argv[1], "rb");

    Array data = {
        .size = 0,
        .cap = 512,
        .array = malloc(512),
    };

    fill_arr(input, &data);
    padd_arr(&data);

    for (usize i = 0; i < data.cap; i++) {
        printf("%x", data.array[i]);
    }
    puts("\n");
    printf("Size: %lu\nCap: %lu\n", data.size, data.cap);

    Digest hash = sha256(&data);

    free(data.array);

    fclose(input);
    return 0;
}
