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
expected = {'.', 'clib2', 'clib4', 'clib2/soft-float', 'clib2/spe',
            'clib4/spe'}
assert {x.split(';')[0] for x in multilib.stdout.splitlines()} == expected
accepted = [([], '.'), (['-mcpu=750'], '.'),
            (['-mcrt=clib2', '-msoft-float'], 'clib2/soft-float'),
            (['-mcpu=8540', '-mspe', '-mabi=spe', '-mfloat-gprs=double',
              '-mcrt=clib2'], 'clib2/spe')]
accepted += [(['-mcpu=8548', '-mcrt=' + crt], crt + '/spe')
             for crt in ['clib2', 'clib4']]
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
# Only clib2 and clib4 have SPE libraries: SPE code compiles with every C
# runtime, but links only with those two, unless the link leaves out the C
# library.
spe = [['-mcpu=8548'], ['-mcpu=8540', '-mspe', '-mabi=spe', '-mfloat-gprs=double']]
spe_crts = [['-mcrt=clib2'], ['-mcrt=clib2-ts'], ['-mcrt=clib4']]
no_spe_crts = [[], ['-mcrt=default'], ['-mcrt=newlib'], ['-mcrt=libnix'],
               ['-mcrt=ixemul']]
for flags in [s + c for s in spe for c in spe_crts + no_spe_crts]:
    r = run(flags + ['-S', str(src), '-o', str(src.with_suffix('.s'))])
    assert r.returncode == 0, (flags, r.stderr)
    for link in [[], ['-shared']]:
        r = run(flags + link + ['-###', str(src.with_suffix('.o'))])
        if flags[-1:] in spe_crts:
            assert 'link only with' not in r.stderr, (flags + link, r)
        else:
            assert r.returncode != 0 and 'AmigaOS4 SPE programs link only with' in r.stderr, (flags + link, r)
    for link in [['-c'], ['-r'], ['-nostdlib'], ['-nodefaultlibs']]:
        r = run(flags + link + ['-###', str(src)])
        assert r.returncode == 0, (flags + link, r.stderr)
report = {'multilibs': multilib.stdout.splitlines(), 'accepted': accepted,
          'rejected_at_compile_and_link': rejected,
          'linked_only_with': [c[0] for c in spe_crts],
          'rejected_at_link': [c[0] if c else '(none)' for c in no_spe_crts]}
(a.output / 'options.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
