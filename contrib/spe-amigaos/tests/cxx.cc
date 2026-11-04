#include <spe.h>
template<typename T> __attribute__((noinline)) T square(T x){return x*x;}
struct Pair { double x,y; Pair(double a,double b):x(a),y(b){} double sum()const{return x+y;} };
extern "C" int test_main(){Pair p(square(1.5),square(2.0)); __ev64_opaque__ a=__ev_create_u32(1,2); return p.sum()!=6.25 || !__ev_all_eq(__ev_addw(a,a),__ev_create_u32(2,4));}
