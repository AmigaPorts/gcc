/* Definitions of target machine for GNU compiler, for AmigaOS.
   Copyright (C) 1997, 2003, 2005 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */


/* Note: NO_IMPLICIT_EXTERN_C removed (patch 0026: don't use poisoned define) */

#undef MD_EXEC_PREFIX
#undef MD_STARTFILE_PREFIX

/* Make CPU default to 604e. FIXME: Make this 750 later */
#undef PROCESSOR_DEFAULT
#define PROCESSOR_DEFAULT PROCESSOR_PPC604e

#undef DEFAULT_ABI
#define DEFAULT_ABI ABI_V4

/* Updated ASM_CPU_SPEC (patch 0040) */
#undef ASM_CPU_SPEC
#define ASM_CPU_SPEC \
"%{mcpu=native: %(asm_cpu_native); \
  mcpu=power10: -mpower10; \
  mcpu=power9: -mpower9; \
  mcpu=power8|mcpu=powerpc64le: %{mpower9-vector: -mpower9;: -mpower8}; \
  mcpu=power7: -mpower7; \
  mcpu=power6x: -mpower6 %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=power6: -mpower6 %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=power5+: -mpower5; \
  mcpu=power5: -mpower5; \
  mcpu=power4: -mpower4; \
  mcpu=power3: -mppc64; \
  mcpu=powerpc: -mppc; \
  mcpu=powerpc64: -mppc64; \
  mcpu=a2: -ma2; \
  mcpu=cell: -mcell; \
  mcpu=rs64: -mppc64; \
  mcpu=401: -mppc; \
  mcpu=403: -m403; \
  mcpu=405: -m405; \
  mcpu=405fp: -m405; \
  mcpu=440: -m440; \
  mcpu=440fp: -m440; \
  mcpu=464: -m440; \
  mcpu=464fp: -m440; \
  mcpu=476: -m476; \
  mcpu=476fp: -m476; \
  mcpu=505: -mppc; \
  mcpu=601: -m601; \
  mcpu=602: -mppc; \
  mcpu=603: -mppc; \
  mcpu=603e: -mppc; \
  mcpu=ec603e: -mppc; \
  mcpu=604: -mppc; \
  mcpu=604e: -mppc; \
  mcpu=620: -mppc64; \
  mcpu=630: -mppc64; \
  mcpu=740: -mppc; \
  mcpu=750: -mppc; \
  mcpu=G3: -mppc; \
  mcpu=7400: -mppc %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=7450: -mppc %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=G4: -mppc %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=801: -mppc; \
  mcpu=821: -mppc; \
  mcpu=823: -mppc; \
  mcpu=860: -mppc; \
  mcpu=970: -mpower4 %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=G5: -mpower4 %{!mvsx:%{!maltivec:-maltivec}}; \
  mcpu=8540: -me500; \
  mcpu=8548: -me500; \
  mcpu=e300c2: -me300; \
  mcpu=e300c3: -me300; \
  mcpu=e500mc: -me500mc; \
  mcpu=e500mc64: -me500mc64; \
  mcpu=e5500: -me5500; \
  mcpu=e6500: -me6500; \
  mcpu=titan: -mtitan; \
  !mcpu*: %{mpower9-vector: -mpower9; \
	    mpower8-vector|mcrypto|mdirect-move|mhtm: -mpower8; \
	    mvsx: -mpower7; \
	    mpowerpc64: -mppc64;: %(asm_default)}; \
  :%eMissing -mcpu option in ASM_CPU_SPEC?\n} \
%{mvsx: -mvsx -maltivec; maltivec: -maltivec}" \
ASM_OPT_ANY

/* CC1_SPEC (patch 0029) */
#undef CC1_SPEC
#define	CC1_SPEC "%{G*} %(cc1_cpu)" \
"%{g: %{!fno-eliminate-unused-debug-symbols: -feliminate-unused-debug-symbols}} \
%{g1: %{!fno-eliminate-unused-debug-symbols: -feliminate-unused-debug-symbols}} \
%{msdata: -msdata=default} \
%{mno-sdata: -msdata=none} \
%{!mbss-plt: %{!msecure-plt: %(cc1_secure_plt_default)}} \
%{profile: -p} \
%{faltivec:-maltivec -include altivec.h} %{fno-altivec:-mno-altivec} \
%<faltivec %<fno-altivec \
%{vec:-maltivec -include altivec.h} %{fno-vec:-mno-altivec} \
%<fvec %<fno-vec "

