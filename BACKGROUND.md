# Arrange Tokens From Standard Input in a Structured Form Controlled by Command Line Arguments

Snappy title, it's going to be quite difficult to explain why I want
this utility ...

Some background: I work on embedded near real time Linux software. Often,
there are long running processes and I use the process\_vm\_readv to
read global variables without interrupting anything. The program which
I used (more of this in another repo, sometime) just emits a line of
hex octets. It's convenient for debugging. Often I'm just interested
in the contents of an integer variable but sometimes I want to see
structures. Ideally, I'd have a program which simply interpreted the
DWARF debug information and pretty printed the structure.  I've made
some tools for that (see https://github.com/sjaturner/cdump for
example) but more often than not I just use the Linux program
https://linux.die.net/man/1/pahole to figure out the structure layout.

## Perhaps an Example Would be Better?

Make the arrange utility and the test program

    :; make clean all
    rm -f test arrange
    cc -Wall -Wextra -g    test.c   -o test
    cc -Wall -Wextra -g    arrange.c   -o arrange
    done

Take a look at that test structure ... How is is arranged in memory?

    :; pahole test
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

Knowing that arrangement, try to render the structure in a more helpful way:

    :; ./test | ./arrange -i i 4 pad 4 str 8 a 3 { 2 { 1 } } pad 2 ui 4 | column -t -s '#' -o '#' 
    7b 00 00 00              # i 
    00 00 00 00              # pad 
    0a e0 aa 5b f9 55 00 00  # str 
            00               # a [0] [0] 
            01               # a [0] [1] 
            00               # a [1] [0] 
            0c               # a [1] [1] 
            14               # a [2] [0] 
            00               # a [2] [1] 
    00 00                    # pad 
    78 56 34 12              # ui 


In more detail, that final command runs test (which outputs the structure contents as bytes on a line).

Pipe that into the arrange utility, which will render it according to the command line arguments. 
Breaking those arguments down:

    ./arrange                   # The arrange utility
    -i                          # Flag: add indexes to arrays

        i 4                     # The first structure element is called i and is four bytes wide
        pad 4                   # GCC inserted a pad here, we need to account for that
        str 8                   # The string pointer is eight bytes long
        a 3                     # a is an array of three other arrays
            {                   # Describe the inner array, it is two elements long
                2 
                { 
                    1           # Specifying this inner element size makes sure everything gets an index
                } 
            } 
        pad 2                   # Another GCC inserted pad
        ui 4                    # The final unsigned integer

The final stage in the pipeline just makes the decorations, which describe the elements, nicely aligned.

## A Terser Output

The decorative stuff can be omitted using the '-q' flag (quiet).

    :; ./test | ./arrange -i i -q 4 4 8 3 { 2 } 2 4
    7b 00 00 00
    00 00 00 00
    0a 30 67 21 13 56 00 00
        00 01
        00 0c
        14 00
    00 00
    78 56 34 12

## Additional flags

There are some silly embellishments which determine the behaviour when there are either:

* Excess input values on stdin, where the '-r' flag can be used to arrange all of the input with the same structure, in a loop.
* Too few input values where the '-f' flag instructs the program to pretend there were enough input values and replace the missing items with '?'

# Conclusion

On reflection, this is just another thing which is very specific to my
requirements. I'm not sure whether anyone else would find it useful but
I'd be interested to find out.

Of course, the input values don't need to be hexadecimal octets. Any
list of whitespace separated tokens would do. In some ways this program
can be thought of as a structured replacement for 'xargs -n' or paste.

Final point: the decorations can be used to annotate values which can
then be used in subsequent pipeline stages (simplistically, in the
earlier example you could grep out the value of 'ui')
