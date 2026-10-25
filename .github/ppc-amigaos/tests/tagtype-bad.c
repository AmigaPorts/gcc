/* A malformed tagtype pragma is an error, not an internal compiler error.  */
#pragma amigaos tagtype(struct S *
static const int TA_S = 1;
#pragma amigaos tagtype(42)
static const int TA_T = 2;
