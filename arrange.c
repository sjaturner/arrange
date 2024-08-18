#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
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
    char format;
};

void reverse_pointer_list(int elems, char **list)
{
    for (int index = 0; index < elems / 2; ++index)
    {
        char *tmp = list[index];

        list[index] = list[elems - 1 - index];
        list[elems - 1 - index] = tmp;
    }
}

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

    if (output_controls.reverse)
    {
        reverse_pointer_list(elems, list);
    }

    if (visible)
    {
        if (output_controls.format)
        {
            int error = 0;
            switch (elems)
            {
                case 1:
                case 2:
                case 4:
                case 8:
                    break;
                default:
                    error = 1;
            }

            if (error)
            {
                printf("? ");
            }
            else
            {
                uint64_t value = 0;
                uint64_t sign = 0;
                for (int index = 0; index < elems; ++index)
                {
                    uint64_t byte = strtol(list[index], 0, 16);

                    sign = (uint64_t)0x80 << 8 * index;
                    value |= (byte & 0xff) << 8 * index;
                }

                if (output_controls.format == 'd')
                {
                    int64_t output = (value & sign) && elems < 8 ? value - (sign << 1) : value;
                    printf("%" PRId64 " ", output);
                }
                else if (output_controls.format == 'u')
                {
                    printf("%" PRIu64 " ", value);
                }
                else if (output_controls.format == 'x')
                {
                    char *fmt = 0;
                    switch (elems)
                    {
                        case 1:
                            fmt = "0x%02" PRIx64 " ";
                            break;
                        case 2:
                            fmt = "0x%04" PRIx64 " ";
                            break;
                        case 4:
                            fmt = "0x%08" PRIx64 " ";
                            break;
                        case 8:
                            fmt = "0x%016" PRIx64 " ";
                            break;
                        default:
                            error = 1;
                    }
                    printf(fmt, value);
                }
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
                    case 'u':
                    case 'd':
                    case 'x':
                        output_controls.format = argv[arg][1];
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
