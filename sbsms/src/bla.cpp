
  if(bStart && tailStart) {
    if(fabsf(w1*mScale) < PI) {
      if(offset == 0.0f) {
        iw = lrintf(w1 * mScale * iScale);
      }
      if(n < 0) {
        if(h2 == 0.0f) {
          dm = 0.0f;
        } else {
          int iStart = lrintf(h2 * offset);
          dm = m1 / (float)this->rise;
          if(iStart > 0) {
            dm = max(dm, m / (float)max(-n,iStart));
          }
        }
        int iph = lrintf(ph * iScale);
        float *end = out - n;
        while(out != end) {
          if(m <= 0.0f) { m = 0.0f; break; }
          *(out++) += m * tab[abs(iph)>>iShift];
          m -= dm;
          iph += iw;
          if(iph>iPI) iph -= i2PI;
          else if(iph<-iPI) iph += i2PI;
        }
        ph = (float)iph / iScale;
      } else {
        int iStart;
        float mmax = m1; 
        if(h2 == 0.0f) {
          iStart = 0;
          dm = 0.0f;
        } else {
          if(offset == 0.0f) {
            iStart = lrintf(h2);
          } else {
            iStart = lrintf(h2 - h2 * offset);
          }
          dm = mmax / (float)this->rise;
          int rise = min(this->rise,max(n,iStart));
          dm = max(dm,(m1 - m) / (float)rise);
          iStart = max(0,iStart-rise);
          if(offset == 0.0f) ph = canon(ph - w0 * mScale * rise);
        }
        if(iStart < n) {
          int iph = lrintf(ph * iScale);
          float *end = out + n;
          out += iStart;
          while(out != end) {
            *(out++) += m * tab[abs(iph)>>iShift];
            m += dm;
            if(m > mmax) m = mmax;
            iph += iw;
            if(iph>iPI) iph -= i2PI;
            else if(iph<-iPI) iph += i2PI;
          }
          ph = (float)iph / iScale;
        }
      }
    }
  } else if(bEnd && tailEnd) {
    if(fabsf(w0*mScale) < PI) {
      if(offset == 0.0f) {     
        iw = lrintf(w0 * mScale * iScale);
      }
      if(n < 0) {
        int iStart;
        float mmax = m0;
        if(h2 == 0.0f) {
          iStart = 0;
          dm = 0.0f;
        } else {
          iStart = lrintf(h2 * offset);
          dm = m0 / (float)this->fall;
          int rise = min(this->fall,max(-n,iStart));
          dm = max(dm,(mmax - m) / (float)rise);
          iStart = max(0,iStart-rise);
        }
        if(iStart < -n) {
          int iph = lrintf(ph * iScale);
          float *end = out - n;
          out += iStart;
          while(out != end) {
            *(out++) += m * tab[abs(iph)>>iShift];
            m += dm;
            if(m > mmax) m = mmax;
            iph += iw;
            if(iph>iPI) iph -= i2PI;
            else if(iph<-iPI) iph += i2PI;
          }
          ph = (float)iph / iScale;
        }
      } else {
        if(h2 == 0.0f) {
          dm = 0.0f;
        } else {
          int iStart;
          if(offset == 0.0f) {
            iStart = lrintf(h2);
          } else {
            iStart = lrintf(h2 - h2 * offset);
          }
          dm = m0 / (float)this->fall;
          int fall = min(this->fall,max(n,iStart));
          dm = max(dm, m / (float)fall);
        }
        int iph = lrintf(ph * iScale);
        float *end = out + n;
        while(out != end) {
          if(m <= 0.0f) { m = 0.0f; break; }
          *(out++) += m * tab[abs(iph)>>iShift];
          m -= dm;
          iph += iw;
          if(iph>iPI) iph -= i2PI;
          else if(iph<-iPI) iph += i2PI;
        }
        ph = (float)iph / iScale;
      }
    }
  } else  {
    float b2;
    if(h2 == 0.0f) {
      b2 = 0.0f;
      dm = 0.0f;
      w = w0 + offset * (w1 - w0);
    } else {
      if(offset == 0.0f) {
        w = w0;
      } else {
        w = w0 + offset * (w1 - w0);
      }
      if(n < 0) {
        float dt = (float)max(-n,(int)lrintf(offset * h2));
        dm = (m - m0) / dt;
        b2 = (w - w0) / dt;
        w -= 0.5f * b2;
      } else {
        float dt = (float)max(n,(int)lrintf(h2 - offset * h2));
        dm = (m1 - m) / dt;
        b2 = (w1 - w) / dt;
        w += 0.5f * b2;
      }
    }
    if(fabsf(w0 * mScale < PI) && fabsf(w1 * mScale < PI)) {
      int ib2 = lrintf(b2 * iScale);
      int iw = lrintf(w * iScale);
      int iph = lrintf(ph * iScale);    
      if(n < 0) {
        float *end = out - n;
        while(out != end) {
          int idph = iw>>logM;
          if(idph < iPI && idph > -iPI) *out += m * tab[abs(iph)>>iShift];
          out++;
          m -= dm;
          iph += idph;
          iw -= ib2;
          if(iph>iPI) iph -= i2PI;
          else if(iph<-iPI) iph += i2PI;
        }
      } else {
        float *end = out + n;
        while(out != end) {
          int idph = iw>>logM;
          if(idph < iPI && idph > -iPI) *out += m * tab[abs(iph)>>iShift];
          out++;
          m += dm;
          iph += idph;
          iw += ib2;
          if(iph>iPI) iph -= i2PI;
          else if(iph<-iPI) iph += i2PI;
        }
      }
      ph = (float)iph / iScale;
    } else if(fabsf(w0 * mScale) >= PI && fabsf(w1 * mScale) >= PI) {
      ph = canon(ph + 0.5f * (w0 + w1) * mScale * n);
    } else {
      w *= mScale;
      b2 *= mScale;
      if(n < 0) {
        float *end = out - n;
        while(out != end) {
          if(w < PI && w > -PI) *out += m * tab[lrintf(cosFactor*fabsf(ph))];
          out++;
          ph += w;
          w -= b2;
          m -= dm;
          while(ph>PI) ph -= TWOPI;
          while(ph<-PI) ph += TWOPI;
        }
      } else {
        float *end = out + n;
        while(out != end) {
          if(w < PI && w > -PI) *out += m * tab[lrintf(cosFactor*fabsf(ph))];
          out++;
          ph += w;
          w += b2;
          m += dm;
          while(ph>PI) ph -= TWOPI;
          while(ph<-PI) ph += TWOPI;
        }
      }
    }    
  }


