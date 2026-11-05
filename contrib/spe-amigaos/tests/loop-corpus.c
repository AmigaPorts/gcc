/* Common loop shapes must compile with strict and fast floating point.  */

void
saxpy(float *restrict y,const float *restrict x,float a,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]+=a*x[i];
}

float
sum(const float*x,unsigned n)
{
  float a=0;
  for (unsigned i=0;i<n;i++)a+=x[i];
  return a;
}

float
dot(const float*x,const float*y,unsigned n)
{
  float a=0;
  for (unsigned i=0;i<n;i++)a+=x[i]*y[i];
  return a;
}

void
clamp(float *restrict y,const float *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=x[i]<0?0:x[i]>1?1:x[i];
}

void
cvt(float *restrict y,const int *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=x[i];
}

void
fix(int *restrict y,const float *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=(int)x[i];
}

void
neg(float *restrict y,const float *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=-x[i];
}

void
divc(float *restrict y,const float *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=x[i]/3;
}

void
gather(float *restrict y,const float *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=x[i*2]+x[i*2+1];
}

void
madd(int *restrict y,const int *restrict x,int a,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]+=a*x[i];
}

void
shift(unsigned *restrict y,const unsigned *restrict x,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=(x[i]<<3)^(x[i]>>7);
}

void
select(float *restrict y,const float *restrict a,const float *restrict b,unsigned n)
{
  for (unsigned i=0;i<n;i++)y[i]=a[i]>b[i]?a[i]:b[i];
}
