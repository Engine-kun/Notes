#!/bin/sh

# mingw-w64 is buggy with <float.h>, possibly on include order, you MUST
# fix the problem before compiling, test it on DBL_EPSILON/FLT_EPSILON.

# CFLAGS "-posix" is a must for proper print format here

cwd=$(pwd)
echo "cwd: ${cwd}"

make_gmp()
{
	make clean

	CFLAGS+="-posix -march=pentium-m" ./configure --disable-static --enable-shared -enable-cxx

	make && make check
}

make_mpfr()
{
	make clean

	# -posix Fix Windows CRT lib shipping with non-posix printf(),
	# which messes up long float point format
	# -march=pentium-m Fix tune with proper x86 mparam.h file
	# -O0 Supress compiler bugs error

	CFLAGS+="-posix -march=pentium-m -O0 -D__tune_i686__" ./configure --disable-static --enable-shared --disable-thread-safe --with-gmp-include=${cwd}/gmp-6.0.0/ --with-gmp-lib=${cwd}/gmp-6.0.0/.libs/

	make && make check
}

make_mpc()
{
	make clean

	CFLAGS+="-posix -march=pentium-m -D__tune_i686__" ./configure --disable-static --enable-shared --with-gmp-include=${cwd}/gmp-6.0.0/ --with-gmp-lib=${cwd}/gmp-6.0.0/.libs/ --with-mpfr-include=${cwd}/mpfr-3.1.2/src --with-mpfr-lib=${cwd}/mpfr-3.1.2/src/.libs

	make && make check
}
# Begin of compiling GMP
# ======================
#pushd .
#cd gmp-6.0.0
#make_gmp
#popd

# Begin of compiling MPFR
# =======================
pushd .
cd mpfr-3.1.2
make_mpfr
popd

# Begin of compiling MPC
# ======================
pushd .
cd mpc-1.0.2
make_mpc
popd

#shutdown -s -f

