// NOTE(Calvin): I just wanted to have fun with function pointers tbh
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MESSAGE "LOREMIPSUMDOLORSITAMETCONSECTETURADIPISCINGELITSEDDOEIUSMODTEMPORINCIDIDUNTUTLABOREETDOLOREMAGNAALIQUAUTENIMADMINIMVENIAMQUISNOSTRUDEXERCITATIONULLAMCOLABORISNISIUTALIQUIPEXEACOMMODOCONSEQUATDUISAUTEIRUREDOLORINREPREHENDERITINVOLUPTATEVELITESSECILLUMDOLOREEUFUGIATNULLAPARIATUREXCEPTEURSINTOCCAECATCUPIDATATNONPROIDENTSUNTINCULPAQUIOFFICIADESERUNTMOLLITANIMIDESTLABORUM"

// Some things to say: (In Disjoint Cycle Notation)
// IDENTITY:     (A)(B)(C)(D)(E)(F)(G)(H)(I)(J)(K)(L)(M)(N)(O)(P)(Q)(R)(S)(T)(U)(V)(W)(X)(Y)(Z)
// (Each letter gets mapped to itself with IDENTITY)
#define IDENTITY "ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
// FAST:         (ABDHPEJT)(CFLVMZOYQIRWUKXSG)(N)
#define FAST     "BDFHJLCPRTXVZNYEIWGAKMUSQO"
// MEDIUM:       (A)(BJ)(CDKLHUP)(ESZ)(FIXVYOMW)(GR)(NT)(Q)
#define MEDIUM   "AJDKSIRUXBLHWTMCQGZNPYFVOE"
// SLOW:         (AELTPHQXRU)(BKNW)(CMOY)(DFG)(IV)(JZ)(S)
#define SLOW     "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
// NOTE: What makes reflect interesting is hard to describe in permutation form. 
// In Disjoint Cycle Notation, however, it immediately becomes more obvious.
// Each letter is in a cycle of order 2! This is why it's called reflect, and it's 
// not the only rotor with this property. Now - how can you use this form to calculate
// how many possible reflect rotors there are?
// REFLECT:      (AI)(BX)(CU)(DH)(EF)(GZ)(JO)(KM)(LT)(NQ)(PW)(RS)(VY)
#define REFLECT  "IXUHFEZDAOMTKQJWNSRLCYPBVG"
// NOTE: Reflect, because of this property, is its own inverse
// Inverses become slightly easier to understand in this notation too, I think. 
// An inverse of A is a rotor, A^-1, such that when you put any letter into A, then A^-1, you get the same letter back.
// (Interestingly, for permutation A, the inverse of A^-1 is just A)
// If we do function composotion on our permutations, it is the permutation function f^-1, such that the function
// (f^-1 ∘ f) results in the identity permutation function

//== Inverses ==//
// NOTE: For inverses, simply reverse the order of the cycles in DCN
#define INV_FAST "TAGBPCSDQEUFVNZHYIXJWLRKOM"
// FAST_INV:     (TJEPHDBA)(GSXKUWRIQYOZMVLFC)(N)
#define INV_MED  "AJPCZWRLFBDKOTYUQGENHXMIVS"
// MEDIUM_INV:   (A)(JB)(PUHLKDC)(ZSE)(WMOYVXIF)(GR)(NT)(Q)
#define INV_SLOW "UWYGADFPVZBECKMTHXSLRINQOJ"
// SLOW_INV:     (URXQHPTLEA)(WNKB)(YOMC)(DGF)(IV)(JZ)(S)

typedef struct Rotor {
    char rotor[27];
    char invrs[27];
    size_t shift;
    struct GroupAction* actions;
    void (*rotate)(struct Rotor* self);
} Rotor;

// Rotate
void rotate(Rotor* self) {
    self->shift = (self->shift + 1) % 26;
}

// Group actions perform on some element in a group
struct GroupAction {
    // This is C, so our api verification process is code comments above a function pointer
    void* (*multiply)(void* self, void* elem);
    void* (*inverse)(void* self);
    void* identity;
};

// groupelem is a char for a rotor, the group is the `self`
void* permute(void* self, void* elem) {
    // Self is a rotor
    char* key = ((Rotor*)self)->rotor;
    // Elem is a ptr to a struct with a pointer to a group element
    char value = *(char*)elem;
    // value is 'a' -> 'z'. Convert to index by subtract 'a';
    size_t index = value - 'A';
    return &key[index];
}

// NOTE: Theoretically you can calculate an inverse and I wanted more than one
// function for my poor vtable, it was hungy.
// It only gets a function stub though.
void* inverse(void* self);

// We making a state machine with this one
typedef struct Enigma {
    void (*crypt)(struct Enigma* self);
    char* message;
    size_t curr;
    Rotor fast;
    Rotor medm;
    Rotor slow;
} Enigma;

void crypt(struct Enigma* self) {
    // TODO
}

// permute is implementation for rotor only
static struct GroupAction permutation = {
    .multiply = permute,
    .identity = IDENTITY,
    // NOTE: Add inverse if function worked
};

int main() {
    Enigma machine = {
        .crypt = crypt,
        .message = MESSAGE,
        .curr = 0,
        .fast = {
            .rotor = FAST,
            .invrs = INV_FAST,
            .shift = 0,
            .actions = &permutation
        },
        .medm = {
            .rotor = MEDIUM,
            .invrs = INV_MED,
            .shift = 0,
            .actions = &permutation
        },
        .slow = {
            .rotor = SLOW,
            .invrs = INV_SLOW,
            .shift = 0,
            .actions = &permutation
        },
    };

    printf("%s\n", machine.message);

    machine.crypt(&machine);

    printf("%s\n", machine.message);

    machine.crypt(&machine);

    printf("%s\n", machine.message);

    return 0;
}
