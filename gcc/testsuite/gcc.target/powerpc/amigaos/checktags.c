/* The argument that follows a tag is checked against the type given by
   "#pragma amigaos tagtype": s2 is not the struct S1 * TA_S1 takes.  */
/* { dg-do compile } */

void TestTags(int type, ...) __attribute__((checktags)); /* { dg-message "expected 'struct S1 \\*' but argument is of type 'struct S2 \\*'" } */

struct S1;
struct S2;

enum {TA_Dummy = 0x12340000};

#pragma amigaos tagtype(struct S1 *)
enum {TA_S1 = TA_Dummy + 0x0D};

extern struct S1 *s1;
extern struct S2 *s2;

int main(void)
{
	TestTags(0, TA_S1, s1);
	TestTags(0, TA_S1, s2); /* { dg-error "passing argument 3 of 'TestTags' from incompatible pointer type" } */
	return 0;
}
