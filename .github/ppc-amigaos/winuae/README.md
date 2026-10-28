# Running the testsuite on AmigaOS 4 in WinUAE

`amigaos-sim` lets DejaGnu run the testsuite's programs on AmigaOS 4 in WinUAE, through the board's `SIM` hook. Without it, the `dg-do run` tests are only linked.

DejaGnu calls `amigaos-sim PROGRAM`. It copies the program into a directory the emulator mounts as `SIMSHARE:`, and `simagent`, running on AmigaOS 4, starts it there. `amigaos-sim` then prints the program's output and exits with its return code.

- **Crashes:** a program that crashes or hangs fails after `$AMIGAOS_SIM_TIMEOUT` seconds (default 60), and the tests after it go on. Each crash leaves a Grim Reaper requester behind in the emulator.
- **Speed:** a program takes about a second to run.

## What you need

- **WinUAE**, with a configuration that boots AmigaOS 4.1, such as the Classic one with a CyberStorm PPC.
- **A copy of its system hardfile** to test with.
- **amitools' `xdftool`** (`pip install amitools`), to prepare that copy.
- **The ppc-amigaos toolchain**, to build `simagent`.

## Setting up

```sh
# a copy of the system hardfile, to be changed
cp amigaos41.hdf amigaos41-sim.hdf
# add "Run >NIL: SIMSHARE:simagent" to its S:User-Startup; it only runs when
# SIMSHARE: exists, so the image still boots normally without it
./amigaos-sim --prepare amigaos41-sim.hdf

ppc-amigaos-gcc -O2 -o simagent simagent.c

export AMIGAOS_SIM_SHARE=$HOME/amigaos-sim      # any empty directory
export AMIGAOS_SIM_CONFIG=$HOME/Documents/WinUAE/Configuration/AmigaOS41.uae
export AMIGAOS_SIM_HDF=$PWD/amigaos41-sim.hdf
export AMIGAOS_SIM_AGENT=$PWD/simagent
./amigaos-sim --start         # boots WinUAE and waits for simagent
```

- **`--start`** writes `$AMIGAOS_SIM_SHARE/winuae.uae`: the configuration with `$AMIGAOS_SIM_HDF` as the system disk and `SIMSHARE:` added as a directory drive that it doesn't boot from.
- **`--stop`** stops that WinUAE.
- **`--status`** says whether `simagent` is running.
- **`amigaos-sim PROGRAM [ARGS]`** runs a single program by hand.

## Running tests

In a gcc build directory:

```sh
export DEJAGNU=/path/to/gcc/.github/ppc-amigaos/dejagnu-site.exp
make check-gcc-c++ RUNTESTFLAGS="--target_board=ppc-amigaos/-mcrt=newlib \
    SIM=/path/to/gcc/.github/ppc-amigaos/winuae/amigaos-sim \
    g++.target/powerpc/amigaos/amigaos.exp dg.exp=eh/*.C"
```

For an installed compiler, use `contrib/test_installed` with the same `--target_board` and `SIM=`. Put the toolchain's `bin` on the `PATH`: some tests run `ppc-amigaos-objdump`.

**C runtimes:** clib4 programs need clib4.library, which an AmigaOS 4.1 installation doesn't have unless you add it.
