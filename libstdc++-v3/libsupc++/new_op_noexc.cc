// Nothrow operator new for -fno-exceptions programs -*- C++ -*-

// Copyright (C) 2026 Free Software Foundation, Inc.
//
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

// Built with -fno-exceptions into libnew_noexc.a (config/os/newlib/t-amigaos),
// where operator new aborts on failure.  The nothrow forms can then not be
// the try/catch around operator new that new_opnt.cc and friends use, so
// they allocate directly: a replaced operator new is not seen by them.
// Compiled once per form, selected by NEW_OP_ARRAY and NEW_OP_ALIGNED,
// giving one archive member each like libsupc++ does.

#include <bits/c++config.h>
#include <stdlib.h>
#include <stdint.h>
#include "new"

#if __cpp_exceptions
# error "only for a -fno-exceptions build"
#endif

#ifdef NEW_OP_ALIGNED
# include <bit>
# if _GLIBCXX_HAVE_ALIGNED_ALLOC || _GLIBCXX_HAVE__ALIGNED_MALLOC \
  || _GLIBCXX_HAVE_POSIX_MEMALIGN || _GLIBCXX_HAVE_MEMALIGN
#  error "keep in step with the aligned_alloc in new_opa.cc"
# endif

// Same layout as new_opa.cc without an aligned allocator in the C library:
// operator delete (void *, align_val_t) frees ((void **) p)[-1].
static inline void *
allocate (std::size_t sz, std::size_t al)
{
  if (al < sizeof (void *))
    al = sizeof (void *);
  if (sz > (std::size_t) -1 - al)
    return nullptr;
  void *const malloc_ptr = malloc (sz + al);
  if (!malloc_ptr)
    return nullptr;
  void *const aligned_ptr = (void *) (((uintptr_t) malloc_ptr + al) & -al);
  ((void **) aligned_ptr)[-1] = malloc_ptr;
  return aligned_ptr;
}
#else
static inline void *
allocate (std::size_t sz)
{
  return malloc (sz);
}
#endif

_GLIBCXX_WEAK_DEFINITION void *
#ifdef NEW_OP_ARRAY
operator new[]
#else
operator new
#endif
  (std::size_t sz,
#ifdef NEW_OP_ALIGNED
   std::align_val_t al,
#endif
   const std::nothrow_t&) noexcept
{
#ifdef NEW_OP_ALIGNED
  std::size_t align = (std::size_t) al;
  if (__builtin_expect (!std::__has_single_bit (align), false))
    return nullptr;
#endif
  /* malloc (0) is unpredictable; avoid it.  */
  if (__builtin_expect (sz == 0, false))
    sz = 1;

  void *p;
#ifdef NEW_OP_ALIGNED
  while ((p = allocate (sz, align)) == nullptr)
#else
  while ((p = allocate (sz)) == nullptr)
#endif
    {
      std::new_handler handler = std::get_new_handler ();
      if (!handler)
	return nullptr;
      handler ();
    }
  return p;
}
