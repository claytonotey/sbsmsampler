#include "sbsms.h"
#include "grain.h"
#include "synth.h"
#include "track.h"
#include <math.h>
#include "utils.h"
#include "gen.h"
#include "voice.h"
#include "synthTable.h"

extern void blob();

namespace _sbsms_ {

float forceTable[1024] = {
0.144249,
0.160274,
0.171414,
0.180305,
0.187864,
0.194526,
0.20054,
0.206058,
0.211183,
0.215989,
0.220529,
0.224843,
0.228964,
0.232916,
0.23672,
0.240393,
0.243948,
0.247398,
0.250752,
0.254019,
0.257207,
0.260321,
0.263369,
0.266355,
0.269283,
0.272157,
0.274981,
0.277759,
0.280493,
0.283186,
0.28584,
0.288458,
0.291041,
0.293592,
0.296112,
0.298603,
0.301066,
0.303503,
0.305915,
0.308302,
0.310667,
0.313011,
0.315333,
0.317636,
0.31992,
0.322185,
0.324433,
0.326664,
0.328879,
0.331079,
0.333264,
0.335435,
0.337592,
0.339736,
0.341867,
0.343986,
0.346093,
0.348189,
0.350274,
0.352348,
0.354412,
0.356466,
0.358511,
0.360547,
0.362574,
0.364592,
0.366602,
0.368604,
0.370598,
0.372585,
0.374564,
0.376537,
0.378503,
0.380462,
0.382415,
0.384362,
0.386303,
0.388238,
0.390168,
0.392092,
0.394011,
0.395925,
0.397835,
0.399739,
0.401639,
0.403535,
0.405426,
0.407314,
0.409197,
0.411076,
0.412952,
0.414824,
0.416693,
0.418558,
0.42042,
0.42228,
0.424136,
0.425989,
0.427839,
0.429687,
0.431532,
0.433374,
0.435214,
0.437052,
0.438888,
0.440721,
0.442552,
0.444382,
0.446209,
0.448035,
0.449859,
0.451682,
0.453502,
0.455322,
0.45714,
0.458956,
0.460771,
0.462585,
0.464398,
0.46621,
0.468021,
0.469831,
0.471639,
0.473448,
0.475255,
0.477061,
0.478867,
0.480673,
0.482478,
0.484282,
0.486086,
0.487889,
0.489692,
0.491495,
0.493298,
0.4951,
0.496903,
0.498705,
0.500507,
0.502309,
0.504112,
0.505914,
0.507716,
0.509519,
0.511322,
0.513125,
0.514929,
0.516733,
0.518537,
0.520342,
0.522147,
0.523953,
0.52576,
0.527567,
0.529374,
0.531183,
0.532992,
0.534801,
0.536612,
0.538423,
0.540236,
0.542049,
0.543863,
0.545678,
0.547494,
0.549312,
0.55113,
0.552949,
0.55477,
0.556592,
0.558415,
0.560239,
0.562064,
0.563891,
0.565719,
0.567549,
0.569379,
0.571212,
0.573046,
0.574881,
0.576718,
0.578556,
0.580396,
0.582238,
0.584081,
0.585926,
0.587772,
0.589621,
0.591471,
0.593323,
0.595176,
0.597032,
0.598889,
0.600749,
0.60261,
0.604473,
0.606338,
0.608206,
0.610075,
0.611946,
0.61382,
0.615695,
0.617573,
0.619453,
0.621335,
0.623219,
0.625106,
0.626995,
0.628886,
0.630779,
0.632675,
0.634573,
0.636474,
0.638377,
0.640283,
0.642191,
0.644101,
0.646015,
0.64793,
0.649848,
0.651769,
0.653693,
0.655619,
0.657548,
0.659479,
0.661414,
0.663351,
0.665291,
0.667233,
0.669179,
0.671127,
0.673079,
0.675033,
0.67699,
0.67895,
0.680913,
0.682879,
0.684848,
0.686821,
0.688796,
0.690774,
0.692756,
0.69474,
0.696728,
0.698719,
0.700714,
0.702711,
0.704712,
0.706716,
0.708724,
0.710735,
0.712749,
0.714766,
0.716787,
0.718812,
0.72084,
0.722871,
0.724906,
0.726945,
0.728987,
0.731033,
0.733082,
0.735135,
0.737192,
0.739252,
0.741316,
0.743384,
0.745456,
0.747531,
0.749611,
0.751694,
0.753781,
0.755872,
0.757966,
0.760065,
0.762168,
0.764275,
0.766385,
0.7685,
0.770619,
0.772742,
0.774869,
0.777,
0.779136,
0.781275,
0.783419,
0.785567,
0.78772,
0.789876,
0.792037,
0.794203,
0.796372,
0.798546,
0.800725,
0.802908,
0.805096,
0.807288,
0.809484,
0.811685,
0.813891,
0.816102,
0.818317,
0.820536,
0.822761,
0.82499,
0.827224,
0.829462,
0.831706,
0.833954,
0.836207,
0.838465,
0.840728,
0.842996,
0.845269,
0.847547,
0.84983,
0.852118,
0.854411,
0.85671,
0.859013,
0.861321,
0.863635,
0.865954,
0.868278,
0.870608,
0.872943,
0.875283,
0.877628,
0.879979,
0.882336,
0.884698,
0.887065,
0.889438,
0.891817,
0.894201,
0.89659,
0.898986,
0.901387,
0.903793,
0.906206,
0.908624,
0.911048,
0.913478,
0.915913,
0.918355,
0.920803,
0.923256,
0.925716,
0.928181,
0.930653,
0.93313,
0.935614,
0.938104,
0.9406,
0.943103,
0.945611,
0.948126,
0.950647,
0.953175,
0.955709,
0.958249,
0.960796,
0.963349,
0.965909,
0.968476,
0.971049,
0.973628,
0.976215,
0.978808,
0.981407,
0.984014,
0.986627,
0.989247,
0.991874,
0.994508,
0.997149,
0.999797,
1.00245,
1.00511,
1.00778,
1.01046,
1.01314,
1.01583,
1.01853,
1.02124,
1.02395,
1.02667,
1.0294,
1.03213,
1.03487,
1.03762,
1.04038,
1.04315,
1.04592,
1.0487,
1.05149,
1.05428,
1.05709,
1.0599,
1.06272,
1.06555,
1.06838,
1.07122,
1.07408,
1.07694,
1.0798,
1.08268,
1.08556,
1.08846,
1.09136,
1.09427,
1.09718,
1.10011,
1.10304,
1.10599,
1.10894,
1.1119,
1.11487,
1.11785,
1.12083,
1.12383,
1.12683,
1.12985,
1.13287,
1.1359,
1.13894,
1.14199,
1.14505,
1.14812,
1.15119,
1.15428,
1.15738,
1.16048,
1.1636,
1.16672,
1.16986,
1.173,
1.17615,
1.17932,
1.18249,
1.18567,
1.18887,
1.19207,
1.19528,
1.1985,
1.20174,
1.20498,
1.20823,
1.2115,
1.21477,
1.21806,
1.22135,
1.22466,
1.22797,
1.2313,
1.23464,
1.23798,
1.24134,
1.24471,
1.24809,
1.25149,
1.25489,
1.2583,
1.26173,
1.26517,
1.26861,
1.27207,
1.27554,
1.27903,
1.28252,
1.28603,
1.28954,
1.29307,
1.29661,
1.30017,
1.30373,
1.30731,
1.3109,
1.3145,
1.31811,
1.32174,
1.32537,
1.32903,
1.33269,
1.33636,
1.34005,
1.34375,
1.34747,
1.3512,
1.35494,
1.35869,
1.36246,
1.36623,
1.37003,
1.37383,
1.37765,
1.38149,
1.38533,
1.38919,
1.39307,
1.39696,
1.40086,
1.40478,
1.40871,
1.41265,
1.41661,
1.42058,
1.42457,
1.42857,
1.43259,
1.43662,
1.44067,
1.44473,
1.4488,
1.45289,
1.457,
1.46112,
1.46526,
1.46941,
1.47358,
1.47776,
1.48196,
1.48618,
1.49041,
1.49466,
1.49892,
1.5032,
1.5075,
1.51181,
1.51614,
1.52048,
1.52484,
1.52922,
1.53362,
1.53803,
1.54246,
1.54691,
1.55138,
1.55586,
1.56036,
1.56488,
1.56941,
1.57396,
1.57854,
1.58312,
1.58773,
1.59236,
1.597,
1.60167,
1.60635,
1.61105,
1.61577,
1.62051,
1.62527,
1.63005,
1.63484,
1.63966,
1.6445,
1.64935,
1.65423,
1.65913,
1.66404,
1.66898,
1.67394,
1.67892,
1.68392,
1.68894,
1.69398,
1.69904,
1.70412,
1.70923,
1.71436,
1.71951,
1.72468,
1.72987,
1.73509,
1.74032,
1.74558,
1.75087,
1.75617,
1.7615,
1.76685,
1.77223,
1.77763,
1.78305,
1.7885,
1.79397,
1.79946,
1.80498,
1.81053,
1.8161,
1.82169,
1.82731,
1.83295,
1.83862,
1.84431,
1.85003,
1.85578,
1.86155,
1.86735,
1.87317,
1.87903,
1.88491,
1.89081,
1.89674,
1.9027,
1.90869,
1.91471,
1.92075,
1.92682,
1.93292,
1.93905,
1.94521,
1.9514,
1.95762,
1.96386,
1.97014,
1.97644,
1.98278,
1.98915,
1.99554,
2.00197,
2.00843,
2.01492,
2.02144,
2.028,
2.03458,
2.0412,
2.04785,
2.05454,
2.06125,
2.068,
2.07479,
2.0816,
2.08845,
2.09534,
2.10226,
2.10922,
2.11621,
2.12323,
2.1303,
2.13739,
2.14453,
2.1517,
2.15891,
2.16615,
2.17343,
2.18075,
2.18811,
2.19551,
2.20294,
2.21042,
2.21793,
2.22548,
2.23308,
2.24071,
2.24838,
2.2561,
2.26386,
2.27165,
2.27949,
2.28738,
2.2953,
2.30327,
2.31128,
2.31933,
2.32743,
2.33558,
2.34377,
2.352,
2.36028,
2.3686,
2.37698,
2.3854,
2.39386,
2.40237,
2.41094,
2.41955,
2.42821,
2.43691,
2.44567,
2.45448,
2.46334,
2.47225,
2.48121,
2.49022,
2.49929,
2.50841,
2.51758,
2.52681,
2.53609,
2.54542,
2.55481,
2.56426,
2.57376,
2.58332,
2.59294,
2.60262,
2.61235,
2.62214,
2.63199,
2.64191,
2.65188,
2.66191,
2.67201,
2.68217,
2.69239,
2.70267,
2.71302,
2.72344,
2.73392,
2.74446,
2.75508,
2.76575,
2.7765,
2.78732,
2.7982,
2.80916,
2.82019,
2.83128,
2.84245,
2.8537,
2.86501,
2.8764,
2.88787,
2.89941,
2.91103,
2.92272,
2.93449,
2.94635,
2.95828,
2.97029,
2.98238,
2.99456,
3.00682,
3.01916,
3.03159,
3.0441,
3.0567,
3.06939,
3.08216,
3.09503,
3.10799,
3.12103,
3.13417,
3.1474,
3.16073,
3.17415,
3.18767,
3.20128,
3.215,
3.22881,
3.24272,
3.25674,
3.27086,
3.28508,
3.29941,
3.31384,
3.32838,
3.34303,
3.35779,
3.37267,
3.38765,
3.40275,
3.41796,
3.43329,
3.44874,
3.46431,
3.47999,
3.4958,
3.51174,
3.52779,
3.54398,
3.56029,
3.57673,
3.5933,
3.61001,
3.62685,
3.64383,
3.66094,
3.67819,
3.69559,
3.71312,
3.7308,
3.74863,
3.7666,
3.78473,
3.80301,
3.82144,
3.84003,
3.85877,
3.87767,
3.89674,
3.91597,
3.93537,
3.95493,
3.97467,
3.99458,
4.01466,
4.03492,
4.05536,
4.07599,
4.0968,
4.11779,
4.13898,
4.16035,
4.18193,
4.2037,
4.22567,
4.24784,
4.27022,
4.29281,
4.31561,
4.33863,
4.36186,
4.38532,
4.409,
4.43291,
4.45705,
4.48142,
4.50603,
4.53088,
4.55598,
4.58133,
4.60693,
4.63278,
4.65889,
4.68527,
4.71192,
4.73884,
4.76603,
4.79351,
4.82127,
4.84932,
4.87766,
4.90631,
4.93525,
4.96451,
4.99408,
5.02396,
5.05417,
5.08471,
5.11559,
5.1468,
5.17836,
5.21027,
5.24254,
5.27517,
5.30817,
5.34155,
5.37531,
5.40946,
5.444,
5.47895,
5.51431,
5.55008,
5.58628,
5.62291,
5.65998,
5.6975,
5.73548,
5.77392,
5.81283,
5.85223,
5.89211,
5.9325,
5.9734,
6.01481,
6.05676,
6.09925,
6.14228,
6.18588,
6.23005,
6.2748,
6.32015,
6.36611,
6.41269,
6.45989,
6.50775,
6.55626,
6.60544,
6.65532,
6.70589,
6.75718,
6.80919,
6.86196,
6.91549,
6.9698,
7.02491,
7.08083,
7.13758,
7.19519,
7.25367,
7.31304,
7.37332,
7.43453,
7.4967,
7.55985,
7.62399,
7.68917,
7.75539,
7.82268,
7.89108,
7.96061,
8.03129,
8.10316,
8.17625,
8.25059,
8.3262,
8.40313,
8.4814,
8.56106,
8.64215,
8.72468,
8.80872,
8.8943,
8.98146,
9.07025,
9.16072,
9.2529,
9.34686,
9.44263,
9.54029,
9.63987,
9.74145,
9.84507,
9.95081,
10.0587,
10.1689,
10.2814,
10.3962,
10.5136,
10.6335,
10.756,
10.8812,
11.0093,
11.1402,
11.2742,
11.4113,
11.5516,
11.6953,
11.8424,
11.9931,
12.1475,
12.3057,
12.468,
12.6344,
12.8052,
12.9804,
13.1603,
13.3451,
13.5349,
13.73,
13.9306,
14.137,
14.3493,
14.5678,
14.7929,
15.0248,
15.2638,
15.5103,
15.7646,
16.0271,
16.2982,
16.5783,
16.8679,
17.1676,
17.4777,
17.7989,
18.1318,
18.477,
18.8352,
19.2072,
19.5938,
19.9958,
20.4143,
20.8502,
21.3046,
21.7788,
22.274,
22.7918,
23.3336,
23.9013,
24.4966,
25.1217,
25.7788,
26.4705,
27.1996,
27.9692,
28.7828,
29.6442,
30.5579,
31.5286,
32.562,
33.6642,
34.8424,
36.1048,
37.4607,
38.9209,
40.4979,
42.2064,
44.0633,
46.0891,
48.3078,
50.7484,
53.4458,
56.443,
59.7927,
63.5612,
67.8321,
72.7131,
78.3451,
84.9157,
92.6809,
85,
80,
75,
65,
50,
35,
20,
10,
5,
0,
};

SampleSynthesizer :: SampleSynthesizer()
{
  fAM0 = 0.0f;
  fAM1 = 0.0f;
  fAM1Mode = 0;
  mAM0 = 0.0f;
  mAM1 = 0.0f;
  mAM1Mode = 0;
  fFM0 = 0.0f;
  fFM1 = 0.0f;
  fFM1Mode = 0;
  mFM0 = 0.0f;
  mFM1 = 0.0f;
  mFM1Mode = 0;
  dist0 = 0.0f;
  dist1 = 0.0f;
  dist1Mode = 0;
  modPivot = -2.769563268113285f;
  Q0 = 16.0f;
  Q1 = 0.0f;
  Q1Mode = 0;
  sidebandMod = 0.0;
  sidebandEnv = 0.0;
  combFB0 = 0.5;
  combFB1 = 0.0;
  combFB1Mode = 0;
  granMode = 0;
  granRate0 = 0.0f;
  granRate1 = 0.0f;
  granRate1Mode = 0;
  granSmooth0 = 0.5f;
  granSmooth1 = 0.0f;
  granSmooth1Mode = 0;
  dwgsDecay0 = 20.0f;
  dwgsDecay1 = 0.0f;
  dwgsDecay1Mode = 0;
  dwgsLopass0 = 50.0f;
  dwgsLopass1 = 0.0f;
  dwgsLopass1Mode = 0;
  dwgsStringPos0 = 0.25f;
  dwgsStringPos1 = 0.0f;
  dwgsStringPos1Mode = 0;
  mForce = 0.0f;
  mDrag = 0.0f;
  mSpring = 0.0f;
  mThresh = 0.0f;
  synthMode = SynthModeOsc;
}

void SampleSynthesizer :: setSynthMode(int mode) 
{
  pthread_mutex_lock(&synthMutex);
  synthMode = mode;
  pthread_mutex_unlock(&synthMutex);
}

VoiceSynthesizer :: VoiceSynthesizer(SampleSynthesizer *sampleSynth, SBSMSVoice *voice, int spectN) : sampleSynth(sampleSynth), voice(voice)
{
  this->spectN = spectN;
  spectWidth = spectN/16;
  spectScale = 1.0f / (0.1875f * spectN * spectN);
  spectX = new audio[spectN/2+1];
  spectX2 = new audio[spectN/2+1];
  memset(spectX,0,sizeof(spectX));
  synthMode = sampleSynth->synthMode;
}

VoiceSynthesizer :: ~VoiceSynthesizer()
{
  delete spectX;
  delete spectX2;
}

inline float triangle(float p)
{
  if(p > 3.0f) {
    return p - 4.0f; 
  } else if(p > 1.0f) {
    return 2.0f - p;
  } else {
    return p;
  }
}

Particles :: Particles()
{
  n = 0;
}


void Particles :: addParticle(float f, float m)
{
  if(m > 1e-3) {
    this->f.push_back(f);
    this->m.push_back(m);
    //big.insert(pair<float,int>(m,n));
    n++;
  }
}

float Particles :: force(float ww, float mm)
{
  float F = 0.0f;
  for(int i = 0; i<n; i++) {
    if(ww < f[i] ) {
      F += forceTable[ lrintf(ww/f[i] * 1023) ] * m[i];
    } else {
      F -= forceTable[ lrintf(f[i]/ww * 1023) ] * m[i];
    }
  }
  return F;
}

void Particles :: zero()
{
  n = 0;
}

/*
void Particles :: detectFormants()
{
  for(pset::iterator i = big.begin(); i != big.end(); ++i) {
    int k0 = i->first;
    float f0 = i->second;
    float m0 = this->m[k0]; 
    float f = this->f[k0] * 2.0f;
    int k = k0;
    vector<float>::iterator fi = f.begin()+k;
    while(f < PI) {
      vector<float>::iterator gl = lower_bound(fi,f.end(),f);
      f += f0;
    }      
  }
}


void VoiceSynthesizer :: particleFormants(int c)
{
  particles[c].detectFormants();
}
*/
void VoiceSynthesizer :: particleInit(int c)
{
  printf("n = %d\n",particles[c].n);
  particles[c].zero();
}


void VoiceSynthesizer :: particlePopulate(int c, float offset, float pitch, countType synthtime, Track *t)
{
  TrackPoint *tp0 = t->getTrackPoint(synthtime);
  TrackPoint *tp1 = t->getTrackPoint(synthtime+1);

  if(!tp0 || !tp1) return;

  bool bStart = (synthtime == t->start);
  bool bEnd = (synthtime+1 == t->end);

  SampleSynthesizer *s = sampleSynth;

  if(!t->gen || synthMode != s->synthMode) {
    if(t->gen) delete t->gen;
    synthMode = s->synthMode;
    if(synthMode == SynthModeOsc) {
      t->gen = new Osc(c);
    } else if(synthMode == SynthModeFilter) {
      t->gen = new Bandpass(c);
    } else if(synthMode == SynthModeDelay) {
      t->gen = new Delay(c);
    } else if(synthMode == SynthModeDecimate) {
      t->gen = new Decimator(c);
    } else if(synthMode == SynthModeGranulate) {
      t->gen = new Granulator(c,granuGrain);
    } else if(synthMode == SynthModeDWGS) {
      t->gen = new DWGS(c);
    }
  }
  
  if(offset == 0.0f) {
    t->w00 = tp0->f;
    t->w11 = tp1->f;
    
    if(bStart) {
      Track *precursor = t->getPrecursor();
      if(precursor) {
        t->state = precursor->stateDescendant;
      }
    }

    t->w00 *= t->mScale;
    t->w11 *= t->mScale;
  }

  Gen *gen = t->gen;

  float m0 = t->mScale * tp0->y;
  float m1 = t->mScale * tp1->y;

  float w0 = t->w00 * pitch;
  float w1 = t->w11 * pitch;

  float w = w0 + offset * (w1 - w0);
  float m = m0 + offset * (m1 - m0);
  w *= gen->x;

  particles[c].addParticle(w,m);
} 


bool SampleSynthesizer :: isPopulateRequired()
{
  return false;
  return (mForce != 0.0f) || (mSpring != 0.0f) || (mDrag != 0.0f);
}

bool VoiceSynthesizer :: isPopulateRequired()
{
  return sampleSynth->isPopulateRequired();
}

/*
    if((sampleSynth->sampleForceMode == forceAll) || (sampleSynth == s && sampleSynth->sampleForceMode == forceSelf) || (sampleSynth != s && sampleForceMode == forceOther)) {

        if((sampleSynth->voiceForceMode == forceAll) || (sampleSynth == s && sampleSynth->sampleForceMode == forceSelf) || (sampleSynth != s && sampleForceMode == forceOther)) {
*/

float VoiceSynthesizer :: particleForce(int c, float f, float m)
{
  float F = 0.0f;
  ProgramSynthesizer *p = sampleSynth->programSynth;
  for(list<SampleSynthesizer*>::iterator i = p->sampleSynths.begin(); i != 
        p->sampleSynths.end(); ++i) {
    SampleSynthesizer *s = *i;
    for(list<VoiceSynthesizer*>::iterator j = s->voiceSynths.begin(); j != 
          s->voiceSynths.end(); ++j) {
      VoiceSynthesizer *v = *j;
      if(v != this) {
        //if(v->voice->isPlaying()) { 
        if(1) {
          printf("f = %g\n",f);
          F += v->particles[c].force(f,m) * s->mForce;

        }       
      }
    }
  }
          
  return F;
}

void VoiceSynthesizer :: particleStep(int c, int n, Track *t)
{  
  return;
  float dn = n * 1e-3;
  Gen *gen = t->gen;
  float F = particleForce(c, gen->w*gen->x,gen->m);
  if(gen->v < 0) {
    F += gen->v*gen->v*sampleSynth->mDrag;
  } else {
    F -= gen->v*gen->v*sampleSynth->mDrag;
  }
  gen->x *= exp(dn * (gen->v + 0.5f * dn * F));  
  gen->v += dn * F;
}

float VoiceSynthesizer :: getSidebandEnv(float w, int c)
{
  int k = lrintf(w * spectN / TWOPI);
  return sampleSynth->sidebandMod * spectX[k][c];
}

float VoiceSynthesizer :: getSidebandMod(float w, int c)
{
  int k = lrintf(w * spectN / TWOPI);
  return 0.016f * (dBApprox(spectX[k][c] * sampleSynth->sidebandMod) + 29.1185f);
}

float VoiceSynthesizer :: getPivotMod(float w)
{
  return 0;
}

bool VoiceSynthesizer :: synth(int c,
                               float *in,
                              float *out,
                              countType synthtime,
                              float h2,
                              float offset,
                              int n,
                              float fScale0,
                              float fScale1,
                              Track *t)
{
  TrackPoint *tp0 = t->getTrackPoint(synthtime);
  TrackPoint *tp1 = t->getTrackPoint(synthtime+1);

  if(!tp0 || !tp1) return false;

  bool bStart = (synthtime == t->start);
  bool bEnd = (synthtime+1 == t->end);

  GenParams params;
  SampleSynthesizer *s = sampleSynth;
  bool bNewGen = false;

  if(!t->gen || synthMode != s->synthMode) {
    if(t->gen) delete t->gen;
    bNewGen = true;
    synthMode = s->synthMode;
    if(synthMode == SynthModeOsc) {
      t->gen = new Osc(c);
    } else if(synthMode == SynthModeFilter) {
      t->gen = new Bandpass(c);
    } else if(synthMode == SynthModeDelay) {
      t->gen = new Delay(c);
    } else if(synthMode == SynthModeDecimate) {
      t->gen = new Decimator(c);
    } else if(synthMode == SynthModeGranulate) {
      t->gen = new Granulator(c,granuGrain);
    } else if(synthMode == SynthModeDWGS) {
      t->gen = new DWGS(c);
    }
  }
  
  if(offset == 0.0f) {
    t->w00 = tp0->f;
    t->w11 = tp1->f;
    
    if(bStart) {
      Track *precursor = t->getPrecursor();
      if(precursor) {
        t->state = precursor->stateDescendant;
      }
    }
    t->w00 *= t->mScale;
    t->w11 *= t->mScale;
  }

  Gen *gen = t->gen;

  float m0 = t->mScale * tp0->y;
  float m1 = t->mScale * tp1->y;

  bool bThresh;
  if(m0 >= s->mThresh || m1 >= s->mThresh || gen->m >= s->mThresh) {
    bThresh = true;
  } else {
    gen->m = 0.0f;
    bThresh = false;
  }

  float w0 = t->w00 * fScale0 * gen->x;
  float w1 = t->w11 * fScale1 * gen->x;

  float nf = (float)abs(n);

  
  float &pAM = gen->pAM;
  float &pFM = gen->pFM;

  if(bStart) {
    pAM = 1.0f;
    pFM = 0.0f;
  }

  float w01 = 0.5f * (w0 + w1);
  float w2 = (log(w01) - s->modPivot);
  float sm = getSidebandMod(w01,c);  
  
  float dpAM;
  float dpFM;
  if(s->fAM1Mode == 0) {
    dpAM = nf * s->fAM0 * exp(s->fAM1 * w2);
  } else {
    dpAM = nf * s->fAM0 * exp(s->fAM1 * sm);
  }
  if(s->fAM1Mode == 0) {
    dpFM = nf * s->fFM0 * exp(s->fFM1 * w2);
  } else {
    dpFM = nf * s->fFM0 * exp(s->fFM1 * sm);
  }

  float tAM0 = triangle(pAM);
  pAM += dpAM;
  while(pAM > 4.0f) pAM -= 4.0f;
  float tAM1 = triangle(pAM);

  float tFM0 = triangle(pFM);
  pFM += dpFM;
  while(pFM > 4.0f) pFM -= 4.0f;
  float tFM1 = triangle(pFM);

  if(bThresh) {
    float mm;
    if(s->mAM1Mode == 0) {
      mm = max(0.0f,(s->mAM0 * (1.0f + s->mAM1 * w2)));
    } else {
      mm = max(0.0f,(s->mAM0 * (1.0f + s->mAM1 * sm)));
    }
    m0 *= max(0.0f,tAM0 * mm + 1.0f);
    m1 *= max(0.0f,tAM1 * mm + 1.0f);    
    
    float se = (1.0f - s->sidebandEnv) + s->sidebandEnv * getSidebandEnv(w01,c);
    m0 *= se;
    m1 *= se;
    
    float dwFM;
    if(s->mFM1Mode == 0) {
      dwFM = max(-1.0f,min(1.0f, s->mFM1 * w2));
    } else {
      dwFM = max(-1.0f,min(1.0f, s->mFM1 * sm));
    }
    w0 = (w0 + tFM0 * (s->mFM0 + dwFM * w0));
    w1 = (w1 + tFM1 * (s->mFM0 + dwFM * w1));
    
    w0 = min(6.0f,max(0.001f,w0));
    w1 = min(6.0f,max(0.001f,w1));
    
    m0 = min(1.0f,max(0.0f,m0));
    m1 = min(1.0f,max(0.0f,m1));
    
    h2 = fabsf(h2);
    
    gen->state = t->state;
    
    float dm;
    
    //printf("%g %g\n",w01,sm);
    
    int dist;
    if(s->dist1Mode == 0) {
      dist = max(0,min((int)DistMax,(int)lrintf(s->dist0 * (1.0f + min(1.0f, s->dist1 * w2)))));
    } else {
      dist = max(0,min((int)DistMax,(int)lrintf(s->dist0 * (1.0f + min(1.0f, s->dist1 * sm)))));
    }
    params.tab1 = distSynthTable.getDistSynthTable1(dist);
    params.tab2 = distSynthTable.getDistSynthTable2(dist);
    
    if(s->Q1Mode == 0) {
      params.Q = s->Q0 * exp(4.0f * s->Q1 * w2);
    } else {
      params.Q = s->Q0 * exp(4.0f * s->Q1 * sm);
    }
    
    if(s->decBits1Mode == 0) {
      params.bits = min(32.0f,max(4.0f,s->decBits0 * (1.0f + s->decBits1 * w2)));
    } else {
      params.bits = min(32.0f,max(4.0f,s->decBits0 * (1.0f + s->decBits1 * sm)));
    }
    
    if(s->combFB1Mode == 0) {
      float F = 1.0f / (1.0f - s->combFB0);
      F = max(1.0f, F * expf(2.0f * s->combFB1 * w2));
      params.fb = F / (1.0f + F);
    } else {
      float F = 1.0f / (1.0f - s->combFB0);
      F = max(1.0f, F * expf(2.0f * s->combFB1 * sm));
      params.fb = F / (1.0f + F);
    }
    
    params.granMode = s->granMode;
    if(s->granRate1Mode == 0) {
      params.granRate = s->granRate0 * (1.0f + s->granRate1 * w2);
    } else {
      params.granRate = s->granRate0 * (1.0f + s->granRate1 * sm);
    }
    params.granRate = max(0.0f,min(1.0f,params.granRate));
    if(s->granSmooth1Mode == 0) {
      params.granSmooth = s->granSmooth0 * (1.0f + s->granSmooth1 * w2);
    } else {
      params.granSmooth = s->granSmooth0 * (1.0f + s->granSmooth1 * sm);
    }
    params.granSmooth = max(0.0f,min(1.0f,params.granSmooth));
    if(s->dwgsDecay1Mode == 0) {
      params.c1 = s->dwgsDecay0 * exp(2.0f * s->dwgsDecay1 * w2);
    } else {
      params.c1 = s->dwgsDecay0 * exp(2.0f * s->dwgsDecay1 * sm);
    }
    if(s->dwgsLopass1Mode == 0) {
      params.c3 = s->dwgsLopass0 * exp(2.0f * s->dwgsLopass1 * w2);
    } else {
      params.c3 = s->dwgsLopass0 * exp(2.0f * s->dwgsLopass1 * sm);
    }
    if(s->dwgsStringPos1Mode == 0) {
      params.inpos = max(0.0f,min(1.0f,s->dwgsStringPos0 * (1.0f + s->dwgsStringPos1 * w2)));
    } else {
      params.inpos = max(0.0f,min(1.0f,s->dwgsStringPos0 * (1.0f + s->dwgsStringPos1 * sm)));
    }
    
    gen->setParams(&params);
    
    if(gen->m < 0.0f) gen->m = 0.0f;
    //  if(t->index == 74) blob();
    
    //printf("%d %g %g %d %d %d %d\n",n,h2,offset,bStart,t->tailStart,bEnd,t->tailEnd);
    if(bStart && t->tailStart) {
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
        rise = min(max(0,(int)lrintf(t->rise - h2 + h2*offset)),max(-n,iStart));
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
          int rise = min(t->rise,max(n,iStart));
          dm = (m1 - gen->m) / rise;
          iStart = max(0,iStart-rise);
          if(synthMode == SynthModeOsc && offset == 0.0f) {
            gen->state.ph = canon(gen->state.ph - w0 * rise);
          }
        }
        if(iStart < n) {
          out += iStart;
          in += iStart;
          gen->gen(0,dm,in,out,n-iStart);
        }
      }
    } else if(bEnd && t->tailEnd) {
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
          int rise = min(t->fall,max(-n,iStart));
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
          fall = min(max(0,(int)lrintf(t->fall-h2*offset)),max(n,nFrame));
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
      //    if(t->index == 74) blob();
      
      if(n < 0) {
        gen->gen(-dw,-dm,in,out,-n);
      } else {
        gen->gen(dw,dm,in,out,n);
      }   
    } 
  }

  if(bEnd) {
    if(n >= 0 && offset == 0.0f) {
      Track *descendant = t->getDescendant();
      if(descendant && descendant->M < t->M) {
        t->stateDescendant = t->state;
      }
    }
    t->state = gen->state;
  } else if(bStart && t->tailStart) {
    t->state = gen->state;
  } else {
    t->state = gen->state;
    if(n >= 0) {
      Track *descendant = t->getDescendant();
      if(descendant && descendant->M > t->M) {
        t->stateDescendant = t->state;
      }
    }
  }
  return true;
}

