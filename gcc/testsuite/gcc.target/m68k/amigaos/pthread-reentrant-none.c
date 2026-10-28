/* Without -pthread, _REENTRANT stays undefined.  */

/* { dg-do preprocess } */

#ifdef _REENTRANT
#error "_REENTRANT is defined without -pthread"
#endif
