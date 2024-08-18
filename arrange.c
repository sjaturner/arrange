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
        else if (0 == strcmp(argv[argc], "{"))
        {
            int next = 0;
            for (int iter = 0; iter < count; ++iter)
            {
                next = recurse(argc, argv, arg + 1, level + 1, &(struct link)
                    {.link = link,.tag = tag,.iter = iter });
            }
            arg = next;
        }
        else if (0 == strcmp(argv[argc], "}"))
        {
            return arg + 1;
        }
        else if (isdigit(argv[argc]))
        {
            count = atoi(argv[argc]);
            if (strcmp(argv[argc + 1], "{"))
            {
                output(count, level, &(struct link)
                    {.link = link,.tag = tag,.iter = 0 });
            }
        }
        else
        {
            tag = argv[argc];
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    recurse(argc, argv, 0, 0, 0);
    return 0;
}
