#ifndef UTILS_H
#define UTILS_H

#include "real.h"
#include "sbsms.h"
#include "config.h"
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

namespace _sbsms_ {

#define ONEOVERTWOPI 0.15915494309189533576888376337251f
#define PI 3.1415926535897932384626433832795f
#define PIOVER2 1.5707963267948966192313216916398f
#define TWOPI 6.28318530717958647692528676655900576f

#define iPI 536870912
#define i2PI 1073741823
#define iScale 1.708913188941079e8f

extern int iShift;
extern int cosExp;
extern int cosSize;
extern float *cosTable;
extern float cosFactor;

void c2even(audio *eo, audio *even, int N);
void c2odd(audio *eo, audio *odd, int N);
void cosInit(int size);
void cosInit(int size, int n, float *tab);
float *cosGet(int j);
void cosDestroy();
inline float square(float x);
inline float canon(float ph);
inline float norm(audio x);
inline float norm2(audio x);
inline float sign(float x);
inline float dBApprox(float x);
int *factor(int n);
void factor(int n, int *f, int m);

#define MINSTRETCHMOD 0.0f
#define MAXSTRETCHMOD 4.0f

enum {
  SBSMS_OFFSET_VERSION = 0,
  SBSMS_OFFSET_SAMPLES = 4,
  SBSMS_OFFSET_FRAMES = 12,
  SBSMS_OFFSET_CHANNELS = 16,
  SBSMS_OFFSET_QUALITY = 18,
  SBSMS_OFFSET_MAXTRACKS = 122,
  SBSMS_OFFSET_DATA = 124
};
 
#ifdef _WIN32
#define FOPEN(fp,fn,mode) fopen_s(&fp,fn,mode)
#else
#define FOPEN(fp,fn,mode) (fp = fopen(fn,mode))
#endif

#ifdef ARCHI386
#undef BIGENDIAN
#endif

#ifdef ARCHPPC
#define BIGENDIAN 1
#endif

#ifdef HAVE_FSEEKO
#define FSEEK fseeko
#define FTELL ftello
#else
#define FSEEK fseek
#define FTELL ftell
#endif

#define LINMAX 65535
#define LINMAXF 65535.0f


inline float pow10f(float x) 
{
  return expf(2.302585092994046f * x);
}

inline int encodeLinear(float x, float min, float max)
{
  if(x<min) return 0;
  else if(x>max) return LINMAX;
  else return lrintf(LINMAXF*(x-min)/(max-min));
}

inline float decodeLinear(int x, float min, float max)
{
  return min + (max-min)*(float)x/LINMAXF;
}

#define MU 65535
#define MUF 65535.0f
#define MU1 65536.0f
#define MUI 1.525902189669642e-05f
#define LOGMU 11.090354888959125f

inline int encodeMu(float x)
{
  x = log(1.0f + MUF * x) / LOGMU;
  int y = lrintf(MUF * x);
  if(y > MU) y = MU;
  return y;
}

inline float decodeMu(int x)
{
  return MUI * (pow(MU1, MUI * (float)x) - 1.0f);
}


inline unsigned short bswap_16(unsigned short x) {
  return (x>>8) | (x<<8);
}

inline unsigned int bswap_32(unsigned int x) {
  return (bswap_16((unsigned short)(x&0xffff))<<16) | (bswap_16((unsigned short)(x>>16)));
}

inline unsigned long long bswap_64(unsigned long long x) {
  return (((unsigned long long)bswap_32((unsigned int)(x&0xffffffffull)))<<32) | (bswap_32((unsigned int)(x>>32)));
}

template<class T>
inline T byteSwap(T &x)
{
	T y = 0;
	int shiftl = 0;
	int shiftr = (sizeof(T)-1)<<3;
	for(int i=0; i<sizeof(T); i++) {
		y |= ((x>>shiftr)&0xf)<<shiftl;
		shiftl += 8;
		shiftr -= 8;
	}
	return y;
}

// assume sizeof(long int) >= 4 and sizeof(int) >= 2

inline unsigned short byteSwapShort(unsigned short x)
{
#if SIZEOF_SHORT == 2
  return bswap_16(x);
#else
	return byteSwap<unsigned short>(x);
#endif
}

inline unsigned int byteSwapInt(unsigned int x)
{
#if SIZEOF_INT == 4 
  return bswap_32(x);
#else
	return byteSwap<unsigned int>(x);
#endif
}

inline unsigned long byteSwapLong(unsigned long int x)
{
#if SIZEOF_LONG == 4 
  return bswap_32(x);
#elif SIZEOF_LONG == 8
	return bswap_64(x);
#else
	return byteSwap<unsigned long int>(x);
#endif
}

inline unsigned long long int byteSwapLongLong(unsigned long long int x)
{
#if SIZEOF_LONG_LONG == 8
  return bswap_64(x);  
#elif SIZEOF_LONG_LONG == 4
  return bswap_32(x);
#else
	return byteSwap<unsigned long long int>(x);
#endif
}

inline unsigned short nativeEndianShort(unsigned short x)
{
#ifdef BIGENDIAN
	return byteSwapShort(x);
#else
	return x;
#endif
}

inline unsigned int nativeEndianInt(unsigned int x)
{
#ifdef BIGENDIAN
	return byteSwapInt(x);
#else
	return x;
#endif
}

inline unsigned long int nativeEndianLong(unsigned long int x)
{
#ifdef BIGENDIAN
	return byteSwapLong(x);
#else
	return x;
#endif
}

inline unsigned long long int nativeEndianLongLong(unsigned long long int x)
{
#ifdef BIGENDIAN
	return byteSwapLongLong(x);
#else
	return x;
#endif
}

inline unsigned short littleEndianShort(unsigned short x)
{
#ifdef BIGENDIAN
	return byteSwapShort(x);
#else
	return x;
#endif
}

inline unsigned int littleEndianInt(unsigned int x)
{
#ifdef BIGENDIAN
	return byteSwapInt(x);
#else
	return x;
#endif
}

inline unsigned long int littleEndianLong(unsigned long int x)
{
#ifdef BIGENDIAN
	return byteSwapLong(x);
#else
	return x;
#endif
}

inline unsigned long long int littleEndianLongLong(unsigned long long int x)
{
#ifdef BIGENDIAN
	return byteSwapLongLong(x);
#else
	return x;
#endif
}

inline int fread_8_little_endian(FILE *fp) 
{	
  unsigned char y;
  fread(&y,1,1,fp);
  return (int)y;
}

inline int fread_16_little_endian(FILE *fp) 
{
#if SIZEOF_SHORT >= 2	
  unsigned short y = 0;
  fread(&y,2,1,fp);
  return (int)nativeEndianShort(y);
#elif SIZEOF_INT >= 2
  unsigned int y = 0;
  fread(&y,2,1,fp);
  return (int)nativeEndianInt(y);
#else
  abort();
#endif
}

inline long int fread_32_little_endian(FILE *fp) 
{
#if SIZEOF_INT >= 4	
  unsigned int y = 0;
  fread(&y,4,1,fp);
  return (long int)nativeEndianInt(y);
#elif SIZEOF_LONG >= 4
  unsigned long int y = 0L;
  fread(&y,4,1,fp);
  return (long int)nativeEndianLong(y);
#else
  abort();
#endif
}

inline long long int fread_64_little_endian(FILE *fp) 
{
#if SIZEOF_LONG_LONG >= 8
  unsigned long long int y = 0LL;
  fread(&y,8,1,fp);
  return (long long int)nativeEndianLongLong(y);
#elif SIZEOF_LONG_LONG >= 4
  unsigned long long int y = 0LL;
  fread(&y,4,1,fp);	
  unsigned long long int z;
  fread(&z,4,1,fp);
  return (long long int)nativeEndianLongLong(y);
#else
  abort();
  return 0LL;
#endif
}

inline size_t fwrite_8_little_endian(int x, FILE *fp) 
{
  unsigned char y = (unsigned char)x;
  return fwrite(&y,1,1,fp);
}

