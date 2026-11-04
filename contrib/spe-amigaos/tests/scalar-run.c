#define DECL(T,S) \
extern T S##_add(T,T),S##_sub(T,T),S##_mul(T,T),S##_div(T,T),S##_neg(T),S##_abs(T),S##_fromint(int),S##_fromuint(unsigned); \
extern int S##_eq(T,T),S##_ne(T,T),S##_lt(T,T),S##_le(T,T),S##_gt(T,T),S##_ge(T,T),S##_toint(T); \
extern unsigned S##_touint(T); \
extern T S##_load(T*),S##_index(T*,int);extern void S##_store(T*,T);
DECL(float,f)
DECL(double,d)
extern float d_to_f(double);
extern double f_to_d(float),call(double,double),varargs(int,...),large_frame(double);
struct pair{double a,b;};extern struct pair aggregate(struct pair);
struct __attribute__((packed)) packed{char c;double d;};extern double packed_load(struct packed*);extern void packed_store(struct packed*,double);
static unsigned fb(float f){union{float f;unsigned u;}v={f};return v.u;}
static unsigned long long db(double d){union{double d;unsigned long long u;}v={d};return v.u;}
#define CHECK(x) do{if(!(x))return __COUNTER__+1;}while(0)
/* NaNs require the OS floating-point assist handler on e500 hardware.
   Enable these cases for an OS4-hosted run, not this freestanding image. */
#ifdef TEST_IEEE_ASSIST
#define NAN_CMP(S,T) do{T nan=__builtin_nan("");CHECK(!S##_eq(nan,1));CHECK(S##_ne(nan,1));CHECK(!S##_lt(nan,1));CHECK(!S##_le(nan,1));CHECK(!S##_gt(nan,1));CHECK(!S##_ge(nan,1));}while(0)
#else
#define NAN_CMP(S,T) ((void)0)
#endif
#define CMP(S,T) do{CHECK(S##_eq(2,2));CHECK(!S##_eq(2,3));CHECK(S##_ne(2,3));CHECK(!S##_ne(2,2));CHECK(S##_lt(-1,2));CHECK(!S##_lt(2,2));CHECK(S##_le(2,2));CHECK(!S##_le(3,2));CHECK(S##_gt(3,2));CHECK(!S##_gt(2,2));CHECK(S##_ge(2,2));CHECK(!S##_ge(1,2));NAN_CMP(S,T);}while(0)
extern int select_eq(double,double,int,int),select_ne(double,double,int,int);
int test_main(void){
CHECK(fb(f_add(1.5f,2.25f))==0x40700000u);
CHECK(fb(f_sub(1.5f,2.25f))==0xbf400000u);
CHECK(fb(f_mul(1.5f,2.25f))==0x40580000u);
CHECK(fb(f_div(9,4))==0x40100000u);
CHECK(fb(f_neg(1.5f))==0xbfc00000u);
CHECK(fb(f_abs(-1.5f))==0x3fc00000u);
CHECK(f_toint(-1.75f)==-1);CHECK(f_touint(2147483648.0f)==0x80000000u);
CHECK(fb(f_fromint(-10))==0xc1200000u);CHECK(fb(f_fromuint(0x80000000u))==0x4f000000u);
CMP(f,float);
CHECK(db(d_add(1.5,2.25))==0x400e000000000000ull);
CHECK(db(d_sub(1.5,2.25))==0xbfe8000000000000ull);
CHECK(db(d_mul(1.5,2.25))==0x400b000000000000ull);
CHECK(db(d_div(9,4))==0x4002000000000000ull);
CHECK(db(d_neg(1.5))==0xbff8000000000000ull);
CHECK(db(d_abs(-1.5))==0x3ff8000000000000ull);
CHECK(d_toint(-1.75)==-1);CHECK(d_touint(2147483648.0)==0x80000000u);
CHECK(db(d_fromint(-10))==0xc024000000000000ull);CHECK(db(d_fromuint(0x80000000u))==0x41e0000000000000ull);
CMP(d,double);
CHECK(fb(d_to_f(1.5))==0x3fc00000u);CHECK(db(f_to_d(1.5f))==0x3ff8000000000000ull);
float fs[2]={1.5f,2.25f};double ds[2]={1.5,2.25};
CHECK(fb(f_load(fs))==0x3fc00000u);CHECK(fb(f_index(fs,1))==0x40100000u);f_store(fs,-1.5f);CHECK(fb(fs[0])==0xbfc00000u);
CHECK(db(d_load(ds))==0x3ff8000000000000ull);CHECK(db(d_index(ds,1))==0x4002000000000000ull);d_store(ds,-1.5);CHECK(db(ds[0])==0xbff8000000000000ull);
CHECK(db(call(1.5,2.25))==0x4029000000000000ull);
CHECK(db(varargs(7,1.0,2.0,3.0,4.0,5.0,6.0,7.0))==0x403c000000000000ull);
CHECK(db(large_frame(1.0))==0x4059000000000000ull);
struct pair pair={1.5,2.25};pair=aggregate(pair);CHECK(db(pair.a)==0x400e000000000000ull);CHECK(db(pair.b)==0x4002000000000000ull);
struct packed p={11,1.5};CHECK(db(packed_load(&p))==0x3ff8000000000000ull);packed_store(&p,-1.5);CHECK(p.c==11);CHECK(db(p.d)==0xbff8000000000000ull);
CHECK(select_eq(1,1,11,22)==11);CHECK(select_eq(1,2,11,22)==22);
CHECK(select_ne(1,1,11,22)==22);CHECK(select_ne(1,2,11,22)==11);
return 0;
}
