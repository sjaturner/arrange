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

## Maybe an example would be easier 