void VoiceSynthesizer :: setGrain(Granugrain *g)
{
  this->granuGrain = g;
}

// exp(-6 * x^2) x=[0:511]/512
float spreadTable[512] = {
1,
0.999969,
0.999878,
0.999725,
0.999512,
0.999237,
0.998902,
0.998506,
0.998049,
0.997531,
0.996953,
0.996314,
0.995615,
0.994856,
0.994036,
0.993157,
0.992218,
0.991219,
0.990161,
0.989044,
0.987867,
0.986632,
0.985338,
0.983986,
0.982575,
0.981107,
0.979581,
0.977998,
0.976358,
0.974661,
0.972908,
0.971098,
0.969233,
0.967313,
0.965337,
0.963306,
0.961221,
0.959082,
0.956889,
0.954644,
0.952345,
0.949994,
0.94759,
0.945135,
0.942629,
0.940073,
0.937466,
0.934809,
0.932102,
0.929347,
0.926544,
0.923692,
0.920793,
0.917848,
0.914855,
0.911817,
0.908734,
0.905605,
0.902433,
0.899216,
0.895957,
0.892654,
0.88931,
0.885924,
0.882497,
0.87903,
0.875522,
0.871976,
0.868391,
0.864768,
0.861107,
0.85741,
0.853676,
0.849907,
0.846103,
0.842264,
0.838392,
0.834486,
0.830549,
0.826579,
0.822578,
0.818546,
0.814484,
0.810393,
0.806274,
0.802126,
0.797951,
0.793749,
0.789522,
0.785268,
0.78099,
0.776688,
0.772363,
0.768015,
0.763644,
0.759252,
0.75484,
0.750407,
0.745954,
0.741483,
0.736994,
0.732487,
0.727963,
0.723423,
0.718868,
0.714297,
0.709712,
0.705114,
0.700503,
0.695879,
0.691244,
0.686597,
0.681941,
0.677274,
0.672599,
0.667915,
0.663223,
0.658523,
0.653818,
0.649106,
0.644389,
0.639667,
0.634941,
0.630211,
0.625479,
0.620744,
0.616007,
0.611269,
0.606531,
0.601792,
0.597054,
0.592318,
0.587583,
0.58285,
0.57812,
0.573394,
0.568671,
0.563953,
0.55924,
0.554532,
0.549831,
0.545136,
0.540448,
0.535768,
0.531096,
0.526433,
0.521778,
0.517133,
0.512499,
0.507875,
0.503261,
0.49866,
0.49407,
0.489493,
0.484928,
0.480377,
0.475839,
0.471315,
0.466806,
0.462312,
0.457833,
0.45337,
0.448923,
0.444493,
0.440079,
0.435683,
0.431304,
0.426943,
0.4226,
0.418277,
0.413972,
0.409686,
0.40542,
0.401174,
0.396948,
0.392743,
0.388558,
0.384395,
0.380253,
0.376133,
0.372034,
0.367958,
0.363904,
0.359873,
0.355865,
0.35188,
0.347919,
0.343981,
0.340067,
0.336177,
0.332311,
0.328469,
0.324652,
0.32086,
0.317093,
0.313351,
0.309634,
0.305943,
0.302277,
0.298637,
0.295023,
0.291434,
0.287872,
0.284336,
0.280826,
0.277343,
0.273886,
0.270455,
0.267052,
0.263675,
0.260325,
0.257002,
0.253705,
0.250436,
0.247194,
0.243979,
0.24079,
0.23763,
0.234496,
0.231389,
0.22831,
0.225258,
0.222233,
0.219236,
0.216265,
0.213322,
0.210406,
0.207517,
0.204656,
0.201821,
0.199014,
0.196234,
0.193481,
0.190754,
0.188055,
0.185383,
0.182737,
0.180118,
0.177526,
0.174961,
0.172422,
0.169909,
0.167423,
0.164963,
0.16253,
0.160122,
0.157741,
0.155385,
0.153056,
0.150752,
0.148474,
0.146221,
0.143993,
0.141791,
0.139614,
0.137462,
0.135335,
0.133233,
0.131155,
0.129102,
0.127074,
0.125069,
0.123089,
0.121132,
0.1192,
0.117291,
0.115406,
0.113544,
0.111705,
0.109889,
0.108096,
0.106326,
0.104579,
0.102854,
0.101151,
0.0994707,
0.0978121,
0.0961752,
0.09456,
0.0929662,
0.0913938,
0.0898424,
0.0883119,
0.0868023,
0.0853132,
0.0838446,
0.0823962,
0.0809679,
0.0795595,
0.0781708,
0.0768017,
0.075452,
0.0741214,
0.0728099,
0.0715172,
0.0702432,
0.0689876,
0.0677504,
0.0665313,
0.0653301,
0.0641467,
0.0629809,
0.0618326,
0.0607014,
0.0595873,
0.0584901,
0.0574096,
0.0563456,
0.0552979,
0.0542665,
0.053251,
0.0522513,
0.0512673,
0.0502987,
0.0493454,
0.0484072,
0.047484,
0.0465755,
0.0456816,
0.0448021,
0.0439369,
0.0430858,
0.0422486,
0.0414251,
0.0406152,
0.0398187,
0.0390354,
0.0382652,
0.0375079,
0.0367634,
0.0360314,
0.0353119,
0.0346046,
0.0339094,
0.0332261,
0.0325546,
0.0318948,
0.0312464,
0.0306093,
0.0299834,
0.0293685,
0.0287645,
0.0281711,
0.0275883,
0.0270159,
0.0264538,
0.0259018,
0.0253598,
0.0248276,
0.024305,
0.0237921,
0.0232885,
0.0227942,
0.022309,
0.0218328,
0.0213655,
0.0209069,
0.0204569,
0.0200154,
0.0195822,
0.0191572,
0.0187403,
0.0183313,
0.0179302,
0.0175367,
0.0171509,
0.0167725,
0.0164015,
0.0160377,
0.015681,
0.0153313,
0.0149885,
0.0146525,
0.0143231,
0.0140003,
0.0136839,
0.0133739,
0.01307,
0.0127723,
0.0124807,
0.0121949,
0.0119149,
0.0116407,
0.0113721,
0.011109,
0.0108513,
0.010599,
0.0103519,
0.0101099,
0.00987301,
0.00964107,
0.009414,
0.00919171,
0.00897413,
0.00876116,
0.00855273,
0.00834874,
0.00814912,
0.00795379,
0.00776267,
0.00757568,
0.00739274,
0.00721378,
0.00703872,
0.00686749,
0.00670001,
0.00653622,
0.00637605,
0.00621942,
0.00606627,
0.00591653,
0.00577013,
0.00562702,
0.00548711,
0.00535036,
0.0052167,
0.00508607,
0.00495841,
0.00483365,
0.00471175,
0.00459264,
0.00447627,
0.00436258,
0.00425152,
0.00414303,
0.00403706,
0.00393357,
0.00383249,
0.00373379,
0.0036374,
0.00354329,
0.0034514,
0.00336169,
0.00327411,
0.00318861,
0.00310516,
0.00302371,
0.00294422,
0.00286664,
0.00279094,
0.00271706,
0.00264499,
0.00257466,
0.00250606,
0.00243913,
0.00237385,
0.00231017,
0.00224806,
0.00218749,
0.00212842,
0.00207082,
0.00201466,
0.0019599,
0.00190651,
0.00185446,
0.00180372,
0.00175427,
0.00170607,
0.00165908,
0.0016133,
0.00156868,
0.00152521,
0.00148284,
0.00144157,
0.00140136,
0.00136219,
0.00132403,
0.00128686,
0.00125066,
0.00121541,
0.00118107,
0.00114764,
0.00111508,
0.00108338,
0.00105252,
0.00102248,
0.000993228,
0.000964757,
0.000937046,
0.000910075,
0.000883826,
0.000858282,
0.000833426,
0.00080924,
0.000785708,
0.000762813,
0.000740541,
0.000718875,
0.0006978,
0.000677302,
0.000657365,
0.000637977,
0.000619123,
0.000600789,
0.000582963,
0.00056563,
0.00054878,
0.000532399,
0.000516476,
0.000500998,
0.000485955,
0.000471334,
0.000457126,
0.000443319,
0.000429902,
0.000416866,
0.000404201,
0.000391896,
0.000379943,
0.000368332,
0.000357055,
0.000346101
};

