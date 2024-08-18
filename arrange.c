#include <stdio.h>

struct link
{
    struct link *link;
    char *tag;
};

int recurse(int argc, char *argv[], struct link *link)
{
}

int main(int argc, char *argv[])
{
    char token[0x100] = { };
    while(scanf("%s", &token) != EOF)
    {
        printf("token:%s\n", token);
    }
    return 0;
}