 inline size_t fwrite_16_little_endian(int x, FILE *fp) 
{
#if SIZEOF_SHORT >= 2
  unsigned short y = littleEndianShort((unsigned short)x);
  return fwrite(&y,2,1,fp);
#elif SIZEOF_INT >= 2
  unsigned int y = littleEndianInt((unsigned int)x);
  return fwrite(&y,2,1,fp);
#else
  abort();
  return 0;
#endif
}

inline size_t fwrite_32_little_endian(long int x, FILE *fp) 
{
#if SIZEOF_INT >= 4
  unsigned int y = littleEndianInt((unsigned int)x);
  return fwrite(&y,4,1,fp);
#elif SIZEOF_LONG >= 4
  unsigned long int y = littleEndianLong((unsigned long int)x);
  reurn fwrite(&y,4,1,fp);
#else
  abort();
#endif
}

inline size_t fwrite_64_little_endian(long long int x, FILE *fp) 
{
#if SIZEOF_LONG_LONG >= 8
  unsigned long long int y = littleEndianLongLong((unsigned long long int)x);
  return fwrite(&y,8,1,fp);	
#elif SIZEOF_LONG_LONG >= 4
  unsigned long long int y = littleEndianLongLong((unsigned long long int)x);
  size_t ret = fwrite(&y,4,1,fp);
  y = 0LL;
  if(ret == 4) ret += fwrite(&y,4,1,fp);
  return ret;
#else
  abort();
#endif
}

inline int ilog2(int x)
{
	int n = 0;
	while(x>1) {
		x>>=1;
		n++;
	}
	return n;
}

// 20 * log10(x)
inline float dBApprox(float x) 
{
  float u = (x-1.0f)/(x+1.0f);
  float u2 = u*u;
  return 17.37177927613007f*u*(1.0f + u2*(0.333333333333333f + u2*(0.2f + u2*0.14285714285714f)));
}


inline float canonPI(float ph) 
{
  ph -= TWOPI * lrintf(ph * ONEOVERTWOPI);
  if(ph < -PI) ph += TWOPI;
  else if(ph >= PI) ph -= TWOPI;
  return ph;
}

inline float canon(float ph) 
{
  return ph - TWOPI*(float)lrintf(ph*ONEOVERTWOPI);
}

inline float norm2(audio x)
{
  return square(x[0]) + square(x[1]);
}

inline float norm(audio x)
{
  return sqrt(norm2(x));
}

inline float square(float x)
{ 
  return x*x;
}

}

#endif
