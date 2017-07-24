#ifndef FILTER_H
#define FILTER_H
#include <stdlib.h>

namespace _sbsms_ {

class Filter {
 public:
  Filter(int n);
  ~Filter();
  
  inline float filter(float in);
  void init();
  void clear();

  //float *x, *y;
  //float *xc, *yc;
  //float *xend, *yend;
  //float *a, *b;
  float *x,*xc,*xend;
  float *b,*bend;
  //float *bend;
  int n;
  int nmax;
};


inline float Filter :: filter(float in) 
{
  float *b = this->b;
  float *x = this->xc;
  float out = *(b) * in;  
  b+=2;
  x+=2;

  while(b <= bend) {
    if(x>xend) x = this->x;
    out += *(b) * *(x);
    out -= *(b+1) * *(x+1);
    b+=2;
    x+=2;
  }
  x = this->xc;
  *(x) = in;
  *(x+1) = out;
  x-=2; if(x<this->x) x = xend; this->xc = x;

  return out;
}

class Thiran : public Filter {
 public:
  Thiran(int nmax);
  void create(float D, int N);
};

class fbdelay 
{
 public:
  fbdelay(int n);
  ~fbdelay();
  void init(int di);
  void setDelay(int di);
  void setFB(float fb);
  float goDelay(float in);

  int di; 
  int d1;
  int size;
  int mask;
  int cursor;
  float *x;
  float *y;
  float fb;
};

class Loss : public Filter {
 public:
  Loss();
  inline float filter(float in);
  void create(float f, float c1, float c3);
};

inline float Loss :: filter(float in)
{
  float out = b[0] * in - b[3] * x[1];
  x[1] = out;
  return out;
}

template <int size>
class delay
{
 public:
  int di; 
  int d1;
  int cursor;
  float x[size];
  bool bFull;
  enum {
    mask = size-1 };

  delay() {
    bFull = false;
    cursor = 0;
  }

  void setDelay(int di) {
    this->di = di;    
    if(bFull) {
      d1 = (cursor+size-di)&(mask);
    } else {
      d1 = cursor - di;
    }
  }

  float goDelay(float in) {
    float y0;
    x[cursor] = in;
    cursor++;
    if(d1 < 0) {
      y0 = 0;
      d1++;
    } else {
      y0 = x[d1];
      d1 = (d1 + 1) & mask;
    }
    if(cursor & size) {
      bFull = true;
      cursor = 0;
    }
    return y0; 
  }
};

}

#endif
