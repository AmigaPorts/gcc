/* A malformed tagtype pragma is an error, not an internal compiler error.  */
/* { dg-do compile } */
#pragma amigaos tagtype(struct S *
/* { dg-error "expected '\\)' before end of line" "" { target *-*-* } .-1 } */
static const int TA_S = 1;
#pragma amigaos tagtype(42)
/* { dg-error "expected specifier-qualifier-list before numeric constant" "" { target *-*-* } .-1 } */
static const int TA_T = 2;
