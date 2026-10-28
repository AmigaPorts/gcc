/* -pthread defines _REENTRANT, as on other POSIX-like targets.  */

/* { dg-do preprocess } */
/* { dg-additional-options "-pthread" } */

#ifndef _REENTRANT
#error "-pthread does not define _REENTRANT"
#endif
