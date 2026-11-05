typedef float v2sf __attribute__((vector_size(8)));
typedef unsigned v2ui __attribute__((vector_size(8)));
float lane0(v2sf x) { return x[0]; }
float lane1(v2sf x) { return x[1]; }
v2sf insert0(v2sf x,float y) { x[0]=y;return x; }
v2sf insert1(v2sf x,float y) { x[1]=y;return x; }
float sum2(v2sf x) { return x[0]+x[1]; }
float dot2(v2sf a,v2sf b) { v2sf x=a*b; return x[0]+x[1]; }
v2sf abs2(v2sf x) { return (v2sf){__builtin_fabsf(x[0]),__builtin_fabsf(x[1])}; }
v2sf pairf(float a,float b) { return (v2sf){a,b}; }
v2ui pairi(unsigned a,unsigned b) { return (v2ui){a,b}; }
v2ui splati(unsigned a) { return (v2ui){a,a}; }
float cvt_i32f(int a) { return a; }
float cvt_u32f(unsigned a) { return a; }
double cvt_i32d(int a) { return a; }
double cvt_u32d(unsigned a) { return a; }
int cvt_fi32(float a) { return a; }
unsigned cvt_fu32(float a) { return a; }
int cvt_di32(double a) { return a; }
unsigned cvt_du32(double a) { return a; }
float minimumf(float a,float b) { return a<b?a:b; }
double minimumd(double a,double b) { return a<b?a:b; }
float absf(float a) { return __builtin_fabsf(a); }
double absd(double a) { return __builtin_fabs(a); }
v2sf load2(const v2sf *p) { return *p; }
void store2(v2sf *p,v2sf a) { *p=a; }
v2sf loadpair(const float *p) { return (v2sf){p[0],p[1]}; }
void storepair(float *p,v2sf a) { p[0]=a[0]; p[1]=a[1]; }
float divf(float a,float b) { return a/b; }
double divd(double a,double b) { return a/b; }
v2sf div2(v2sf a,v2sf b) { return a/b; }
float maddf(float a,float b,float c) { return a*b+c; }
double maddd(double a,double b,double c) { return a*b+c; }
