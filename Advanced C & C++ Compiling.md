Advanced C & C++ Compiling
==========================

----------

Linux process virtual memory mapping scheme
-------------------------------------------

Typical Linux process memory map:
	
    +--------------+------------------------------------------+
    |SYSTEM        | operating system functionality for       |
    |              | controlling the program execution        |
    +--------------+------------------------------------------+
    |              | environment variables                    |
    |              | argv(list of command line arguments)     |
    |              | argc(number of command line arguments)   |
    |STACK         |                                          |
    |              | local variables for main() function      |
    |              | local variables for other function       |
    |              |                                          |
    |              |                 |  |                     |
    |              |                 \  /                     |
    |              |                  \/                      |
    +--------------+------------------------------------------+
    |SHARED        | functions from linked dynamic libraries  |
    |MEMORY        |                                          |
    +--------------+------------------------------------------+
    |              |                  /\                      |
    |              |                 /  \                     |
    |              |                 |  |                     |
    |              |                                          |
    |HEAP          |                                          |
    +--------------+------------------------------------------+
    |DATA          | initialized data                         |
    |              | uninitialized data                       |
    +--------------+------------------------------------------+
    |TEXT          | functions from linked static libraries   |
    |              | other program functions                  |
    |              | main function(main.o)                    |
    |              | startup routines(crt0.o)                 |
    +0x00000000----+------------------------------------------+


The Stages of Compiling
-----------------------

1. pre-processing

2. linguistic analysis
    * Lexical analysis

        break source into non-divisible tokens

    * Parsing/syntax analysis

        concatenates tokens to token chains, verify the chain
        order according to language rules

    * Semantic analysis

        discover whether the syntactically correct statements actually make
        any sense

3. assembling
    * Assembly Format
        1. AT&T (`-masm=att`)
        2. Intel (`-masm=intel`)

4. optimization

5. code emmission
    * convert assembly instructions to binary CPU opcodes
    * write into specific locations in object files


The Stages of Linking
---------------------

1. relocation
    * translate zero-based address ranges into more concrete address ranges
    * create most(not all!) of the program memory map

2. resolving references
    * tiling sections and look up for symbols

Executable File Properties
--------------------------

1. extra code added to program memory map
    * crt0

        first part of program code executed under kernel control

    * crt1

        modern startup routine support tasks before main function gets
        executed and after program terminates

2. section types
    * .bss .text .data
    * .ctor .dtor .dynamic .got .interp .init .fini .plt .symtab
    *etc.

3. symbol types
    * "A"

        absolute, not change by further linking

    * "B" "b"

        .bss data section

    * "C"

        common symbol

    * "D" "d"

        initialized data section

    * "G" "g"

        initialized data section for small objects

    * "N"

        debugging symbol

    * "R" "r"

        read only data section

    * "S" "s"

        uninitialized data section for small objects

    * "T" "t"

        text(code) section

    * "U"

        undefined symbol

    * etc.


Loader Roles
------------

1. segments
    * loader segments typically carry several sections have in common
      access attributes(read/read&write/to-be-patched)

    * find program entry point

            _start()
            __libc_start_main()

Stack & Functions Calling Conventions
-------------------------------------

1. cdecl

    1. arguments passed from right to left
    2. stack cleanup by caller function
    3. function name decorated by prepending underscore '`_`'

2. stdcall

    1. arguments passed from right to left
    2. stack cleanup by called function
    3. function name decorated by:
        - prepending underscore '`_`'
        - appending '`@`'
        - appending the number of bytes of stack space required

3. fastcall

    1. arguments placement:

        - 1st & 2nd arguments(sizes 4 bytes or less) placed in register ECX
          and EDX

        - the rest arguments pushed on stack from right to left

    2. arguments popped from stack by called function

    3. function name decorated by:

        - prepending '`@`'
        - appending '`@`'
        - appending the number of bytes of stack space required

4. thiscall

    * default calling convention for C++ member functions

    * arguments placement
        - passed from right to left on stack
        - '`this`' is placed in ECX

    * stack cleanup by called function



