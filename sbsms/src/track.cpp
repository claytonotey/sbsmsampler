#include "track.h"
#include "synth.h"
#include "synthTable.h"

namespace _sbsms_ {

Track :: Track(int M, int res, float h1, int index)
{
  this->M = M;
  this->res = res;
  this->h1 = h1;
  this->index = index;
  logM = ilog2(M);
  mScale = 1.0f / (float)M;
  end = -1;
  tailEnd = 0;
  tailStart = 0;
  gen = NULL;
}


bool Track :: synth(int c,
                    float *out,
                    countType synthtime,
                    float h2,
                    float offset,
                    int n,
                    float fScale0,
                    float fScale1)
{
  TrackPoint *tp0 = getTrackPoint(synthtime);
  TrackPoint *tp1 = getTrackPoint(synthtime+1);
  if(!tp0 || !tp1) return false;
  bool bStart = (synthtime == start);
  bool bEnd = (synthtime+1 == end);
  
  if(!gen) gen = new Osc(c);

  if(offset == 0.0f) {
    w00 = tp0->f;
    w11 = tp1->f;
    
    if(bStart) {
      Track *precursor = getPrecursor();
      if(precursor) {
        state = precursor->stateDescendant;
  if(M == 16 && c == 0&& mScale*tp1->y  > 0.01) 
    printf("4 %lld %g %g\n",synthtime*16,state.ph,mScale*tp1->y);

      }
    }
    w00 *= mScale;
    w11 *= mScale;
  }


  
  gen->state = state;
  GenParams params;
  params.tab1 = distSynthTable.getDistSynthTable1(0);
  params.tab2 = distSynthTable.getDistSynthTable2(0);
  gen->setParams(&params);

  float ph = state.ph;

  float m0 = mScale * tp0->y;
  float m1 = mScale * tp1->y;

  float w0 = w00 * fScale0;
  float w1 = w11 * fScale1;

  float dm;
  float *in = NULL;
  if(bStart && tailStart) {
    if(offset == 0.0f) {
        gen->w = w1;
      }
      if(n < 0) {
      int iStart;
      int rise;
      if(h2 == 0.0f) {
        iStart = 0;
        dm = 0.0f;
        rise = -n;
      } else {
        iStart = lrintf(h2 - h2 * offset);
        rise = min(max(0,(int)lrintf(this->rise - h2 + h2*offset)),max(-n,iStart));
        dm = gen->m / rise;
        rise = min(-n,rise);
      }
      gen->gen(0,-dm,in,out,rise);
      } else {
        int iStart;
        if(h2 == 0.0f) {
          iStart = 0;
          dm = 0.0f;
        } else {
          iStart = lrintf(h2 - h2 * offset);
          int rise = min(this->rise,max(n,iStart));
          dm = (m1 - gen->m) / rise;
          iStart = max(0,iStart-rise);
          //XXX
          if(offset == 0.0f) {
            gen->state.ph = canon(gen->state.ph - w0 * rise);
          }
        }
        if(iStart < n) {
          out += iStart;
          in += iStart;
          gen->gen(0,dm,in,out,n-iStart);
        }
      }
    } else if(bEnd && tailEnd) {
      if(offset == 0.0f) {
        gen->w = w0;
      }
      if(n < 0) {
        int iStart;
        if(h2 == 0.0f) {
          iStart = 0;
          dm = 0.0f;
        } else {
          iStart = lrintf(h2 * offset);
          int rise = min(this->fall,max(-n,iStart));
          dm = (m0 - gen->m) / rise;
          iStart = max(0,iStart-rise);
        }
        if(iStart < -n) {
          out += iStart;
          in += iStart;
          gen->gen(0,dm,in,out,-n-iStart);
        }
      } else {
        int fall;
        if(h2 == 0.0f) {
          dm = 0.0f;
          fall = n;
        } else {
          int nFrame = lrintf(h2 - h2 * offset);
          fall = min(max(0,(int)lrintf(this->fall-h2*offset)),max(n,nFrame));
          dm = gen->m / (float)fall;
          fall = min(n,fall);
        }
        gen->gen(0,-dm,in,out,fall);
      }
    } else {
      float dw;
      if(h2 == 0.0f) {
        dw = 0.0f;
        dm = 0.0f;
        gen->w = w0 + offset * (w1 - w0);
      } else {
        if(offset == 0.0f) {
          gen->w = w0;
        } else {
          gen->w = w0 + offset * (w1 - w0);
        }
        if(n < 0) {
          float dt = (float)max(-n,(int)lrintf(offset * h2));
          dm = (gen->m - m0) / dt;
          dw = (gen->w - w0) / dt;
          gen->w -= 0.5f * dw;
        } else {
          float dt = (float)max(n,(int)lrintf(h2 - offset * h2));
        dm = (m1 - gen->m) / dt;
        dw = (w1 - gen->w) / dt;
        gen->w += 0.5f * dw;
        }
      }
      if(n < 0) {
        gen->gen(-dw,-dm,in,out,-n);
      } else {
        gen->gen(dw,dm,in,out,n);
      }   
    } 
  


  if(bEnd) {
    if(n >= 0 && offset == 0.0f) {
      Track *descendant = getDescendant();
      if(descendant && descendant->M < M) {
        stateDescendant = state;
        if(M ==32 && c == 0 && mScale*tp0->y > 0.01) 
          printf("5 %lld %g %g\n",32*synthtime,stateDescendant.ph,mScale*tp0->y);
      }
    }
    state = gen->state;
  } else if(bStart && tailStart) {
    state = gen->state;
  } else {
    state = gen->state;
    if(n >= 0) {
      Track *descendant = getDescendant();
      if(descendant && descendant->M > M) {
        stateDescendant = state;
      }
    }
  }

  return true;

}

template<>
void TrackImp<TrackPoint2> :: push_back(TrackPoint2 *p)
{
  point.push_back(p);
  p->owner = this;
}

template<>
TrackImp<TrackPoint1> :: ~TrackImp() {
  if(precursor) precursor->descendant = NULL;
  if(descendant) descendant->precursor = NULL;
  if(tailStart) delete point.front();
  if(tailEnd) delete point.back();
  if(gen) delete gen;
}

}
