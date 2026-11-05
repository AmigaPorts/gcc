#include <spe.h>
unsigned long long test(void){unsigned long long m[8]={0};__ev_stdd(__ev_create_u32(1,2),m,3);return m[3];}
unsigned long long readback(void){unsigned long long m[8]={0};m[3]=0x123456789abcdef0ull;return __ev_convert_u64(__ev_ldd(m,3));}

int test_main(void){return test()!=0x100000002ull || readback()!=0x123456789abcdef0ull;}
