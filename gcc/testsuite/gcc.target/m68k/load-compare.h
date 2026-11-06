#define TEST(NAME, TYPE, QUAL) \
void NAME (QUAL TYPE *p, QUAL TYPE *q) \
{ TYPE a = *p; TYPE b = *q; \
 if (a == 0) { *p = b; *q = 1; } \
 else { *p = a - b; *q = 0; } }
TEST(qi, unsigned char,)
TEST(hi, unsigned short,)
TEST(si, unsigned int,)
TEST(volqi, unsigned char,volatile)
TEST(volhi, unsigned short,volatile)
TEST(volsi, unsigned int,volatile)
void indexed (unsigned int *p, unsigned int *q)
{
  unsigned int a = *p;
  unsigned int b = q[a];
  if (a == 0) { *p = b; *q = 1; }
  else { *p = a - b; *q = 0; }
}
void shared (unsigned int *p)
{
  unsigned int a = *p++;
  unsigned int b = *p;
  if (a == 0) *p = b + 1;
  else *p = a - b;
}
