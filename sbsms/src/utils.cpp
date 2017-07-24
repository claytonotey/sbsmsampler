#include <math.h>
#include "utils.h"
#include "real.h"
#include <algorithm>
using namespace std;

namespace _sbsms_ {

int iShift;
int cosExp;
int cosSize;
float *cosTable = NULL;
float cosFactor;

int *factor(int n)
{
  int *f = (int*)calloc(ilog2(n)+1,sizeof(int));
  factor(n,f,0);
  return f;
}

void factor(int n, int *f, int m)
{
  for(int k=2;k<=n;k++) {
    if(n%k==0) {
      f[m++] = k;
      n /= k;
      break;
    }
  }
  if(n>1)
	  factor(n,f,m);
}

void c2even(audio *eo, audio *even, int N)
{
  int Nover2 = N/2;
  for(int k=0;k<=Nover2;k++) {
    int fk = (k==0)?0:N-k;    
    even[k][0] = 0.5f*(eo[k][0] + eo[fk][0]);
    even[k][1] = 0.5f*(eo[k][1] - eo[fk][1]);
  }
}

void c2odd(audio *eo, audio *odd, int N)
{
  int Nover2 = N/2;
  for(int k=0;k<=Nover2;k++) {
    int fk = (k==0)?0:N-k;
    odd[k][0] = 0.5f*(eo[k][1] + eo[fk][1]);
    odd[k][1] = 0.5f*(-eo[k][0] + eo[fk][0]);
  }
}

void cosInit(int exp)
{
  if(cosTable) free(cosTable);
  int size = 1<<exp;
  cosTable = (float*)malloc((size+1)*sizeof(float));
  for(int k=0; k<=size; k++) {
    cosTable[k] = cos((float)k/(float)size*PI);
  }
  cosInit(exp,1,cosTable);
}

void cosInit(int exp, int n, float *tab)
{
  cosExp = exp;
  cosSize = 1<<cosExp;
  iShift = 29 - cosExp;
  cosFactor = (float)cosSize/PI;
  cosTable = tab;
}

float *cosGet(int j)
{
  return cosTable + j * (cosSize+1);
}

void cosDestroy() 
{
  free(cosTable);
}

}
