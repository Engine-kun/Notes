
# LLVM Environment exporter, put this script in
# /etc/profile.d/
# under msys system, then you may invoke 'env.llvm' or 'env.gcc'
# to switch environment between LLVM & GCC

function env.llvm {
# LLVM using Mingw-W64
LLVM_HOME=/d/toolchains/LLVM
LLVM_VERSION=3.6.0

MINGW_HOME=/d/toolchains/mingw-w64-x86_64-4.9.2-release-posix-seh-rt_v3-rev1
MINGW_VERSION=4.9.2

# C headers & LLVM platform headers go first
C_INCLUDE_PATH=\
${LLVM_HOME}/lib/clang/${LLVM_VERSION}/include:\
${MINGW_HOME}/lib/gcc/x86_64-w64-mingw32/${MINGW_VERSION}/include:\
${MINGW_HOME}/lib/gcc/x86_64-w64-mingw32/${MINGW_VERSION}/include-fixed:\
${MINGW_HOME}/x86_64-w64-mingw32/include

# C++ headers & C headers
CPLUS_INCLUDE_PATH=\
${C_INCLUDE_PATH}:\
${MINGW_HOME}/x86_64-w64-mingw32/include/c++:\
${MINGW_HOME}/x86_64-w64-mingw32/include/c++/x86_64-w64-mingw32:\
${MINGW_HOME}/x86_64-w64-mingw32/include/c++/backward

LIBRARY_PATH=\
${MINGW_HOME}/lib/gcc/x86_64-w64-mingw32/${MINGW_VERSION}/:\
${MINGW_HOME}/lib/gcc/:\
${MINGW_HOME}/x86_64-w64-mingw32/lib/:\
${MINGW_HOME}/lib/

# export
export C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH
export LIBRARY_PATH
}

function env.gcc {
# Let GCC decide  it's own search paths
unset LLVM_HOME
unset LLVM_VERSION

unset MINGW_HOME
unset MINGW_VERSION

unset C_INCLUDE_PATH
unset CPLUS_INCLUDE_PATH
unset LIBRARY_PATH
}
