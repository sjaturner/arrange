# Arrange Tokens From Standard Input in a Structured Form Controlled by Command Line Arguments

_A debugging tool_

## Introduction

This program is a command line too which operates on a stream of space
separated tokens and arranges them in a structured format which is sent
to stdout.

In it's plain form the tokens are not interpreted and are emitted without
modification.

    :; echo this line contains five things but this contains four | ./arrange -q 5 4
    this line contains five things
    but this contains four

There's a way of imposing structure on the output:

    :; echo first line and second line and third line | ./arrange -q 2 2 { 3 }
    first line
       and second line
       and third line

The numbers and brackets say that there are two elements, followed by set of 
two groups of three elements.

As you might expect, these things are nestable:

    :; echo first line and second line and third line | ./arrange -q 2 2 { 1 1 { 2 } }
    first line
       and
          second line
       and
          third line

## Motivation

Despite appearances, this tool has practical applications. I wrote it
because I have become used to working with binary data in a particular
format.

    :; echo make octets | od -tx1 | grep ' ' | cut -d ' ' -f2- | xargs
    6d 61 6b 65 20 6f 63 74 65 74 73 0a

_Hexadecimal bytes on a line._

It's so easy to put something like this into some your code:

    void dump(void *base, size_t length)
    {
        unsigned char *p = base;

        while (length--)
        {
            printf("%02x ", *p++);
        }
        printf("\n");
    }

Or peek into the memory of a running process and get live data in that
form (more of this in another repo).  Or even put blocks of data into
a UDP datagram and have a receiver output in the hexadecimal octet format.

Perhaps the data is a C structure? You can write something code to read
the octets and pretty print the structures. I've made various attempts
at that, including https://github.com/sjaturner/cdump which uses the
DWARF information to make JSON.

However, for debugging, it's sometimes easier to do something ad hoc. 
My observation is that C structures are often small and simple.

## A small example

This directory contains a file called test.c, it defines a simple C structure, 
initialises it and then dumps its contents using the dump function above.

For brevity, here's the structure and its initialisation:

    struct test
    {
        int i;
        char *str;
        char a[3][2];
        unsigned int ui;
    };

    struct test test = {
        .i = 123,
        .str = "foo",
        .a[0][1] = 1,
        .a[1][1] = 12,
        .a[2][0] = 20,
        .ui = 0x12345678,
    };
    
When I run test, I see the following memory dump of the structure.

    :; ./test
    7b 00 00 00 00 00 00 00 04 a0 71 32 a4 55 00 00 00 01 00 0c 14 00 00 00 78 56 34 12 00 00 00 00

