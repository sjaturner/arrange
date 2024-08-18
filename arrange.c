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

struct output_controls
{
    int reverse;
    int hide;
};

void output(int elems, int level, struct link *link, struct output_controls output_controls)
{
    int visible = !output_controls.hide;

    if (visible)
    {
        for (int indent = 0; indent < level; ++indent)
        {
            printf("    ");
        }
    }

    char **list = calloc(elems, sizeof(char *));

    for (int index = 0; index < elems; ++index)
    {
        list[index] = get_token();
    }

    if (visible)
    {
        if (output_controls.reverse)
        {
            for (int index = elems - 1; index >= 0; --index)
            {
                printf("%s ", list[index]);
            }
        }
        else
        {
            for (int index = 0; index < elems; ++index)
            {
                printf("%s ", list[index]);
            }
        }
    }

    for (int index = 0; index < elems; ++index)
    {
        free(list[index]);
    }

    if (!quiet && visible)
    {
        printf(" #%s ", output_controls.reverse ? ":r" : ":f");
        output_link(link);
    }

    if (visible)
    {
        printf("\n");
    }
}

int recurse(int argc, char *argv[], int arg, int level, struct link *link, struct output_controls output_controls_param)
{
    char *tag = 0;
    int count = 0;
    struct output_controls output_controls = output_controls_param;

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
                next = recurse(argc, argv, arg + 1, level + 1, &(struct link) {.link = link,.tag = tag,.iter = &iter }, output_controls);
            }
            output_controls = output_controls_param;
            arg = next;
            tag = 0;
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
                output(count, level, &(struct link) {.link = link,.tag = tag,.iter = 0 }, output_controls);
                output_controls = output_controls_param;
            }

            if (eof)
            {
                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            if (strlen(argv[arg]) >= 2 && argv[arg][0] == '+')
            {
                switch (argv[arg][1])
                {
                    case 'r':
                        output_controls.reverse = 1;
                        break;
                    case 'h':
                        output_controls.hide = 1;
                        break;
                }
            }
            else
            {
                tag = argv[arg];
            }
        }
    }
    return arg;
}

int main(int argc, char *argv[])
{
    int opt = 0;
    while ((opt = getopt(argc, argv, "qrfi")) != -1)
    {
        switch (opt)
        {
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
        struct output_controls output_controls = { };
        recurse(argc - optind, argv + optind, 0, 0, 0, output_controls);
    }
    while (repeat);
    return 0;
}
