#include <spe.h>
extern __ev64_opaque__ plus(__ev64_opaque__,__ev64_opaque__),shift(__ev64_opaque__),splat(void),load(void*),selectv(__ev64_opaque__,__ev64_opaque__,__ev64_opaque__,__ev64_opaque__);
extern void store(void*,__ev64_opaque__);extern int all(__ev64_opaque__,__ev64_opaque__);
static unsigned long long bits(__ev64_opaque__ a){union{__ev64_opaque__ v;unsigned long long u;}x={a};return x.u;}
#define CHECK(x) do{if(!(x))return __COUNTER__+1;}while(0)
int test_main(void){
__ev64_opaque__ a=__ev_create_u32(1,2),b=__ev_create_u32(3,4);
CHECK(bits(a)==0x100000002ull);CHECK(bits(plus(a,b))==0x400000006ull);
CHECK(bits(shift(a))==0x800000010ull);CHECK(bits(splat())==0xfffffffdfffffffdull);
unsigned long long mem[3]={0x1234567887654321ull,0xabcdefab12345678ull,0};
CHECK(bits(load(mem))==mem[1]);store(mem,a);CHECK(mem[0]==0x1234567887654321ull);CHECK(mem[1]==0x100000002ull);CHECK(mem[2]==0);
CHECK(all(a,a));CHECK(!all(a,b));
CHECK(bits(selectv(a,__ev_create_u32(1,5),a,b))==0x100000004ull);
CHECK(bits(__ev_create_u16(0x1234,0x5678,0x9abc,0xdef0))==0x123456789abcdef0ull);
CHECK(bits(__ev_create_u64(0xfedcba9876543210ull))==0xfedcba9876543210ull);
return 0;
}