const float spreadTableScale = 85.333333333333333;

void VoiceSynthesizer :: setSideGrain(grain *g, int samples)
{
  float rel = pow(sampleSynth->sidebandRelease,samples);
  memset(spectX2,0,(1+spectN/2)*sizeof(audio));
  for(int c=0; c<2; c++) {
    for(int k=0; k<=spectN/2; k++) {
      float spectk = sampleSynth->sidebandBW * exp(-(sampleSynth->sidebandBWScale * k) / spectN);
      float scale = sqrt(spectk) * spectScale;
      int spectWidth = min(this->spectWidth,(int)lrintf(spectk==0.0?0.0:sqrt(6.0 / spectk)));
      int k2min = max(0,k-spectWidth);
      int k2max = min(spectN/2,k+spectWidth);
      for(int k2 = k2min; k2<k2max; k2++) {
        int kk2 = lrintf((k-k2)*(k-k2)*spectk*spreadTableScale);
        kk2 = min(kk2,511);
        spectX2[k2][c] += scale * g->x[k][c] * spreadTable[kk2];
      }
    }
    for(int k=0; k<=g->N/2; k++) {
      spectX[k][c] = max(spectX[k][c] * rel, spectX2[k][c]);
    }
  }
  /*
  printf("\n");
  for(int k=0; k<=g->N/2; k++) {
    printf("%g %g\n",(float)k/(float)spectN*44100.0,spectX[k][0]);
  }
  */
}

}