#define IS_MCRT(MCRTNAME) \
  (strcmp(amigaos_crt, MCRTNAME) == 0)

/* Make most of the definitions from other compilers available */
#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()                \
  do                                            \
    {                                           \
      builtin_define_std ("PPC");		\
      builtin_define_std ("powerpc");		\
      builtin_assert ("cpu=powerpc");		\
      builtin_assert ("machine=powerpc");	\
      builtin_define_std ("AMIGA");		\
      builtin_define ("_AMIGA");		\
      builtin_define_std ("AMIGAOS");		\
      builtin_define_std ("AMIGAOS4");		\
      builtin_define_std ("amiga");		\
      builtin_define_std ("amigaos");		\
      builtin_define_std ("amigaos4");		\
      builtin_assert ("system=amigaos");	\
      if (!amigaos_crt)			\
        {					\
          error ("no CRT specified");		\
        }					\
      else if (IS_MCRT("clib2") || IS_MCRT("clib2-ts")) \
        {					\
          builtin_define_std ("CLIB2");		\
          if (IS_MCRT("clib2-ts"))		\
            builtin_define ("__THREAD_SAFE");	\
        }					\
      else if (IS_MCRT("clib4"))		\
        {					\
          builtin_define_std ("CLIB4");		\
          builtin_define ("CLIB4");		\
        }					\
      else if (IS_MCRT("ixemul"))		\
        {					\
          builtin_define_std ("ixemul");	\
          builtin_define_std ("IXEMUL");	\
        }					\
      else if (IS_MCRT("libnix"))		\
        {					\
          builtin_define_std ("libnix");	\
          builtin_define_std ("LIBNIX");	\
        }					\
      else if (IS_MCRT("newlib"))		\
        {					\
          builtin_define_std ("NEWLIB");	\
        }					\
      TARGET_OS_SYSV_CPP_BUILTINS ();		\
    }                                           \
  while (0)

#undef CPP_SPEC
#define CPP_SPEC "%{posix: -D_POSIX_SOURCE} %(cpp_os_default)"

/*#define STANDARD_INCLUDE_DIR "/GCC/include"*/
/*#undef SYSTEM_INCLUDE_DIR *//* So that the include path order is the same in native and cross compilers */
#undef LOCAL_INCLUDE_DIR

#ifndef CROSS_DIRECTORY_STRUCTURE
#define BASE_GCC_SPEC "/GCC/"
#define BASE_SDK_SPEC "/SDK/"
#else
#define BASE_GCC_SPEC EXEC_PREFIX
#define BASE_SDK_SPEC EXEC_PREFIX "ppc-amigaos/SDK/"
#endif

#define LIB_SUBDIR_TYPE_SPEC "\
%{mbaserel:/baserel; msdata|msdata=default|msdata=sysv:/small-data}\
%{msoft-float:/soft-float}"

/* default linker specs */
#undef REAL_LIBGCC_SPEC
#define REAL_LIBGCC_SPEC "\
%{static|static-libgcc: %{!use-dynld: -lgcc -lgcc_eh} %{use-dynld: -lgcc} }%{!static:%{!static-libgcc:%{!shared:%{!shared-libgcc: %{!use-dynld: -lgcc -lgcc_eh} %{use-dynld: -lgcc}}%{shared-libgcc:-lgcc}}%{shared:%{shared-libgcc:-lgcc}%{!shared-libgcc:-lgcc}}}}"


/* make newlib the default */
#define CPP_AMIGA_DEFAULT_SPEC "%{mcrt=default|!mcrt=*:%<mcrt=default -mcrt=newlib} %(cpp_newlib)"
#define LINK_AMIGA_DEFAULT_SPEC "%(link_newlib)"
#define STARTFILE_AMIGA_DEFAULT_SPEC "%(startfile_newlib)"
#define ENDFILE_AMIGA_DEFAULT_SPEC "%(endfile_newlib)"
#undef MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS {"mcrt=newlib"}


/* For specifying the include system paths, we generally use -idirafter so the include
 * paths are added at the end of the gcc default include paths. This is required for
 * fixincludes and libstdc++ to work properly
 */


/* clib2 */

#define CPP_CLIB2_SPEC "\
-idirafter %(base_sdk)clib2/include -idirafter %(base_sdk)local/clib2/include"

