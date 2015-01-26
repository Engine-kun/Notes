Compiling Errors
================

Errorneous floating point number header files
---------------------------------------------

MinGW GCC may issue errors on compiling floating point programs due to error
definitions in `<float.h>`, you may check on `FLT_EPSILON` or `DBL_EPSILON`
macro, to see if they are missing or not.

The correct result may be like this:

    $ gcc -posix e.c
    $ ./a.exe
    0.032258
    1.192093e-007
    2.220446e-016

The errorneous result may look like this:

    error: 'DBL_EPSILON' undeclared (first use in this function)

To correct this problem(for example, MinGW GCC 4.9.2):

1. correct GCC version test macro in standard `<float.h>`

        mingw-w64-i686-4.9.2-release-posix-dwarf-rt_v3-rev1.7\i686-w64-mingw32\include\float.h
        
        !#if (__GNUC__ < 4  || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)) \
            || (__clang_major__ >=3)
        
        !#if (__GNUC__ < 4  || (__GNUC__ == 4 && __GNUC_MINOR__ <= 9)) \
            || (__clang_major__ >=3)

2. comment out `#include_next <float.h>` at the bottom of `float.h` from gcc
architecture include folders(x86/x86_64).

        mingw-w64-i686-4.9.2-release-posix-dwarf-rt_v3-rev1.7\lib\gcc\i686-w64-mingw32\4.9.2\include\float.h
        
        #endif /* _FLOAT_H___ */
        +/*
        #include_next <float.h>
        +*/


Errorneous printf format for floating point numbers
---------------------------------------------------

To avoid using the Microsoft runtime (which might not be conform to ISO C),
you can use the MinGW runtime package (which is an integral part of MinGW).
For example, with MinGW versions 3.15 and later you can get an
ISO-compliant `printf()` if you compile your application with either
`'-ansi'`, `'-posix'` or `'-D__USE_MINGW_ANSI_STDIO'`. For example, you can
compile and test MPFR with `CC="gcc -D__USE_MINGW_ANSI_STDIO"`.

For example under Win32, the following problem has been experienced with
MPFR 2.4.0 RC1 and the MSVC runtime (`msvcrt.dll`):

    Error in mpfr_vsprintf (s, "%.*Zi, %R*e, %Lf%n", ...);
    expected: "00000010610209857723, -1.2345678875e+07, 0.032258"
    got:      "00000010610209857723, -1.2345678875e+07, -0.000000"
    FAIL: tsprintf.exe

This error is due to the MSVC runtime not supporting the `L` length modifier
for formatted output (e.g. `printf` with `%Lf`). You can check this with the
following program:

    #include <stdio.h>
    int main (void)
    {
        long double d = 1. / 31.;
        printf ("%Lf\n", d);
        return 0;
    }

The expected output is `0.032258`.

Note: The L modifier has been standard for a long time (it was added
in ISO C89).

Errorneous "localeconv"
-----------------------

Error information when `make check`

    lib/libmsvcrt.a(dxrobs01029.o):(.text+0x0): 
    multiple definition of 'localeconv'
    t-locale.o:t-locale.c:(.text+0x0): first defined here

It's a confliction between MinGW runtime library & gmp source code, get
rid of this error by wrapping the redefinition of localeconv:

    #ifndef __MINGW64__
    #if HAVE_LOCALECONV
    struct lconv *
    localeconv (void)
    {
        static struct lconv  l;
        l.decimal_point = point_string;
        return &l;
    }
    #endif
    #endif


