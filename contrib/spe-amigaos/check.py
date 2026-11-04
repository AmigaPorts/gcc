#!/usr/bin/env python3
"""Build freestanding SPE regression images; optionally run them in QEMU."""
import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import re
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--gcc-build', type=Path, required=True)
parser.add_argument('--gcc-source', type=Path, required=True)
parser.add_argument('--binutils', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--run', action='store_true')
parser.add_argument('--libgcc', type=Path, help='SPE libgcc.a for runtime helper tests')
a = parser.parse_args()
out = a.output.resolve()
out.mkdir(parents=True, exist_ok=True)
tests = Path(__file__).resolve().parent / 'tests'
config = a.gcc_source.resolve() / 'gcc/config/rs6000'
build = a.gcc_build.resolve() / 'gcc'
cc = [str(build / 'xgcc'), '-B' + str(build) + '/', '-I' + str(config)]
as_ = str(a.binutils / 'ppc-amigaos-as')
ld = str(a.binutils / 'ppc-amigaos-ld')
report = {'compiler': subprocess.check_output(cc + ['--version'], text=True).splitlines()[0],
          'source': str(a.gcc_source.resolve()), 'compiled': [], 'runtime': []}

def run(command):
    p = subprocess.run(command, text=True, capture_output=True)
    if p.returncode:
        raise RuntimeError(' '.join(command) + '\n' + p.stdout + p.stderr)
    return p

run([as_, '-me500', str(tests / 'start.S'), '-o', str(out / 'start.o')])
run([as_, '-me500', str(tests / 'external.S'), '-o', str(out / 'external.o')])
run(cc + ['-mcpu=8548', '-O2', '-c', str(tests / 'minicrt.c'),
          '-o', str(out / 'minicrt.o')])
helpers = [str(out / 'minicrt.o')]
for src in (a.gcc_source / 'libgcc/config/rs6000').glob('e500crt*.S'):
    obj = out / (src.stem + '.o')
    run(cc + ['-mcpu=8548', '-c', str(src), '-o', str(obj)])
    helpers.append(str(obj))

cases = [('scalar', ['scalar.c', 'scalar-run.c']),
         ('vector', ['intrinsics.c', 'vector-run.c']),
         ('alias', ['alias.c']), ('cxx', ['cxx.cc']),
         ('bridge', ['bridge-kernel.c', 'bridge-run.c'])]
if a.libgcc:
    cases.append(('libgcc', ['libgcc.c', 'libgcc-ops.c']))
for opt in ['-O0', '-O2', '-O3', '-Os']:
    for label, sources in cases:
        objects = []
        for name in sources:
            obj = out / (name + opt + '.o')
            compiler = cc if not name.endswith('.cc') else [str(build / 'xg++')] + cc[1:]
            cpu = '-mcpu=750' if name == 'bridge-run.c' else '-mcpu=8548'
            run(compiler + [cpu, opt, '-c', str(tests / name), '-o', str(obj)])
            objects.append(str(obj))
        elf = out / (label + opt + '.elf')
        run([ld, '-Ttext=0x100000', '-e', '_start', '-o', str(elf),
             str(out / 'start.o'), str(out / 'external.o')] + objects + helpers
            + ([str(a.libgcc.resolve())] if a.libgcc else []))
        report['compiled'].append(elf.name)
        if a.run:
            env = dict(os.environ, PPC_NM=str(a.binutils / 'ppc-amigaos-nm'))
            subprocess.run([sys.executable, str(tests.parent / 'run-qemu.py'), str(elf), '0'],
                           env=env, check=True)
            report['runtime'].append(elf.name)

# Exercise every declaration through the GCC16 builtin generator interface.
source = (config / 'rs6000-builtins.def').read_text().split('[spe]')[1]
types = {'spev2si': '__ev64_opaque__', 'spev2sf': 'fv'}
wrappers = []
for m in re.finditer(r'^  (?:const )?(\w+) (__builtin_spe_\w+) \((.*)\);', source, re.M):
    ret, name, args = m.groups()
    parameters, values = [], []
    for arg in re.split(r', (?![^<]*>)', args) if args else []:
        if arg.startswith('const int<'):
            values.append('1')
        else:
            var = 'a' + str(len(parameters))
            parameters.append(types.get(arg, arg) + ' ' + var)
            values.append(var)
    code = '#include <spe.h>\ntypedef float fv __attribute__((vector_size(8)));\n'
    code += types.get(ret, ret) + ' f(' + ', '.join(parameters) + '){'
    code += ('return ' if ret != 'void' else '') + name + '(' + ', '.join(values) + ');}\n'
    path = out / (name + '.c')
    path.write_text(code)
    wrappers.append(path)

def compile_builtin(path):
    run(cc + ['-mcpu=8548', '-O2', '-c', str(path), '-o', str(path.with_suffix('.o'))])

with concurrent.futures.ThreadPoolExecutor(8) as pool:
    list(pool.map(compile_builtin, wrappers))
report['builtin_compile_count'] = len(wrappers)
(out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