#define LIB_SUBDIR_CLIB2_SPEC "%{mcrt=clib2-ts:lib.threadsafe; :lib}%(lib_subdir_type)"

#define LINK_CLIB2_SPEC "\
-L%(base_sdk)clib2/%(lib_subdir_clib2) \
-L%(base_gcc)lib/gcc/ppc-amigaos/%(version)/%{mcrt=clib2-ts:clib2-ts; :clib2}/lib%(lib_subdir_type) \
-L%(base_sdk)local/clib2/%(lib_subdir_clib2)"

#define STARTFILE_CLIB2_SPEC "\
%(base_sdk)clib2/%{mcrt=clib2-ts:lib.threadsafe; :lib}" \
                 "%{!msoft-float:%(lib_subdir_type)}/crtbegin.o \
%(base_sdk)clib2/%{mcrt=clib2-ts:lib.threadsafe; :lib}" \
                 "%{!msoft-float:%(lib_subdir_type)}/crt0.o"

#define ENDFILE_CLIB2_SPEC "\
%(base_sdk)clib2/%{mcrt=clib2-ts:lib.threadsafe; :lib}" \
                 "%{!msoft-float:%(lib_subdir_type)}/crtend.o"

/* clib4 */

#define CPP_CLIB4_SPEC "\
-idirafter %(base_sdk)clib4/include -idirafter %(base_sdk)local/clib4/include"

#define LIB_SUBDIR_CLIB4_SPEC "lib%(lib_subdir_type)"

/* clib4 has baserel libraries but no small-data ones.  */
#define LINK_CLIB4_SPEC "\
%{msdata|msdata=default|msdata=sysv: \
%e-msdata is not supported with -mcrt=clib4, which has no small-data libraries} \
-L%(base_sdk)clib4/%(lib_subdir_clib4) \
-L%(base_gcc)lib/gcc/ppc-amigaos/%(version)/clib4/lib%(lib_subdir_type) \
-L%(base_sdk)local/clib4/%(lib_subdir_clib4)"

#define STARTFILE_CLIB4_SPEC "\
%{shared: %(base_sdk)clib4/%(lib_subdir_clib4)/shcrtbegin.o} %{!shared: %(base_sdk)clib4/%(lib_subdir_clib4)/crtbegin.o} %{!shared: %(base_sdk)clib4/%(lib_subdir_clib4)/crt0.o}"

#define ENDFILE_CLIB4_SPEC "\
%{shared: %(base_sdk)clib4/%(lib_subdir_clib4)/shcrtend.o} %{!shared: %(base_sdk)clib4/%(lib_subdir_clib4)/crtend.o}"

/* ixemul */

#define CPP_IXEMUL_SPEC "\
-idirafter %(base_sdk)ixemul/include -idirafter %(base_sdk)local/ixemul/include"

#define LIB_SUBDIR_IXEMUL_SPEC "lib%(lib_subdir_type)"

#define LINK_IXEMUL_SPEC "\
-L%(base_sdk)ixemul/%(lib_subdir_ixemul) \
-L%(base_gcc)lib/gcc/ppc-amigaos/%(version)/ixemul/%(lib_subdir_ixemul) \
-L%(base_sdk)local/ixemul/%(lib_subdir_ixemul)"

/* ixemul startfile should work for all library flavours */
#define STARTFILE_IXEMUL_SPEC "%(base_sdk)ixemul/%(lib_subdir_ixemul)/crtbegin.o"
#define ENDFILE_IXEMUL_SPEC "%(base_sdk)ixemul/%(lib_subdir_ixemul)/crtend.o"

/* libnix */

#define CPP_LIBNIX_SPEC "\
-idirafter %(base_sdk)libnix/include -idirafter %(base_sdk)local/libnix/include"

#define LIB_SUBDIR_LIBNIX_SPEC "lib%(lib_subdir_type)"

#define LINK_LIBNIX_SPEC "\
-L%(base_sdk)libnix/%(lib_subdir_libnix) \
-L%(base_gcc)lib/gcc/ppc-amigaos/%(version)/libnix/%(lib_subdir_libnix) \
-L%(base_sdk)local/libnix/%(lib_subdir_libnix)"

#define STARTFILE_LIBNIX_SPEC "%(base_sdk)libnix/%(lib_subdir_libnix)/crtbegin.o"
#define ENDFILE_LIBNIX_SPEC "%(base_sdk)libnix/%(lib_subdir_libnix)/crtend.o"