Scrub :: Scrub()
{
  start = clock();
  v = 0;
}

void Scrub :: update(float pos)
{
  now = clock();
  float dt = (float)(now - then) / (float)CLOCKS_PER_SEC;
  float v1 = (here - there) / dt;
  v = v1;
}

float Scrub ::  getPos()
{
  return here;
}

float Scrub ::  getVelocity()
{
  return v;
}

class Scrub {
public:
  Scrub();
  void update(float pos);
  void getPos();
  void getVelocity();
  float here;
  clock_t start;
  clock_t then;
  clock_t now;
};



delay :: delay(int n)
{
  init(n);
}

delay :: ~delay()
{
  free(x);
}

void delay :: init(int di)
{
  // turn size into a mask for quick modding
  size = di;
  int p = 0;
  while(size) {
    size /= 2;
    p++;
  }
  size = 1;
  while(p) {
    size *= 2;
    p--;
  }
  mask = size - 1;

  x = (float*) malloc(size*sizeof(float*));
  bFull = false;
  cursor = 0;
}

void delay :: setDelay(int di)
{
  this->di = di;
  
  if(bFull) {
    d1 = (cursor+size-di)&(mask);
  } else {
    d1 = cursor - di;
  }
}

float delay :: goDelay(float in)
{
  float y0;
  if(d1 < 0) {
    y0 = 0;
    d1++;
  } else {
    y0 = x[d1];
    d1 = (d1 + 1) & mask;
  }
  x[cursor] = in;
  cursor++;
  if(cursor & size) {
    bFull = true;
    cursor = 0;
  }

  return y0; 
}

}

  
 float loadr_1 = alpha * a1_4;

 a1 = -a0;
 a0_2 = a0_3 + load_7;
 a1_3 = a1_2 + load_7;
 float a = fracdelay->filter(loadr_1);
 a0_4 = loss->filter(a);

#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>

using namespace std;
class Tempos {
public:
  vector<pair<float,string> > tempos;
  Tempos() {
    tempos.push_back(pair<float,string>(1.0/64.0,string("1/64")));
    tempos.push_back(pair<float,string>(1.0/48.0,string("1/48")));
    tempos.push_back(pair<float,string>(1.0/32.0,string("1/32")));
    tempos.push_back(pair<float,string>(1.0/24.0,string("1/24")));
    tempos.push_back(pair<float,string>(1.0/16.0,string("1/16")));
    tempos.push_back(pair<float,string>(1.0/12.0,string("1/12")));
    tempos.push_back(pair<float,string>(1.0/8.0,string("1/8")));
    tempos.push_back(pair<float,string>(1.0/6.0,string("1/6")));
    tempos.push_back(pair<float,string>(3.0/16.0,string("3/16")));
    tempos.push_back(pair<float,string>(1.0/4.0,string("1/4")));
    tempos.push_back(pair<float,string>(1.0/3.0,string("1/3")));
    tempos.push_back(pair<float,string>(3.0/8.0,string("3/8")));
    tempos.push_back(pair<float,string>(1.0/2.0,string("1/2")));
    tempos.push_back(pair<float,string>(3.0/4.0,string("3/4")));
    tempos.push_back(pair<float,string>(1.0,string("1/1")));
    tempos.push_back(pair<float,string>(3.0/2.0,string("3/2")));
    tempos.push_back(pair<float,string>(2.0,string("2/1")));
    tempos.push_back(pair<float,string>(3.0,string("3/1")));
    tempos.push_back(pair<float,string>(4.0,string("4/1")));
  }
};

Tempos tempos;
 
 inline bool tempoCmp(const pair<float,string> &a, const pair<float,string> &b) {
   return a.first < b.first;
 }

inline float closestTempo(float f)
{
  if(f < tempos.tempos[0].first) {
    return tempos.tempos[0].first;
  } else if(f > tempos.tempos.back().first) {
    return tempos.tempos.back().first;
  }
  float dmin = 1e6;
  float vmin = 0.0;
  for(vector< pair<float,string> >::iterator k=tempos.tempos.begin(); k != tempos.tempos.end(); ++k) {
    printf("%g\n",k->first);
    float d = fabsf(k->first - f);
    if(d < dmin) {
      dmin = d;
      vmin = k->first;
    }
  }
  return vmin;
}


int main(int c, char **v) 
{
  printf("%d\n",tempos.tempos.size());
  printf("%s\n",tempos.tempos.at(0).second.c_str());
  float t = atof(v[1]);
  printf("%g %g\n",t,closestTempo(t));
}
