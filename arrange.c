#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct link
{
    struct link *link;
    char *tag;
    int iter;
};

char *get_token(void)
{
    char *token = calloc(256 + 1, 1);
    if (1 == scanf("%256s", token))
    {
        return strdup(token);
    }
    else
    {
        return 0;
    }
}

void output(int elems, int level, struct link *link)
{
    for (int indent = 0; indent < level; ++indent)
    {
        printf("    ");
    }

    for (int count = 0; count < elems; ++count)
    {
        char *token = get_token();
        if (token)
        {
            printf("%s ", token);
        }
    }
    printf("\n");
}

int recurse(int argc, char *argv[], int arg, int level, struct link *link)
{
    char *tag = 0;
    int count = 0;

    for (; arg < argc; ++arg)
    {
        if (0)
        {
        }
        else if (0 == strcmp(argv[arg], "{"))
        {
            int next = 0;
            for (int iter = 0; iter < count; ++iter)
            {
                next = recurse(arg, argv, arg + 1, level + 1, &(struct link) {.link = link,.tag = tag,.iter = iter });
            }
            arg = next;
        }
        else if (0 == strcmp(argv[arg], "}"))
        {
            return arg + 1;
        }
        else if (isdigit(argv[arg][0]))
        {
            count = atoi(argv[arg]);
            if (arg + 1 >= argc || strcmp(argv[arg + 1], "{"))
            {
                for (int emit = 0; emit < count; ++emit)
                {
                    output(count, level, &(struct link) {.link = link,.tag = tag,.iter = 0 });
                }
            }
        }
        else
        {
            tag = argv[arg];
        }
    }
    return arg;
}

int main(int argc, char *argv[])
{
    recurse(argc, argv, 0, 0, 0);
    return 0;
}
