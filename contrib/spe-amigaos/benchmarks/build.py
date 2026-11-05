#!/usr/bin/env python3
"""Build the same numerical kernels for a host or an A1222."""
import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--cc', default='cc', help='kernel compiler, including driver options')
p.add_argument('--driver-cc', help='optional common compiler for timer/I/O wrapper')
p.add_argument('--amigaos', action='store_true')
p.add_argument('--opt', choices=['O2', 'O3', 'Os'], default='O3')
p.add_argument('--kernel-flag', action='append', default=[],
               help='extra diagnostic flag, e.g. --kernel-flag=-fno-tree-vectorize')
p.add_argument('--output', type=Path, required=True)
p.add_argument('--label', default='local')
a = p.parse_args()
src = Path(__file__).resolve().parent
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
cc = shlex.split(a.cc)
driver_cc = shlex.split(a.driver_cc) if a.driver_cc else cc
common = ['-std=c99', '-fno-fast-math', '-ffp-contract=off', '-Wall', '-Wextra']
cpu = ['-mcpu=8548', '-Wa,-me500'] if a.amigaos else []
driver_cpu = ['-mcpu=750'] if a.amigaos else []
flags = common + cpu + ['-' + a.opt] + a.kernel_flag
commands = []

def run(command):
    commands.append(command)
    subprocess.run(command, check=True)

run(cc + flags + ['-c', str(src / 'kernels.c'), '-o', str(out / 'kernels.o')])
run(cc + flags + ['-S', str(src / 'kernels.c'), '-o', str(out / 'kernels.s')])
run(cc + common + cpu + ['-O0', '-c', str(src / 'reference.c'),
                         '-o', str(out / 'reference.o')])
run(driver_cc + common + driver_cpu + ['-O2', '-DBUILD_LABEL=' + json.dumps(a.label),
    '-c', str(src / 'driver.c'), '-o', str(out / 'driver.o')])
# GCC6 labels SPE objects as soft-float.  This kernel's pointer/integer
# boundary and absence of FP library calls make this particular mix valid.
version = subprocess.check_output(cc + ['-dumpversion'], text=True).strip()
link_flags = ['-Wl,--no-warn-mismatch'] if a.amigaos and version.startswith('6.') else []
run(driver_cc + driver_cpu + link_flags + [str(out / f) for f in ('driver.o', 'kernels.o', 'reference.o')]
    + ['-o', str(out / 'spe-bench')])
manifest = {
    'label': a.label,
    'compiler': subprocess.check_output(cc + ['--version'], text=True),
    'driver_compiler': subprocess.check_output(driver_cc + ['--version'], text=True),
    'commands': commands,
    'sha256': {f.name: hashlib.sha256(f.read_bytes()).hexdigest()
               for f in src.iterdir() if f.suffix in ('.c', '.h', '.py')},
}
(out / 'build.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(out / 'spe-bench')
