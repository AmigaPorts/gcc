#!/bin/bash
# Build and test steps of the ppc-amigaos toolchain CI, run from the
# workspace that holds the gcc checkout as ./gcc:
#
#   ci.sh deps|lha|binutils|sdk|bootstrap|clibs|gcc|test
#
# The workflow (.github/workflows/ppc-amigaos-ci.yml) sets TARGET, PREFIX
# and the repositories and refs of the other components.  Each step can
# also be run by hand with those set.

set -euo pipefail

CI_DIR=$(cd "$(dirname "$0")" && pwd)
GCC_SRC=$(cd "$CI_DIR/../.." && pwd)
WORK=$PWD
JOBS=${JOBS:-$(nproc)}
SDK_DIR="$PREFIX/$TARGET/SDK"
export PATH="$PREFIX/bin:$PATH"
# Build without texinfo and install no info files.  configure replaces a
# makeinfo it can't get a version from, so it goes on the make command
# line as well.
export MAKEINFO=true

# The bootstrap compiler, which builds clib2 and clib4 before gcc's
# runtime libraries can be built against them.
XGCC="$WORK/gcc-build/gcc/xgcc -B$WORK/gcc-build/gcc/"

# fetch URL DIR REF: shallow checkout of a branch, tag or commit.
fetch() {
  rm -rf "$2"
  git init -q "$2"
  git -C "$2" fetch -q --depth 1 "$1" "$3"
  git -C "$2" checkout -q FETCH_HEAD
}

step_deps() {
  sudo apt-get -y update
  sudo apt-get -y install --no-install-recommends \
    autoconf automake bison curl flex g++ gcc libgmp-dev libisl-dev \
    libmpc-dev libmpfr-dev libtool m4 make
  mkdir -p "$PREFIX"
}

step_lha() {
  fetch https://github.com/jca02266/lha.git lha master
  cd lha
  autoreconf -vfi
  ./configure --prefix="$PREFIX"
  make -j "$JOBS"
  make install
}

step_binutils() {
  fetch "$BINUTILS_REPO" binutils-gdb "$BINUTILS_REF"
  mkdir -p binutils-build
  cd binutils-build
  ../binutils-gdb/configure --target="$TARGET" --prefix="$PREFIX" \
    --disable-werror --disable-nls --without-msgpack \
    --disable-gdb --disable-gdbserver --disable-sim --disable-gprofng
  make -j "$JOBS" MAKEINFO=true
  make install MAKEINFO=true
}

