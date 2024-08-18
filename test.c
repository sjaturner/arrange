#include <stdio.h>
#include <stdint.h>

struct test
{
    int i;
    char *str;
    char a[3][2];
    unsigned int ui;
};

void dump(void *base, size_t length)
{
    unsigned char *p = base;

    while (length--)
    {
        printf("%02x ", *p++);
    }
    printf("\n");
}

int main()
{
    struct test test = {
        .i = 123,
        .str = "foo",
        .a[0][1] = 1,
        .a[1][1] = 12,
        .a[2][0] = 20,
        .ui = 0x12345678,
    };
    dump(&test, sizeof(test));
    return 0;
}
