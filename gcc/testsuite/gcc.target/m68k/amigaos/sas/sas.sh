#!/bin/sh
# Compile the SAS/C halves of the interoperability tests (see README) into
# the checked-in obj/*.o objects, with SAS/C 6.58 run by volamos.
#   ./sas.sh /path/to/SC
# where /path/to/SC is a SAS/C installation (the SC: directory, with C/,
# include/ and lib/).  Run ./gen.py first when it changed.
#
# Each group gives three objects:
#   G-callee-reg.o   the functions, __regargs (parameters=registers): @name
#   G-callee-both.o  the same with a stack entry too (parameters=both):
#                    _name for callers that pass everything on the stack
#   G-caller-reg.o   run_G() calling them as __regargs functions
# No globals and no multiply or divide, so they need neither sc.lib nor
# the startup code, and link into a gcc program as they are.
set -e
sc=${1:?usage: $0 /path/to/SC}
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$here/obj"

for g in noargs core narrow struct asm fpu; do
	cpu=
	[ $g = fpu ] && cpu="cpu=68020 math=68881"
	for spec in callee:reg:registers callee:both:both caller:reg:registers; do
		half=${spec%%:*}
		rest=${spec#*:}
		tag=${rest%%:*}
		params=${rest#*:}
		# a failing group still makes the others; the missing object
		# says which, and the test reports it unsupported
		volamos -V SC:"$sc" -V W:"$here" -a INCLUDE:SC:include -a LIB:SC:lib \
			-a ENV:W: --cwd W: "$sc/C/sc" "$g-$half.c" \
			parameters=$params data=far code=far nostackcheck opt $cpu noicons \
			objname=obj/$g-$half-$tag.o ||
			echo "sas.sh: $g-$half ($params) failed" >&2
	done
done
