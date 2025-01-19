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

void resize(Array*);
// always 512 size array behind pointer;
void insert(Array*, u8*, usize);

u32 rotate(u32, u32);
int main(int, char**);
void fill_arr(FILE*, Array*);
void padd_arr(Array*);

u32 k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

u32 h[8] = {
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
    0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
};

u32 rotate(u32 a, u32 b) {
    __asm__("rorl %%cl, %0" : "+r" (a) : "c" (b));
    return a;
}

void resize(Array *self) {
    usize size = self->cap * 2;
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

void padd_arr(Array *self) {
    usize offset = (self->cap - self->size);
    // create a copy
    u8 *cpy = malloc(self->size);
    memcpy(cpy, self->array, self->size);
    memset(self->array, 0, self->cap);
    memcpy(self->array + offset, cpy, self->size);
    if (offset != 0) {
        self->array[offset - 1] = 1;
    }
    free(cpy);
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

    free(data.array);

    u32 db = 0xdeadbeef;

    printf("%x\n", db);

    btle(&db);

    printf("%x\n", db);

    fclose(input);
    return 0;
}