/* newlib */

#define CPP_NEWLIB_SPEC "\
-idirafter %(base_sdk)newlib/include -idirafter %(base_sdk)local/newlib/include"

#define LIB_SUBDIR_NEWLIB_SPEC "lib%(lib_subdir_type)"

/* The newlib baserel, small-data and soft-float directories of the SDK
   only hold the files that differ (baserel has just crtbegin.o,
   small-data just libc.a), so fall back to the plain lib directory for
   everything else.  */
#define LINK_NEWLIB_SPEC "\
-L%(base_sdk)newlib/%(lib_subdir_newlib) \
-L%(base_gcc)lib/gcc/ppc-amigaos/%(version)/newlib/%(lib_subdir_newlib) \
-L%(base_sdk)local/newlib/%(lib_subdir_newlib) \
%{mbaserel|msdata|msdata=default|msdata=sysv|msoft-float: \
-L%(base_sdk)newlib/lib -L%(base_sdk)local/newlib/lib}"

/* Spec function arguments do not expand %(...), hence BASE_SDK_SPEC and
   the variant directories spelled out.  */
#define NEWLIB_CRT_VARIANT_SPEC(dir, f) \
"%:if-exists-else(" BASE_SDK_SPEC "newlib/lib/" dir "/" f \
" " BASE_SDK_SPEC "newlib/lib/" f ")"

#define NEWLIB_CRT_SPEC(f) \
"%{mbaserel:" NEWLIB_CRT_VARIANT_SPEC ("baserel", f) ";" \
" msdata|msdata=default|msdata=sysv:" \
NEWLIB_CRT_VARIANT_SPEC ("small-data", f) ";" \
" msoft-float:" NEWLIB_CRT_VARIANT_SPEC ("soft-float", f) ";" \
" :" BASE_SDK_SPEC "newlib/lib/" f "}"

/* newlib startfile should work for all library flavours */
#define STARTFILE_NEWLIB_SPEC \
"%{shared: " NEWLIB_CRT_SPEC ("shcrtbegin.o") "} " \
"%{!shared: " NEWLIB_CRT_SPEC ("crtbegin.o") "}"

#define ENDFILE_NEWLIB_SPEC \
"%{shared: " NEWLIB_CRT_SPEC ("shcrtend.o") "} " \
"%{!shared: " NEWLIB_CRT_SPEC ("crtend.o") "}"

/* End clib specific */

/* CPP_OS_DEFAULT_SPEC - patch 0018: respect -nostdinc for SDK includes */
#undef CPP_OS_DEFAULT_SPEC
#define CPP_OS_DEFAULT_SPEC "\
%{mcrt=clib2|mcrt=clib2-ts: %(cpp_clib2); \
mcrt=clib4: %(cpp_clib4); \
mcrt=ixemul: %(cpp_ixemul); \
mcrt=libnix: %(cpp_libnix); \
mcrt=newlib: %(cpp_newlib); \
mcrt=default|!mcrt=*: %{mcrt=default|!nostdinc: %(cpp_amiga_default)}; \
: %eInvalid C runtime library} \
%{!nostdinc: -idirafter %(base_sdk)include/include_h -idirafter %(base_sdk)include/netinclude -idirafter %(base_sdk)local/common/include} \
%{mbaserel: %{msdata|msdata=default|msdata=sysv: %e-mbaserel and -msdata options are incompatible}} \
%{newlib: %e-newlib is obsolete, use -mcrt=newlib instead}"

/* LINK_SPEC - patch 0043: add --eh-frame-hdr */
#undef LINK_SPEC
#define LINK_SPEC "\
--defsym __amigaos4__=1 \
--eh-frame-hdr \
%{!shared: %{!use-dynld: -Bstatic}} \
-q -d %{h*} %{v:-V} %{G*} \
%{Wl,*:%*} %{YP,*} %{R*} \
%{Qy:} %{!Qn:-Qy} \
%(link_thread) %(link_shlib) %(link_text) \
%{mbaserel: %{msdata|msdata=default|msdata=sysv: %e-mbaserel and -msdata options are incompatible}} \
%{mcrt=clib2|mcrt=clib2-ts: %(link_clib2); \
mcrt=clib4: %(link_clib4); \
mcrt=ixemul: %(link_ixemul); \
mcrt=libnix: %(link_libnix); \
mcrt=newlib: %(link_newlib); \
mcrt=default|!mcrt=*: %(link_amiga_default); \
: %eInvalid C runtime library} \
-L%(base_sdk)local/common/lib%(lib_subdir_type) \
%{newlib: %e-newlib is obsolete, use -mcrt=newlib instead}"

