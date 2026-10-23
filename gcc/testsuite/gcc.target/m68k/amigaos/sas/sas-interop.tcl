# Copyright (C) 2026 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GCC; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.

# SAS/C interoperability of -msasregparm -mmangle-regparm, sourced by
# amigaos.exp.  For each group of gen.py's functions, a gcc driver runs
# the caller half against the callee half, in every pairing of gcc and the
# checked-in SAS/C objects (obj/, made by sas.sh), and passes when every
# function returned what gen.py computed.  See README.

set sas_dir $srcdir/$subdir/sas

# Whether the board runs 68881 code, for the fpu group.
proc check_effective_target_m68881_runtime { } {
    return [check_runtime m68881_runtime {
	int main (void)
	{
	  volatile double x = 1.5;
	  return x * 2.0 != 3.0;
	}
    } "-m68020 -m68881"]
}

# Compile a gcc half SRC into OBJ, or return 0.
proc sas_compile { src obj opts } {
    global tool
    set out [${tool}_target_compile $src $obj object \
		 [list "additional_flags=$opts"]]
    if { ![file exists $obj] } {
	verbose -log "sas: $src: $out"
	return 0
    }
    return 1
}

# Link DRIVER with the two halves and run it: PASS when every function
# returned the expected value.  XFAIL marks pairings known to differ.
proc sas_run { name driver caller callee opts xfail } {
    global tool
    set exe [file rootname [file tail $driver]]-[file rootname [file tail $caller]]-[file rootname [file tail $callee]].exe
    foreach o [list $caller $callee] {
	if { ![file exists $o] } {
	    unsupported "$name: [file tail $o] missing"
	    return
	}
    }
    set out [${tool}_target_compile "$driver $caller $callee" $exe \
		 executable [list "additional_flags=$opts -I[file dirname $driver]"]]
    if { ![file exists $exe] || [regexp {out of range|undefined reference} $out] } {
	if { $xfail } { xfail "$name: link" } else { fail "$name: link" }
	verbose -log "sas: $name: $out"
	return
    }
    set result [${tool}_load $exe "" ""]
    set status [lindex $result 0]
    verbose -log "sas: $name: [lindex $result 1]"
    file delete $exe
    if { $status eq "pass" } {
	if { $xfail } { xpass $name } else { pass $name }
    } elseif { $xfail } {
	xfail $name
    } else {
	fail $name
    }
}

# Group -> the pairings expected to fail, as known differences from SAS/C
# rather than bugs: SAS/C passes structures on the stack whatever their
# size and returns them through a buffer it passes on the stack, gcc
# passes small ones in registers and returns them in d0/d1.
array set sas_xfail {
    struct {gcc->sas sas->gcc plain->sas}
}

foreach g {noargs core narrow struct asm fpu} {
    # a parallel check runs this file in every slice: each group in one
    if { ![gcc_parallel_test_run_p "sas-$g"] } {
	continue
    }
    set opts ""
    if { $g eq "fpu" } {
	if { ![check_effective_target_m68881_runtime] } {
	    unsupported "sas $g: the board runs no 68881 code"
	    continue
	}
	set opts "-m68020 -m68881"
    }
    set reg "-O2 $opts -msasregparm -mmangle-regparm"
    set callee_gcc "sas-$g-callee-gcc.o"
    set caller_gcc "sas-$g-caller-gcc.o"
    set caller_plain "sas-$g-caller-plain.o"
    if { ![sas_compile $sas_dir/$g-callee.c $callee_gcc "$reg -I$sas_dir"]
	 || ![sas_compile $sas_dir/$g-caller.c $caller_gcc "$reg -I$sas_dir"]
	 || ![sas_compile $sas_dir/$g-caller.c $caller_plain "-O2 $opts -I$sas_dir"] } {
	fail "sas $g: compile"
	continue
    }
    set xf [expr { [info exists sas_xfail($g)] ? $sas_xfail($g) : {} }]
    set driver $sas_dir/$g-driver.c
    set obj $sas_dir/obj
    foreach {pair caller callee} [list \
	    gcc->gcc $caller_gcc $callee_gcc \
	    gcc->sas $caller_gcc $obj/$g-callee-reg.o \
	    sas->gcc $obj/$g-caller-reg.o $callee_gcc \
	    sas->sas $obj/$g-caller-reg.o $obj/$g-callee-reg.o \
	    plain->sas $caller_plain $obj/$g-callee-both.o] {
	sas_run "sas $g $pair" $driver $caller $callee "-O2 $opts" \
	    [expr { [lsearch -exact $xf $pair] >= 0 }]
    }
    file delete $callee_gcc $caller_gcc $caller_plain
}
