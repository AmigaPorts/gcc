#define OPS(T, S) \
T S##_add(T a,T b){return a+b;} \
T S##_sub(T a,T b){return a-b;} \
T S##_mul(T a,T b){return a*b;} \
T S##_div(T a,T b){return a/b;} \
T S##_neg(T a){return -a;} \
int S##_eq(T a,T b){return a==b;} \
int S##_ne(T a,T b){return a!=b;} \
int S##_lt(T a,T b){return a<b;} \
int S##_le(T a,T b){return a<=b;} \
int S##_gt(T a,T b){return a>b;} \
int S##_ge(T a,T b){return a>=b;} \
int S##_toint(T a){return (int)a;} \
unsigned S##_touint(T a){return (unsigned)a;} \
T S##_fromint(int a){return a;} \
T S##_fromuint(unsigned a){return a;} \
T S##_load(T *p){return *p;} \
void S##_store(T *p,T a){*p=a;} \
T S##_index(T *p,int i){return p[i];}
OPS(float,f)
OPS(double,d)
float f_abs(float a){return __builtin_fabsf(a);}
double d_abs(double a){return __builtin_fabs(a);}
float d_to_f(double a){return a;}
double f_to_d(float a){return a;}
extern double external(double, float, int);
double call(double a, double b){return external(a, (float)b, 5) + a + b;}
struct pair {double a,b;};
struct pair aggregate(struct pair a){a.a+=a.b;return a;}
double varargs(int n, ...){__builtin_va_list ap;__builtin_va_start(ap,n);double s=0;for(int i=0;i<n;i++)s+=__builtin_va_arg(ap,double);__builtin_va_end(ap);return s;}
double large_frame(double a){volatile double b[128];for(int i=0;i<128;i++)b[i]=a+i;return b[99];}
struct __attribute__((packed)) packed {char c;double d;};
double packed_load(struct packed *p){return p->d;}
void packed_store(struct packed *p,double d){p->d=d;}

int select_eq(double a,double b,int yes,int no){return a==b?yes:no;}
int select_ne(double a,double b,int yes,int no){return a!=b?yes:no;}