/* FIXME: LINK_TEXT has been made empty now. Could we get rid of it? */
#if 0
#define LINK_TEXT "\
%{use-dynld: -Ttext=0x100000} %{!use-dynld: %{shared: -Ttext=0x100000} %{!shared: %{!Wl,-T*: %{!T*:-Ttext=0}}}}"
#else
#define LINK_TEXT ""
#endif

#define LINK_SHLIB "\
%{shared:-shared -dy --defsym __dynld_version__=1} %{!shared: %{static:-static}} %{use-dynld: -dy}"

#define LINK_THREAD "\
%{pthread:%{!mcrt=clib4:%e-pthread requires -mcrt=clib4}} \
%s%{athread=native:gthr-amigaos-native.o;athread=single:gthr-amigaos-single.o;athread=pthread:%{mcrt=clib4:gthr-amigaos-pthread.o;:%e-athread=pthread requires -mcrt=clib4}}"

#undef STARTFILE_SPEC
#define STARTFILE_SPEC "\
%{mcrt=clib2|mcrt=clib2-ts: %(startfile_clib2); \
mcrt=clib4: %(startfile_clib4); \
mcrt=ixemul: %(startfile_ixemul); \
mcrt=libnix: %(startfile_libnix); \
mcrt=newlib: %(startfile_newlib); \
mcrt=default|!mcrt=*: %(startfile_amiga_default); \
: %eInvalid C runtime library}"

#undef ENDFILE_SPEC
#define ENDFILE_SPEC "\
%{mcrt=clib2|mcrt=clib2-ts: %(endfile_clib2); \
mcrt=clib4: %(endfile_clib4); \
mcrt=ixemul: %(endfile_ixemul); \
mcrt=libnix: %(endfile_libnix); \
mcrt=newlib: %(endfile_newlib); \
mcrt=default|!mcrt=*: %(endfile_amiga_default); \
: %eInvalid C runtime library}"

/* patch 0039: pthread support in LIB_SPEC */
#undef LIB_SPEC
#define LIB_SPEC "\
--start-group %{pthread|athread=pthread:-lpthread} -lc --end-group"

#undef TARGET_DEFAULT
#define TARGET_DEFAULT 0

#undef SUBTARGET_EXTRA_SPECS
#define SUBTARGET_EXTRA_SPECS \
  {"base_gcc", BASE_GCC_SPEC}, \
  {"base_sdk", BASE_SDK_SPEC}, \
  {"cpp_os_default", CPP_OS_DEFAULT_SPEC}, \
  {"lib_subdir_type", LIB_SUBDIR_TYPE_SPEC}, \
  /* default C runtime */ \
  {"cpp_amiga_default", CPP_AMIGA_DEFAULT_SPEC}, \
  {"link_amiga_default", LINK_AMIGA_DEFAULT_SPEC}, \
  {"startfile_amiga_default", STARTFILE_AMIGA_DEFAULT_SPEC}, \
  {"endfile_amiga_default", ENDFILE_AMIGA_DEFAULT_SPEC}, \
  /* clib2 */ \
  {"cpp_clib2", CPP_CLIB2_SPEC}, \
  {"lib_subdir_clib2", LIB_SUBDIR_CLIB2_SPEC}, \
  {"link_clib2", LINK_CLIB2_SPEC}, \
  {"startfile_clib2", STARTFILE_CLIB2_SPEC}, \
  {"endfile_clib2", ENDFILE_CLIB2_SPEC}, \
  /* clib4 */ \
  {"cpp_clib4", CPP_CLIB4_SPEC}, \
  {"lib_subdir_clib4", LIB_SUBDIR_CLIB4_SPEC}, \
  {"link_clib4", LINK_CLIB4_SPEC}, \
  {"startfile_clib4", STARTFILE_CLIB4_SPEC}, \
  {"endfile_clib4", ENDFILE_CLIB4_SPEC}, \
  /* ixemul */ \
  {"cpp_ixemul", CPP_IXEMUL_SPEC}, \
  {"lib_subdir_ixemul", LIB_SUBDIR_IXEMUL_SPEC}, \
  {"link_ixemul", LINK_IXEMUL_SPEC}, \
  {"startfile_ixemul", STARTFILE_IXEMUL_SPEC}, \
  {"endfile_ixemul", ENDFILE_IXEMUL_SPEC}, \
  /* libnix */ \
  {"cpp_libnix", CPP_LIBNIX_SPEC}, \
  {"lib_subdir_libnix", LIB_SUBDIR_LIBNIX_SPEC}, \
  {"link_libnix", LINK_LIBNIX_SPEC}, \
  {"startfile_libnix", STARTFILE_LIBNIX_SPEC}, \
  {"endfile_libnix", ENDFILE_LIBNIX_SPEC}, \
  /* newlib */ \
  {"cpp_newlib", CPP_NEWLIB_SPEC}, \
  {"lib_subdir_newlib", LIB_SUBDIR_NEWLIB_SPEC}, \
  {"link_newlib", LINK_NEWLIB_SPEC}, \
  {"startfile_newlib", STARTFILE_NEWLIB_SPEC}, \
  {"endfile_newlib", ENDFILE_NEWLIB_SPEC}, \
  /* used in link spec  */ \
  {"link_text", LINK_TEXT}, \
  {"link_shlib", LINK_SHLIB}, \
  {"link_thread", LINK_THREAD},

