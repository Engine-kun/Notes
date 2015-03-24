LLVM + Mingw-W64 Settings
=========================

gcc version 4.9.2 (x86_64-posix-seh-rev1, Built by MinGW-W64 project)
To get mingw's default search path for 'include' & 'library':

    gcc -v helloworld.c | awk '/#include/,/End/'
    gcc -v helloworld.cpp | awk '/#include/,/End/'

    C_INCLUDE_PATH
    #include "..." search starts here:
    #include <...> search starts here:
    ${HOME_MINGW}/lib/gcc/x86_64-w64-mingw32/4.9.2/include
    ${HOME_MINGW}/lib/gcc/x86_64-w64-mingw32/4.9.2/include-fixed
    ${HOME_MINGW}/x86_64-w64-mingw32/include
    End of search list.

    CPLUS_INCLUDE_PATH
    #include "..." search starts here:
    #include <...> search starts here:
    ${HOME_MINGW}/lib/gcc/x86_64-w64-mingw32/4.9.2/include
    ${HOME_MINGW}/lib/gcc/x86_64-w64-mingw32/4.9.2/include-fixed
    ${HOME_MINGW}/x86_64-w64-mingw32/include
    ${HOME_MINGW}/x86_64-w64-mingw32/include/c++
    ${HOME_MINGW}/x86_64-w64-mingw32/include/c++/x86_64-w64-mingw32
    ${HOME_MINGW}/x86_64-w64-mingw32/include/c++/backward
    End of search list.

    LIBRARY_PATH=
    ${HOME_MINGW}/lib/gcc/x86_64-w64-mingw32/4.9.2/
    ${HOME_MINGW}/lib/gcc/
    ${HOME_MINGW}/x86_64-w64-mingw32/lib/
    ${HOME_MINGW}/lib/

Invoke
------

    clang -m64 -o helloworld.exe helloworld.c

We're running x86_64 windows, so `-m64` is used, `clang` default to `-m32`, which
leads to compiling error:

    d:/toolchains/mingw-w64-x86_64-4.9.2-release-posix-seh-rt_v3-rev1/bin/../lib/gcc
    /x86_64-w64-mingw32/4.9.2/../../../../x86_64-w64-mingw32/bin/ld.exe: skipping in
    compatible d:/toolchains/mingw-w64-x86_64-4.9.2-release-posix-seh-rt_v3-rev1/x86
    _64-w64-mingw32/lib\libmsvcrt.a when searching for -lmsvcrt

Environment Virables for Msys
-----------------------------
See compiler.sh

