#!/usr/bin/env python3
"""Check the supported AmigaOS4 SPE ABI and multilib selections."""
import argparse
import json
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--gcc-build', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
build = a.gcc_build.resolve() / 'gcc'
cc = [str(build / 'xgcc'), '-B' + str(build) + '/']
src = a.output.resolve() / 'empty.c'
src.write_text('int value;\n')

def run(flags):
    return subprocess.run(cc + flags, text=True, capture_output=True, timeout=30)

multilib = run(['-print-multi-lib'])
assert multilib.returncode == 0, multilib.stderr
expected = {'.', 'clib2', 'clib4', 'clib2/soft-float', 'spe',
            'newlib/spe', 'clib2/spe', 'clib4/spe'}
assert {x.split(';')[0] for x in multilib.stdout.splitlines()} == expected
accepted = [([], '.'), (['-mcpu=750'], '.'),
            (['-mcrt=clib2', '-msoft-float'], 'clib2/soft-float'),
            (['-mcpu=8548'], 'spe'),
            (['-mcpu=8540', '-mspe', '-mabi=spe', '-mfloat-gprs=double'], 'spe')]
accepted += [(['-mcpu=8548', '-mcrt=' + crt], 'spe' if crt == 'newlib' else crt + '/spe')
             for crt in ['newlib', 'clib2', 'clib4']]
for flags, directory in accepted:
    r = run(flags + ['-S', str(src), '-o', str(src.with_suffix('.s'))])
    assert r.returncode == 0, (flags, r.stderr)
    r = run(flags + ['-print-multi-directory'])
    assert r.returncode == 0 and r.stdout.strip() == directory, (flags, r)
rejected = [['-mcpu=8540'], ['-mspe'], ['-mabi=spe'],
            ['-mcpu=8548', '-msoft-float'],
            ['-mcpu=8548', '-mcrt=clib2', '-msoft-float'],
            ['-mcpu=8548', '-mabi=no-spe'],
            ['-mcpu=8548', '-mno-spe'],
            ['-mcpu=8548', '-mfloat-gprs=single']]
for flags in rejected:
    # -### checks the driver's link-only path without invoking the linker.
    for action in [['-S', str(src)], ['-###', str(src.with_suffix('.o'))]]:
        r = run(flags + action)
        assert r.returncode != 0 and 'AmigaOS4 SPE requires' in r.stderr, (flags, r)
report = {'multilibs': multilib.stdout.splitlines(), 'accepted': accepted,
          'rejected_at_compile_and_link': rejected}
(a.output / 'options.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
