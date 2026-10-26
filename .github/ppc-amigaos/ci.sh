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
    autoconf automake bison curl dejagnu flex g++ gcc libgmp-dev \
    libisl-dev libmpc-dev libmpfr-dev libtool m4 make
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

# gcc's AmigaOS tests (gcc.target/powerpc/amigaos and
# g++.target/powerpc/amigaos) for each C runtime, with the board in
# baseboards/.  Nothing here runs AmigaOS programs, so the tests that would
# run one only link it.
step_test() {
  local boards='ppc-amigaos/\{-mcrt=newlib,-mcrt=clib2,-mcrt=clib4\}'
  local sums="gcc-build/gcc/testsuite/gcc/gcc.sum gcc-build/gcc/testsuite/g++/g++.sum"
  local sum
  export DEJAGNU="$CI_DIR/dejagnu-site.exp"
  make -C gcc-build check-gcc-c \
    RUNTESTFLAGS="--target_board=$boards gcc.target/powerpc/amigaos/amigaos.exp"
  make -C gcc-build check-gcc-c++ \
    RUNTESTFLAGS="--target_board=$boards g++.target/powerpc/amigaos/amigaos.exp"
  # make check succeeds whatever the results, so look at them.
  for sum in $sums; do
    grep -E '^([[:space:]]+=== .* Summary|# of )' "$sum"
    grep -q '^# of expected passes' "$sum"
  done
  if grep -E '^(FAIL|XPASS|UNRESOLVED|ERROR):' $sums; then
    return 1
  fi
}

[ $# -eq 1 ] || { echo "usage: $0 STEP" >&2; exit 2; }
"step_$1"
