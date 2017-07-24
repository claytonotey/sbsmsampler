#include "gen.h"
#include "filter.h"
#include "track.h"
#include "synthTable.h"
#include "vstsbsms.h"

extern void blob();

namespace _sbsms_ {


Gen :: Gen(int c)
{
  this->c = c;
  m = 0.0;
  x = 1.0;
  v = 0.0;
  pAM = 0;
  pFM = 0;
}

GenState :: GenState()
{
  memset(this,0,sizeof(GenState));
}

Osc :: Osc(int c) : Gen(c) 
{
}

void Osc :: setParams(GenParams *params)
{
  this->tab1 = params->tab1;
  this->tab2 = params->tab2;
}


void Osc :: gen(float dw, float dm, float *in, float *out, int n)
{
  int iph = lrintf(state.ph * WScale);
  if(iph>=W2PI) iph -= W2PI;
  else if(iph<0) iph += W2PI;
  int iw = lrintf(w * WScale);
  int idw = lrintf(dw * WScale);
  float *end = out + n;
  float m = this->m * MScale;
  dm *= MScale;
  while(out != end) {
    if(iw < WPI) {
      long f = (iph>>PhShift)&Ph1;
      long i = iph>>WShift;
      *out += m * (float)(tab1[i] + f * tab2[i]);
    }
    iph += iw;
    iw += idw;
    iph &= W2PIMask;
    m += dm;
    out++;
  }

  w = (float)iw / WScale;
  state.ph = (float)iph / WScale;
  this->m = m / MScale;
}

Bandpass :: Bandpass(int c) : Gen(c) 
{
  Q = 16.0;
}

void Bandpass :: setParams(GenParams *params)
{
  this->Q = params->Q;
}

void Bandpass :: gen(float dw, float dm, float *in, float *out, int n)
{
  float y0;
  
  float a,a2,aoQ,d;
  if(!n) return;

  a = 1.0/(2.0*tan(0.5*w));
  a2 = a*a;
  aoQ = a/Q;
  d = (4*a2+2*aoQ+1);

  float A1, A2, B0;
  float A11, A21, B01;

  A1 = -(8*a2-2) / d;
  A2 = (4*a2 - 2*aoQ + 1) / d;
  B0 = 2*aoQ/d;

  int nout = n;
  float w1 = w+n*dw;
  
  if(w1 > M_PI) {
    if(w < M_PI) {
      nout = min(n,(int)lrintf((M_PI - w) / dw));
      w1 = M_PI;
      a = 0.0;
    } else {
      nout = 0;
    }
  } else {
    a = 1.0/(2.0*tan(0.5*w1));
  }

  a2 = a*a;
  aoQ = a/Q;
  d = (4*a2+2*aoQ+1);

  A11 = -(8*a2-2) / d;
  A21 = (4*a2 - 2*aoQ + 1) / d;
  B01 = 2*aoQ/d;

  float dA1, dA2, dB0;
  if(nout != 0) {
    dA1 = (A11 - A1) / nout;
    dA2 = (A21 - A2) / nout;
    dB0 = (B01 - B0) / nout;
  }

  float *end = out + nout;
  while(out != end) {
    float y0 = (B0 * (*in - state.x1) - A1 * state.y0 - A2 * state.y1);
    state.y1 = state.y0;
    state.y0 = y0;
    state.x1 = state.x0;
    state.x0 = *in;
    
    *out += m * y0;
    in++;
    out++;
    m += dm;
    A1 += dA1;
    A2 += dA2;
    B0 += dB0;
  }

  w = w1;
}

Delay :: Delay(int c) : Gen(c) 
{
  del = new fbdelay(4095);
  fracdelay = new Thiran(8);
}

Delay :: ~Delay()
{
  delete del;
  delete fracdelay;
}

void Delay :: setParams(GenParams *params)
{
  del->setFB(params->fb);
}

void Delay :: gen(float dw, float dm, float *in, float *out, int n)
{
  if(w < PI) {
    setFreq(w);
    for(int i=0; i<n; i++) {
      float o = del->goDelay(in[i]);		
      o = fracdelay->filter(o);
      out[i] += m * o;
      m += dm;
    }
    w += n * dw;
  }
}

void Delay :: setFreq(float f0)
{
	float deltot = TWOPI/f0;
	int del1 = lrintf(deltot - 6.0);
	if(del1<0)
		del1 = 0;
	float D = deltot - del1;
  int N = (int)D;
  fracdelay->create(D,N);
  del1 = min(del1,4095);
  del->setDelay(del1);
}

Decimator :: Decimator(int c) : Gen(c) 
{
  cursor = 0;
  hold = 0;
  samplesToHold = 0;
  multiplier = 2147483648.0;
}

void Decimator :: setParams(GenParams *params)
{
	multiplier = pow(2.0,params->bits);
}

void Decimator :: gen(float dw, float dm, float *in, float *out, int n)
{
	for(int i=0;i<n;i++) {		
    cursor += 1.0;
		if(cursor > samplesToHold) {
			cursor -= samplesToHold;
			hold = lrintf(multiplier*in[i]) /  multiplier;
		}
    samplesToHold = (TWOPI / w);
		out[i] += m * hold;
    w += dw;
    m += dm;
	}
}


Granugrain :: Granugrain(int channels, int maxSize)
{
  this->channels = channels;
  this->maxSize = 0;
  this->pb = 1.0f;
  for(int c=0; c<channels; c++) {
    inputs[c] = NULL;
  }
  reset(maxSize);
}

Granugrain :: ~Granugrain()
{
  for(int c=0; c<channels; c++) {
    delete inputs[c];
  }
}

void Granugrain :: reset(int maxSize)
{
  size = 0;
  time = 0;
  if(maxSize > this->maxSize) {
    for(int c=0; c<channels; c++) {
      if(inputs[c]) delete inputs[c];
      inputs[c] = new float[maxSize];
    }
  }
  this->maxSize = maxSize;
}

void Granugrain :: setBaseFreq(float pitch, float f0)
{
  this->pitch = pitch;
  this->f0 = f0;
}

void Granugrain :: setPitchbend(float pb)
{
  this->pb = pb;
}

void Granugrain :: writeInputs(float **buf, int n, int offset)
{
	if(size + n <= maxSize) {
    for(int c=0; c<channels; c++) {
      memcpy(inputs[c]+size,buf[c]+offset,n*sizeof(float));
    }
		size += n;
	}
  time += n;
}

Granulator :: Granulator(int c, Granugrain *g) : Gen(c) 
{
  this->g = g;
  granMode = 0;
	step = 1.0;
  state.length = 0;
	state.cursor = 0.0;	
	state.start = 0.0;
	state.nextStart = 0.0;
}

void Granulator :: setParams(GenParams *params)
{
	granMode = params->granMode;
  smooth = params->granSmooth;
  rate = params->granRate;
}

void Granulator :: setFreq(float f)
{
  if(granMode == GranulateKeyLengthTrackLengthPBLength) {
    step = 1;
    nextLength = TWOPI / f ;
  } else if(granMode == GranulateKeyLengthTrackLengthPBStep) {
    step = g->pb;
    nextLength = TWOPI / f  * g->pb;
  } else if(granMode == GranulateKeyLengthTrackStepPBLength) {
    step = f / g->f0 / g->pb;
    nextLength = TWOPI / g->f0  / g->pb;
  } else if(granMode == GranulateKeyLengthTrackStepPBStep) {
    step = f / g->f0;
    nextLength = TWOPI / g->f0 ;
  } else if(granMode == GranulateKeyStepTrackLengthPBLength) {
    step = g->pitch;
    nextLength = TWOPI / f ;
  } else if(granMode == GranulateKeyStepTrackLengthPBStep) {
    step = g->pitch * g->pb;
    nextLength = TWOPI / f  * g->pb;
  }

  if(state.length == 0.0) {
    state.length = nextLength;
    state.crossover = smooth * state.length;			
    nextCrossover = state.crossover;
  }
}

void Granulator :: gen(float dw, float dm, float *in, float *out, int n)
{	
  if(w >= PI) {
    w += n * dw;
    m += n * dm;
    return;
  }

  setFreq(w);
	if(state.start >= g->size) return;

	for(int i=0;i<n;i++) {				
		float envPoint = state.start+state.length;		
		nextCrossover = smooth * nextLength;		
		if(state.cursor <= min(envPoint,state.start+nextLength)) {
			state.crossover = nextCrossover;			
			state.length = nextLength;
			envPoint = state.start+state.length;
		}


    float *buf = g->inputs[c];

		int k = lrintf(state.cursor);
		int k2,l2;
    float a,b;
		float cursor2 = (state.cursor - state.start + state.nextStart - state.length);
		int l = lrintf(cursor2);		
    if(k<=state.cursor) {
      k2 = k + 1;
      a = state.cursor - k;
    } else {
      k2 = k;
      k = k2 - 1;
      a = state.cursor - k;
    }
    if(l<=cursor2) {
      l2 = l + 1;
      b = cursor2 - l;
    } else {
      l2 = l;
      l = l2 - 1;
      b = cursor2 - l;
    }

		float env;
    float o;
		if(l>=0 && state.cursor > envPoint) {
      assert(state.crossover > 0);
			env = 1.0f-(state.cursor-envPoint)/state.crossover;
      if(l2 >= g->size) {        
        if(l >= g->size) {
          o = 0.0f;
        } else {
          o = (1.0f - env) * buf[l];
        }
      } else {
        o = (1.0f - env) * ((1.0f-b)*buf[l] + b*buf[l2]);
      }
    } else {
      env = 1.0f;
      o = 0.0f;
    }

    if(k2 >= g->size) {
      if(k >= g->size) {
      } else {
        o += buf[k];
      }
    } else {
      assert(k >= 0 );
      o += env * ((1.0f-a)*buf[k] + a*buf[k2]);
    }

 		out[i] += m * o; 
		state.cursor += step;			
		if(state.cursor >= envPoint+state.crossover) {
			state.cursor = state.cursor - state.start + state.nextStart - state.length;
			state.start = state.nextStart;				
			if(rate == 1.0f) {
				state.nextStart = g->size - state.length - state.crossover;
      } else {
				state.nextStart = (rate * g->time);
        while(state.nextStart >= g->size) state.nextStart -= g->size;
			}
			if(state.nextStart < 0) state.nextStart = 0;
		}
    w += dw;
    m += dm;
	}
}

DWGS :: DWGS(int c) : Gen(c)
{
	inpos = 1.0/7.0;
	c1 = 8.0;
	c3 = 30.0;
	Z = 1.0;
	Zb = 24000.0;
  zc = 2*Z/(Z+Zb);
  d = new dwgs(Z,Zb,0.0);
  d->set(.4,inpos);
  d->damper(c1,c3);
}

DWGS :: ~DWGS()
{
  delete d;
}

void DWGS :: setParams(GenParams *params)
{
  c1 = params->c1;
  c3 = params->c3;
  inpos = params->inpos;
}

void DWGS :: setFreq(float f)
{
  d->set(f,inpos);			
  d->damper(c1,c3);
}

void DWGS :: gen(float dw, float dm, float *in, float *out, int n)
{
  setFreq(w);
  for(int i=0; i<n; i++) {
     if(w < PIOVER2 && w > 0.001570796326795f) {
      float o = d->go(in[i]);
      out[i] += m * o;
    }
    w += dw;
    m += dm;
  }
}

}
