/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMON_COMPILER_H
#define COMMON_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__CLION_IDE__)
#define __INTELLISENSE__ 1
#endif

#if defined(__INTELLISENSE__)
#define REGARG(arg, reg) arg
#else
#define REGARG(arg, reg) arg asm(reg)
#endif

#ifdef __cplusplus
}
#endif

#endif // COMMON_COMPILER_H