step_sdk() {
  local archive="$WORK/SDK_${SDK_VERSION}.lha"
  [ -f "$archive" ] || curl -fsSL -o "$archive" "$SDK_URL"
  rm -rf sdk-extract
  mkdir -p sdk-extract "$SDK_DIR/include"
  cd sdk-extract
  lha xqf "$archive"
  cd SDK_Install
  lha xqf newlib*.lha
  lha xqf base.lha
  lha xqf execsg*.lha
  rm -f ./*.lha
  mv newlib* "$SDK_DIR/"
  mv Include/* "$SDK_DIR/include/"
}

step_bootstrap() {
  mkdir -p gcc-build
  cd gcc-build
  "$GCC_SRC/configure" --target="$TARGET" --prefix="$PREFIX" \
    --enable-languages=c,c++ --enable-threads=amigaos --enable-lto \
    --with-dwarf2 --disable-tls --disable-libstdcxx-pch \
    --disable-c++tools --disable-nls --disable-bootstrap
  make -j "$JOBS" all-gcc MAKEINFO=true
}

step_clibs() {
  fetch "$CLIB2_REPO" clib2 "$CLIB2_REF"
  make -C clib2/library -f GNUmakefile.os4 -j "$JOBS" \
    CC="$XGCC -Wno-error=int-conversion -Wno-error=incompatible-pointer-types -Wno-error=implicit-function-declaration" \
    AR="$TARGET-ar -q" RANLIB="$TARGET-ranlib"
  mkdir -p "$SDK_DIR/clib2"
  cp -Rp clib2/library/include clib2/library/lib "$SDK_DIR/clib2/"

  fetch "$CLIB4_REPO" clib4 "$CLIB4_REF"
  make -C clib4 -f GNUmakefile.os4 -j "$JOBS" \
    CC="$XGCC" AR="$TARGET-ar -q" RANLIB="$TARGET-ranlib" \
    STRIP="$TARGET-strip" INSTALL_PREFIX="$SDK_DIR/clib4" \
    SDK_INCLUDE="$SDK_DIR/include"
  mkdir -p "$SDK_DIR/clib4/include" "$SDK_DIR/clib4/lib"
  make -C clib4 -f GNUmakefile.os4 install INSTALL_PREFIX="$SDK_DIR/clib4"
}

step_gcc() {
  make -C gcc-build -j "$JOBS" MAKEINFO=true
  make -C gcc-build install MAKEINFO=true
}

# fails MESSAGE COMMAND...: COMMAND must fail with an error that contains
# MESSAGE.  The driver exits with status 1 for an internal compiler error
# as well, so that is checked for separately.
fails() {
  local msg=$1 status=0
  shift
  "$@" > out.txt 2>&1 || status=$?
  cat out.txt
  if [ "$status" -ne 1 ] || ! grep -qF -- "$msg" out.txt; then
    echo "expected exit status 1 and \"$msg\", got status $status" >&2
    return 1
  fi
  if grep -q "internal compiler error" out.txt; then
    echo "expected an error, got an internal compiler error" >&2
    return 1
  fi
}

step_test() {
  local t="$CI_DIR/tests" crt thread o
  rm -rf tests
  mkdir tests
  cd tests
  set -x
  for crt in newlib clib2 clib4; do
    "$TARGET-gcc" -O2 -mcrt=$crt "$t/hello.c" -o c-$crt
    "$TARGET-gcc" -O2 -mcrt=$crt -mbaserel "$t/hello.c" -o c-baserel-$crt
    for thread in native single; do
      "$TARGET-g++" -O2 -mcrt=$crt -athread=$thread "$t/hello.cc" -o cc-$thread-$crt
    done
    # Without -athread, g++ links the native thread implementation.
    "$TARGET-g++" -O2 -mcrt=$crt "$t/hello.cc" -o cc-default-$crt
    # import std: the std module is built once per C runtime.
    mkdir modules-$crt
    (cd modules-$crt &&
     "$TARGET-g++" -std=c++23 -fmodules -fsearch-include-path -mcrt=$crt -c bits/std.cc &&
     "$TARGET-g++" -O2 -std=c++23 -fmodules -mcrt=$crt "$t/modules.cc" -o modules)
  done
  "$TARGET-g++" -O2 -mcrt=clib4 -athread=pthread "$t/hello.cc" -o cc-pthread
  "$TARGET-gcc" -O2 -mcrt=clib4 -pthread "$t/hello.c" -o c-pthread
  "$TARGET-gcc" -O2 -msdata=sysv "$t/hello.c" -o c-sdata
  "$TARGET-gcc" -O2 -use-dynld -athread=native "$t/hello.c" -o c-dynld
  # -mbaserel must address g relative to r2.
  for o in -O0 -O2; do
    "$TARGET-gcc" $o -mbaserel -S "$t/baserel.c" -o - | grep -q 'g@brel@'
  done
  # clib4 has no small-data libraries, and only clib4 has pthreads; the
  # driver must say so rather than leave it to the linker.
  fails "-msdata is not supported with -mcrt=clib4" \
    "$TARGET-gcc" -mcrt=clib4 -msdata=sysv "$t/hello.c" -o c-sdata-clib4
  fails "-pthread requires -mcrt=clib4" \
    "$TARGET-gcc" -pthread "$t/hello.c" -o c-pthread-newlib
  fails "-athread=pthread requires -mcrt=clib4" \
    "$TARGET-g++" -mcrt=clib2 -athread=pthread "$t/hello.cc" -o cc-pthread-clib2
  # A failing _Static_warning warns rather than fails.
  "$TARGET-gcc" -c "$t/static-warning.c" -o static-warning.o 2> out.txt
  cat out.txt
  [ "$(grep -c "warning: static warn-only assertion failed" out.txt)" -eq 1 ]
  grep -q "static warn-only assertion failed: \"deprecated\"" out.txt
  # checktags: the argument after a tag is checked against its tagtype.
  fails "checktags.c:19:28: error: passing argument 3 of 'TestTags' from incompatible pointer type" \
    "$TARGET-gcc" -c "$t/checktags.c" -o checktags.o
  [ "$(grep -c "error:" out.txt)" -eq 1 ]
  fails "error:" "$TARGET-gcc" -c "$t/tagtype-bad.c" -o tagtype-bad.o
  "$TARGET-readelf" -h c-newlib cc-native-clib4
}

[ $# -eq 1 ] || { echo "usage: $0 STEP" >&2; exit 2; }
"step_$1"
