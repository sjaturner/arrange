#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

int sustain;
int quiet;
int extend;
int linear;
int prefix;
unsigned int fields;

int eof;

char *get_token(void)
{
    char *token = calloc(256 + 1, 1);

    if (1 == scanf("%256s", token))
    {
        return strdup(token);
    }
    else if (extend)
    {
        return strdup("@");
    }
    else if (sustain)
    {
        eof = 1;
        return strdup("?");
    }
    else
    {
        eof = 1;
        return 0;
    }
}

struct link
{
    struct link *link;
    char *tag;
    int *iter;
};

void output_link(struct link *link, int indices)
{
    if (!link)
    {
        return;
    }

    if (link->link)
    {
        output_link(link->link, indices);
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

void reverse_pointer_list(int elems, char **list)
{
    for (int index = 0; index < elems / 2; ++index)
    {
        char *tmp = list[index];

        list[index] = list[elems - 1 - index];
        list[elems - 1 - index] = tmp;
    }
}

struct output_controls
{
    int reverse;
    int hide;
    char format;
    int offset;
    int indices;
};

void output(int elems, int level, struct link *link, struct output_controls output_controls, char **prefix, unsigned *offset)
{
    char **list = calloc(elems, sizeof(char *));

    for (int index = 0; index < elems; ++index)
    {
        char *token = get_token();
        ++*offset;

        if (!level && prefix && !*prefix)
        {
            *prefix = strdup(token);
        }
        list[index] = token;
    }

    int visible = !output_controls.hide;

    if (visible)
    {
        if (prefix && *prefix)
        {
            printf("%s ", *prefix);
        }

        if (linear)
        {
            printf(" ");
        }
        else
        {
            for (int indent = 0; indent < level; ++indent)
            {
                printf("   ");
            }
        }

        if (output_controls.reverse)
        {
            reverse_pointer_list(elems, list);
        }

        int error = 0;

        if (output_controls.format)
        {
            if (output_controls.format == 'c')
            {
                printf("\"");
                for (int index = 0; index < elems; ++index)
                {
                    if (!list[index])
                    {
                        error = 1;
                        break;
                    }
                    char *end = 0;
                    uint64_t byte = strtol(list[index], &end, 16);

                    if (end - list[index] != 2)
                    {
                        error = 1;
                        break;
                    }

                    printf("%c", isprint(byte) ? (char)byte : '?');
                }
                printf("\" ");
            }
            else if (elems > 8)
            {
                error = 1;
            }
            else
            {
                uint64_t value = 0;
                uint64_t sign = 0;

                for (int index = 0; index < elems; ++index)
                {
                    if (!list[index])
                    {
                        error = 1;
                        break;
                    }
                    char *end = 0;
                    uint64_t byte = strtol(list[index], &end, 16);

                    if (end - list[index] != 2)
                    {
                        error = 1;
                        break;
                    }

                    sign = (uint64_t)0x80 << 8 * index;
                    value |= (byte & 0xff) << 8 * index;
                }

                if (error)
                {
                }
                else if (output_controls.format == 'd')
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
                    char fmt[0x20] = { };

                    sprintf(fmt, "0x%%0%d" PRIx64 " ", elems * 2);
                    printf(fmt, value);
                }
            }
        }

        if (!output_controls.format || error)
        {
            for (int index = 0; index < elems; ++index)
            {
                printf("%s ", list[index]);
            }
        }

        if (!quiet)
        {
            printf("# ");

            output_link(link, output_controls.indices);

            printf("%s ", output_controls.reverse ? "+r" : "");

            printf("# ");

            if (output_controls.format)
            {
                printf("+%c ", output_controls.format);
            }

            if (output_controls.offset)
            {
                printf("+o %u ", *offset);
            }
        }

        if (!linear)
        {
            printf("\n");
        }
    }

    for (int index = 0; index < elems; ++index)
    {
        if (list[index])
        {
            free(list[index]);
        }
    }

    free(list);

}

int set_output_controls(struct output_controls *output_controls, char flag)
{
    switch (flag)
    {
        case 'r': /* Reverse. */
            output_controls->reverse = 1;
            break;
        case 'f': /* Forward. */
            output_controls->reverse = 0;
            break;
        case 'h': /* Hide. */
            output_controls->hide = 1;
            break;
        case 's': /* Show. */
            output_controls->hide = 0;
            break;
        case 'u': /* Unsigned. */
        case 'd': /* Signed. */
        case 'x': /* Hexadecimal. */
        case 'c': /* Hexadecimal. */
            output_controls->format = flag;
            break;
        case 'n': /* No format. */
            output_controls->format = 0;
            break;
        case 'o': /* Add an offset field. */
            output_controls->offset = 1;
            break;
        case 'i': /* Add an offset field. */
            output_controls->indices = 1;
            break;
        default:
            return 0;
    }
    return 1;
}

int recurse(int argc, char *argv[], int arg, int level, struct link *link, struct output_controls output_controls_param, char **prefix, unsigned *offset)
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
                next = recurse(argc, argv, arg + 1, level + 1, &(struct link) {.link = link,.tag = tag,.iter = &iter }, output_controls, prefix, offset);
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
                output(count, level, &(struct link) {.link = link,.tag = tag,.iter = 0 }, output_controls, prefix, offset);
                output_controls = output_controls_param;
                tag = 0;
            }

            if (eof && !extend)
            {
                if (linear)
                {
                    printf("\n");
                }
                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            int control = 0;

            if (strlen(argv[arg]) >= 2 && argv[arg][0] == '+')
            {
                if (set_output_controls(&output_controls, argv[arg][1]))
                {
                    control = 1;
                }
            }

            if (!control)
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
    struct output_controls output_controls = { };

    while ((opt = getopt(argc, argv, "qlsepf:" "rfhsudxnoci")) != -1)
    {
        int not_handled = 0;
        switch (opt)
        {
            case 'q':
                quiet = 1;
                break;
            case 'l':
                linear = 1;
                break;
            case 's':
                sustain = 1;
                break;
            case 'e':
                extend = 1;
                break;
            case 'p':
                prefix = 1;
                break;
            case 'f':
                fields = atoi(optarg);
                break;
            default:
                not_handled = 1;
        }

        if (not_handled && !set_output_controls(&output_controls, opt))
        {
            exit(EXIT_FAILURE);
        }
    }

    do
    {
        char *prefix_buffer = 0;
        unsigned int offset_buffer = 0;
        recurse(argc - optind, argv + optind, 0, 0, 0, output_controls, prefix ? &prefix_buffer : 0, &offset_buffer);

        while (fields && offset_buffer++ < fields)
        {
            char *token = get_token();

            if (token)
            {
                free(token);
            }
        }

        if (prefix_buffer)
        {
            free(prefix_buffer);
        }

        if (linear)
        {
            printf("\n");
        }
    }
    while (sustain && !eof);

    return 0;
}
