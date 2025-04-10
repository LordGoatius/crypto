#include "list_t.h"

int main() {
    list_t list = list_new();
    list_print(list);
    int a = 0xdeadbeef;
    int b = 0xbeefdead;
    int c = 0xea5445ae;
    int d = 0x0000bbbb;
    int e = 0x52146e00;
    list_append(list, &a);
    list_append(list, &b);
    list_append(list, &c);
    list_append(list, &d);
    list_append(list, &e);
    list_insert(list, 0, NULL);
    list_print(list);
    // void list_extend(list_t l1, list_t l2);
    // void list_insert(list_t l, size_t i, void *x);
    // bool list_remove(list_t l, void *x);
    // void *list_pop(list_t l, size_t i);
    // void list_clear(list_t l);
    // size_t list_index(list_t l, void *x);
    // uint64_t list_count(list_t l, void *x);
    // void list_reverse(list_t l);
    // list_t list_copy(list_t l);

    list_free(list);
}