#undef DEFAULT_VTABLE_THUNKS
#ifndef USE_GNULIBC_1
#define DEFAULT_VTABLE_THUNKS 1
#endif

#undef JUMP_TABLES_IN_TEXT_SECTION
#define JUMP_TABLES_IN_TEXT_SECTION 0

/* Used as cookie for linear vararg passing */
#define CALL_LINEARVARARGS      0x10000000

/* Enum for AmigaOS MD builtins */
enum amigaos_builtins
{
  AMIGAOS_BUILTIN_GETLINEARVA
};

#define SUB3TARGET_OVERRIDE_OPTIONS  \
do                                   \
{                                    \
  if (TARGET_ALTIVEC)                \
  {                                  \
    rs6000_altivec_abi = 1;          \
    TARGET_ALTIVEC_VRSAVE = 1;       \
  }                                  \
} while(0)

#undef SUBTARGET_INIT_BUILTINS
#define SUBTARGET_INIT_BUILTINS \
  amigaos_init_builtins ()

#undef SUBTARGET_EXPAND_BUILTIN
#define SUBTARGET_EXPAND_BUILTIN(EXP, TARGET, SUBTARGET, MODE, IGNORE, SUCCESS) \
  amigaos_expand_builtin (EXP, TARGET, SUBTARGET, MODE, IGNORE, SUCCESS)

/* AmigaOS specific attribute */
/* { name, min_len, max_len, decl_req, type_req, fn_type_req,
       affects_type_identity, handler, exclude } */
#define SUBTARGET_ATTRIBUTE_TABLE \
  { "linearvarargs", 0, 0, false, true,  true, false, amigaos_handle_linearvarargs_attribute, NULL}, \
  { "checktags", 0, 0, false, true, true, false, amigaos_handle_checktags_attribute, NULL}, \
  { "baserel_restore", 0, 0, false, true, true, false, amigaos_handle_baserel_restore_attribute, NULL }, \
  { "force_no_baserel", 0, 0, true, false, false, false, amigaos_handle_force_no_baserel_attribute, NULL }, \
  { "check68kfuncptr", 0, 0, false, true, true, false, amigaos_handle_check68kfuncptr_attribute, NULL }

/* Overrides */
#undef EXPAND_BUILTIN_VA_START
#define EXPAND_BUILTIN_VA_START(VALIST, NEXTARG) \
  amigaos_expand_builtin_va_start (VALIST, NEXTARG)

/* Disable section anchors in baserel mode (patch 0017).  */
#undef TARGET_USE_ANCHORS_FOR_SYMBOL_P
#define TARGET_USE_ANCHORS_FOR_SYMBOL_P amigaos_use_anchors_for_symbol_p

/* This target uses the amigaos.opt file.  */
#define TARGET_USES_AMIGAOS_OPT 1

/* Sentinel macro so other files can detect the AmigaOS 4 (PowerPC) target.
   Not TARGET_AMIGAOS: the m68k AmigaOS port defines that and uses it in
   shared code for things specific to hunk objects and libnix.  */
#define TARGET_AMIGAOS4 1
