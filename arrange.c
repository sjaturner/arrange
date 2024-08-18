#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct link
{
    struct link *link;
    char *tag;
    int *iter;
};

int repeat;
int quiet;
int fill;
int indices;

int eof;

char *get_token(void)
{
    char *token = calloc(256 + 1, 1);
    if (1 == scanf("%256s", token))
    {
        return strdup(token);
    }
    else if (fill)
    {
        return "@";
    }
    else if (repeat)
    {
        eof = 1;
        return "?";
    }
    else
    {
        eof = 1;
        return 0;
    }
}

void output_link(struct link *link)
{
    if (!link)
    {
        return;
    }

    if (link->link)
    {
        output_link(link->link);
    }

    if (link->tag)
    {
        printf("%s ", link->tag);
    }

    if (indices && link->iter)
    {
        printf("[%d] ", *link->iter);
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

    if (!quiet)
    {
        printf(" # ");
        output_link(link);
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
        else if (!strcmp(argv[arg], "{"))
        {
            int next = 0;
            for (int iter = 0; iter < count; ++iter)
            {
                next = recurse(argc, argv, arg + 1, level + 1, &(struct link) {.link = link,.tag = tag,.iter = &iter });
            }
            arg = next;
        }
        else if (!strcmp(argv[arg], "}"))
        {
            return arg;
        }
        else if (isdigit(argv[arg][0]))
        {
            count = atoi(argv[arg]);
            if (arg + 1 >= argc || strcmp(argv[arg + 1], "{"))
            {
                output(count, level, &(struct link) {.link = link,.tag = tag,.iter = 0 });
            }

            if (eof)
            {
                exit(EXIT_SUCCESS);
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
    int opt = 0;
    while ((opt = getopt(argc, argv, "qrfi")) != -1) {
        switch (opt) {
            case 'q':
                quiet = 1;
                break;
            case 'r':
                repeat = 1;
                break;
            case 'f':
                fill = 1;
                break;
            case 'i':
                indices = 1;
                break;
            default: /* '?' */
                exit(EXIT_FAILURE);
        }
    }

    do
    {
        recurse(argc - optind, argv + optind, 0, 0, 0);
    } while (repeat);
    return 0;
}