I am interested in its layout, so I run pahole (https://linux.die.net/man/1/pahole), empty lines 
suppressed again for brevity.

    :; pahole test | grep -v '^$'
    struct test {
            int                        i;                    /*     0     4 */
            /* XXX 4 bytes hole, try to pack */
            char *                     str;                  /*     8     8 */
            char                       a[3][2];              /*    16     6 */
            /* XXX 2 bytes hole, try to pack */
            unsigned int               ui;                   /*    24     4 */
            /* size: 32, cachelines: 1, members: 4 */
            /* sum members: 22, holes: 2, sum holes: 6 */
            /* padding: 4 */
            /* last cacheline: 32 bytes */
    };

Looking at the sizes of the elements in that output I can start to render the dump in a 
more legible form.

    :; ./test | ./arrange -q 4 4 8 3 { 2 } 2 4
    7b 00 00 00
    00 00 00 00
    04 50 1e 02 8e 55 00 00
       00 01
       00 0c
       14 00
    00 00
    78 56 34 12

The "-q" flag makes the output quieter than normal. You can already see elements 
of the initialisation quite clearly. At this point, the octets are treated as tokens 
and not interpreted in any way. Let's add some more decorations and see whether we 
can get more out of this.

I'm going to name all the fields for a start.

    :; ./test | ./arrange i 4 pad 4 str 8 a 3 { 2 } pad 2 ui 4 | column -t -s '#' -o '#'
    7b 00 00 00             # i    #
    00 00 00 00             # pad  #
    04 00 66 f7 50 56 00 00 # str  #
       00 01                # a    #
       00 0c                # a    #
       14 00                # a    #
    00 00                   # pad  #
    78 56 34 12             # ui   #

The "#" character is used as a separator so I can let the column (https://linux.die.net/man/1/column) 
tidy up for me.

Now I can hide the pad, it's not really useful. There's a +h control which can be inserted 
between the pad identifier and the token count which follows, like so:

    :; ./test | ./arrange i 4 pad +h 4 str 8 a 3 { 2 } pad +h 2 ui 4 | column -t -s '#' -o '#'
    7b 00 00 00             # i    #
    04 90 3a d8 c9 55 00 00 # str  #
       00 01                # a    #
       00 0c                # a    #
       14 00                # a    #
    78 56 34 12             # ui   #

A full list of the controls is listed in the "Flags and Command Line Syntax" section 
later.

Worth mentioning is that the controls act hierarchically, everything in
the tree structure below the control will have that control applied. Those
controls can be countermanded further down, if necessary.

It can be useful to see array indexes, too. That's just a command line flag 
at the moment "-i".

    :; ./test | ./arrange -i i 4 pad +h 4 str 8 a 3 { 2 } pad +h 2 ui 4 | column -t -s '#' -o '#'
    7b 00 00 00             # i      #
    04 c0 66 d9 57 55 00 00 # str    #
       00 01                # a [0]  #
       00 0c                # a [1]  #
       14 00                # a [2]  #
    78 56 34 12             # ui     #

Or for the full picture we can expand the array completely (note the additional detail in the braced section).

    :; ./test | ./arrange -i i 4 pad +h 4 str 8 a 3 { 2 { 1 } } pad +h 2 ui 4 | column -t -s '#' -o '#'
    7b 00 00 00             # i          #
    04 b0 b2 a1 8e 55 00 00 # str        #
          00                # a [0] [0]  #
          01                # a [0] [1]  #
          00                # a [1] [0]  #
          0c                # a [1] [1]  #
          14                # a [2] [0]  #
          00                # a [2] [1]  #
    78 56 34 12             # ui         #

It'd be nice to see those numbers in decimal form, there's a flag for that ("-s") ...

    :; ./test | ./arrange -i -u i 4 pad +h 4 str 8 a 3 { 2 { 1 } } pad +h 2 ui 4 | column -t -s '#' -o '#'
    123            # i          # +u
    94445099528196 # str        # +u
          0        # a [0] [0]  # +u
          1        # a [0] [1]  # +u
          0        # a [1] [0]  # +u
          12       # a [1] [1]  # +u
          20       # a [2] [0]  # +u
          0        # a [2] [1]  # +u
    305419896      # ui         # +u

Many of the controls (which start with a "+" and are later in the arguments also have a conventional flag 
format, preceded by the usual "-". In the flag form they apply from the top level of the tree.

Looking at the last output, I think that it would be nicer to see the
pointer and the unsigned value as hexadecimal. We can use a command,
"-x". The formatting commands are loosely modelled on the C printf
formats.

    :; ./test | ./arrange -i -u i 4 pad +h 4 str +x 8 a 3 { 2 { 1 } } pad +h 2 ui +x 4 | column -t -s '#' -o '#'
    123                # i          # +u
    0x000055ddeb56d004 # str        # +x
          0            # a [0] [0]  # +u
          1            # a [0] [1]  # +u
          0            # a [1] [0]  # +u
          12           # a [1] [1]  # +u
          20           # a [2] [0]  # +u
          0            # a [2] [1]  # +u
    0x12345678         # ui         # +x

Looking back at the original code, that resembles the initialisation.

## An apology

This is an inelegant program, it does too much and lacks orthogonality. However, it's been 
shaped by my requrirements. I have spend so long formatting structures in my head, which 
is error prone and tiring. I hope that this effort will save me time in future, but on balance 
the saving will be small. Unless somebody else gets some use from it too ...

## Flags and Command Line Syntax

The arrange program shows this help when run without arguments:

    Flags:
        -q Quiet operation, show no additional decorations
        -l Linear mode, instead of displaying the indented structure place the output on a single line
        -s Sustain, keep reapplying the commands as if the structure described repeats indefinitely
        -e Extend, extend the input token list to satisfy all of the command line arguments
        -p Prefix, allows the first token on the line to act as a prefix for all output, intended for timestamps
        -f Assume that there are this many tokens to interpret before restarting the decode
    Flags or Commands
        -r, +r Reverse bytes, useful for handling endianness
        -f, +f Forward, which countermands reverse
        -h, +h Hide elements
        -s, +s Show elements, which countermands hide
        -u, +u Format any set of less than eight bytes as unsigned
        -d, +d Format any set of less than eight bytes as signed
        -x, +x Format any set of less than eight bytes as hexadecimal
        -c, +c Format as characters check using isprint, anything which fails will be printed as "?"
        -n, +n Stop current formatting and just go back to outputting tokens
        -o, +o Adds a decoration with the offset of the element, useful for poking structures
        -i, +i Print indices for arrays

## Examples
