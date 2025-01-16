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
    char (*obfuscate)(struct Rotor* self, char, size_t invert);
} Rotor;

// Rotate
void rotate(Rotor* self) {
    self->shift = (self->shift + 1) % 26;
}

// Group actions perform on some element in a group
struct GroupAction {
    // This is C, so our api verification process is code comments above a function pointer
    void* (*multiply)(void* self, void* elem);
    void* (*inverse)(void* self, void* elem);
    void* identity;
};

// Apply rotor to letter
char obfuscate(Rotor* self, char letter, size_t invert) {
    void* (*actions[2])(void*, void*) = {self->actions->multiply, self->actions->inverse};
    void* (*action)(void*, void*) = actions[invert];
    // apply shift
    letter = IDENTITY[(letter - 'A' + self->shift) % 26];

    letter = *(char*)action(self, &letter);

    // unapply shift
    letter = IDENTITY[((letter - 'A' + 26) - self->shift) % 26];
    return letter;
}

// groupelem is a char for a rotor, the group is the `self`
void* permute(void* self, void* elem) {
    // Self is a rotor
    char* key = ((Rotor*)self)->rotor;
    // Elem is a ptr to a struct with a pointer to a group element
    char value = *(char*)elem;
    // value is 'A' -> 'Z'. Convert to index by subtract 'A';
    size_t index = value - 'A';
    return &key[index];
}


// funcion that does the opposite of permute in the same action
void* invert(void* self, void* elem) {
    // Self is a rotor
    char* key = ((Rotor*)self)->invrs;
    // Elem is a ptr to a struct with a pointer to a group element
    char value = *(char*)elem;
    // value is 'A' -> 'Z'. Convert to index by subtract 'A';
    size_t index = value - 'A';
    return &key[index];
}

// We making a state machine with this one
// An engima machine is a set $\Epislon = \{ \delta_1, m, i, \Rho, \rho \}$
// Where 
// - $\delta_1$ is the encrypt/decrypt function
// - m contains the message to be encrypted or decrypted
// - i containts the machine location state in the message
// - $\Rho$ contains the rotors, which are are permutation functions and
// - $\rho$ is the reflect
typedef struct Enigma {
    void (*crypt)(struct Enigma* self);
    char* message;
    size_t curr;
    Rotor fast;
    Rotor medm;
    Rotor slow;
    Rotor refl;
} Enigma;

void crypt(Enigma* self) {
    // reset curr for reproducibility
    self->curr = 0;
    size_t len = strlen(self->message);

    Rotor* fast = &self->fast;
    Rotor* medm = &self->medm;
    Rotor* slow = &self->slow;
    Rotor* refl = &self->refl;

    // reset shift for same
    fast->shift = 0;
    medm->shift = 0;
    slow->shift = 0;

    while (self->curr < len) {
        // TODO: Actually write the logic
        // It's not even that hard I just need to do it
        fast->rotate(fast);
        if (fast->shift == 0) 
            medm->rotate(medm);
        if (medm->shift == 0 && fast->shift == 0)
            slow->rotate(slow);

        char letter = self->message[self->curr];
        letter = fast->obfuscate(fast, letter, 0);
        letter = medm->obfuscate(medm, letter, 0);
        letter = slow->obfuscate(slow, letter, 0);
        letter = refl->obfuscate(refl, letter, 0);
        letter = slow->obfuscate(slow, letter, 1);
        letter = medm->obfuscate(medm, letter, 1);
        letter = fast->obfuscate(fast, letter, 1);

        self->message[self->curr] = letter;

        self->curr += 1;
    }
}

// permute is implementation for rotor only
static struct GroupAction permutation = {
    .multiply = permute,
    .inverse  = invert,
    .identity = IDENTITY,
    // NOTE: Add inverse if function worked
};

int main(int argc, char **argv) {
    if (argc < 2) { return 1; }
    Enigma machine = {
        .crypt = crypt,
        .message = argv[1],
        .curr = 0,
        .fast = {
            .rotor = FAST,
            .invrs = INV_FAST,
            .shift = 0,
            .rotate = rotate,
            .obfuscate = obfuscate,
            .actions = &permutation
        },
        .medm = {
            .rotor = MEDIUM,
            .invrs = INV_MED,
            .shift = 0,
            .rotate = rotate,
            .obfuscate = obfuscate,
            .actions = &permutation
        },
        .slow = {
            .rotor = SLOW,
            .invrs = INV_SLOW,
            .shift = 0,
            .rotate = rotate,
            .obfuscate = obfuscate,
            .actions = &permutation
        },
        .refl = {
            .rotor = REFLECT,
            .invrs = REFLECT,
            .shift = 0,
            .rotate = NULL,
            .obfuscate = obfuscate,
            .actions = &permutation
        }
    };

    printf("%s\n", machine.message);

    machine.crypt(&machine);
    puts("\n");

    printf("%s\n", machine.message);

    machine.crypt(&machine);
    puts("\n");

    printf("%s\n", machine.message);

    return 0;
}
