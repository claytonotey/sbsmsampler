#ifndef TRACK_H
#define TRACK_H

#include "buffer.h"
#include "trackpoint.h"
#include "config.h"
#include "utils.h"
#include "gen.h"

#ifdef MULTITHREADED
#include "pthread.h"
#endif
#include <queue>
#include <vector>

extern void blob();

using namespace std;

namespace _sbsms_ {

class Track {
 public:
  Track(int M, int res, float h1, int index);
  virtual ~Track() {}
  virtual TrackPoint *getTrackPoint(countType time)=0;
  virtual Track *getDescendant()=0;
  virtual Track *getPrecursor()=0;
  virtual void popPrecursor()=0;
  inline int getIndex();
  inline float getH();
  inline int getM();
  inline countType getStart();
  inline countType getEnd();
  inline bool isTailStart();
  inline bool isTailEnd();
  inline bool isStart(countType synthtime);
  inline bool isEnd(countType synthtime);
  inline bool isEnded();
  inline countType size();
  bool synth(int c,
             float *out,
             countType synthtime,
             float h2,
             float offset,
             int n,
             float fScale0,
             float fScale1);
  virtual void prune(countType time)=0;
  bool bEnd;

  friend class Gen;
  friend class VoiceSynthesizer;
 protected:
  int M;
  int res;
  float h1;
  float mScale;
  float w00;
  float w11;
  int logM;
  int index;
  char tailEnd;
  char tailStart;
  int rise;
  int fall;
  countType start;
  countType end;
  GenState state;
  GenState stateDescendant;
  Gen *gen;
};

int Track :: getIndex()
{
  return index;
}

float Track :: getH()
{
  return h1;
}

int Track :: getM()
{
  return M;
}

countType Track :: getStart()
{
  return start;
}

countType Track :: getEnd()
{
  return end;
}

bool Track :: isTailStart()
{
  return tailStart?true:false;
}

bool Track :: isTailEnd()
{
  return tailEnd?true:false;
}

bool Track :: isStart(countType synthtime)
{
  return (synthtime == start);
}

bool Track :: isEnd(countType synthtime)
{
  return (synthtime == end);
}

bool Track :: isEnded()
{
  return (end>0);
}

countType Track :: size()
{
  return 1+end-start-tailEnd-tailStart;
}
  
template<class T>
class TrackImp : public Track {
 public:      
  TrackImp(TrackImp<T> *precursor, int M, int res, int h, int index);
  ~TrackImp();
  void push_back(T *p);
  T *back();
  void startTrack(T *p, bool bTail);
  void endTrack(bool bTail);
  TrackPoint *getTrackPoint(countType time);
  Track *getDescendant();
  Track *getPrecursor();
  void popPrecursor();
  void prune(countType time);

  vector<T*> point;
  TrackImp<T> *descendant;
  TrackImp<T> *precursor;
};

template<class T>
TrackImp<T> :: TrackImp(TrackImp<T> *precursor, int M, int res, int h1, int index) 
: Track(M,res,(float)h1,index)
{
  this->descendant = NULL;
  this->precursor = precursor;
}

template<class T>
void TrackImp<T> :: startTrack(T *p, bool bTail)
{
  start = p->time;
  stateDescendant.ph = p->ph;
  if(bTail && start > 0) {
    tailStart = 1;
    start--;
    T *s = new T();
    s->f = p->f;
    s->y = 0.0f;
    s->ph = p->ph;
    s->time = p->time - 1;
    push_back(s);
    this->rise = lrintf((float)M * TWOPI / p->f);
    state.ph = p->ph;
  } else {
    state.ph = p->ph;
  }
  push_back(p);
}

template<class T>
void TrackImp<T> :: endTrack(bool bTail)
{
  T *p = point.back();
  end = p->time;
  if(bTail) {
    this->fall = lrintf((float)M * TWOPI / p->f);
    tailEnd = 1;
    end++;
    T *f = new T();
    f->f = p->f;
    f->y = 0.0f;
    f->ph = p->ph;
    f->time = p->time + 1;
    push_back(f);
  }
}

template<class T>
T *TrackImp<T> :: back() 
{
  return point.back(); 
}

template<class T>
void TrackImp<T> :: prune(countType time)
{
  while(point.size() && point.back()->time > time) {
    if(end > 0 && (point.back()->time == end)) {
      if(tailEnd) delete point.back();
      end = -1;
      tailEnd = 0;
    }
    if(tailStart && (point.back()->time == start)) {
      delete point.back();
    }
    point.pop_back();
  }
}

template<class T>
TrackPoint *TrackImp<T> :: getTrackPoint(countType time)
{
  long k = (long)(time-start);
  if(k<0 || k >= (long)point.size())
    return NULL;
  else
    return point[k];
}

template<class T>
Track *TrackImp<T> :: getPrecursor()
{
  return precursor;
}

template<class T>
void TrackImp<T> :: popPrecursor()
{
  if(precursor) precursor->descendant = NULL;
  precursor = NULL;
}

template<class T>
Track *TrackImp<T> :: getDescendant()
{
  return descendant;
}

template<class T>
void TrackImp<T> :: push_back(T *p)
{
  point.push_back(p);
}

template<>
void TrackImp<TrackPoint2> :: push_back(TrackPoint2 *tp);

template<class T>
TrackImp<T> :: ~TrackImp() {
  if(precursor) precursor->descendant = NULL;
  if(descendant) descendant->precursor = NULL;
  for(typename vector<T*>::iterator i = point.begin();
      i != point.end();
      i++) {
    delete (*i);
  }
  if(gen) delete gen;
}

template<>
TrackImp<TrackPoint1> :: ~TrackImp();

template<class T>
class TrackAllocatorImp {
 public:
  TrackAllocatorImp();
  TrackAllocatorImp(int maxTracks);
  void init();
  int size();
  TrackImp<T> *getTrack(int index);
  TrackImp<T> *create(TrackImp<T> *precursor, int M, int res, int h, int index);  
  TrackImp<T> *create(TrackImp<T> *precursor, int M, int res, int h);
  void destroy(TrackImp<T> *t);

 protected:
#ifdef MULTITHREADED
  pthread_mutex_t taMutex;
#endif
  bool bManageIndex;
  queue<int> gTrackIndex;
  vector<TrackImp<T>*> gTrack;
};

#define SBSMS_TRACK_BLOCK 4096

template<class T>
TrackAllocatorImp<T> :: TrackAllocatorImp()
{
  this->bManageIndex = true;
}

template<class T>
TrackAllocatorImp<T> :: TrackAllocatorImp(int maxTracks)
{
  this->bManageIndex = false;
  gTrack.resize(maxTracks);
  init();
}

template<class T>
void TrackAllocatorImp<T> :: init()
{
  for(int k=0;k<(int)gTrack.size();k++)
    gTrack[k] = NULL;
  while(!gTrackIndex.empty())
    gTrackIndex.pop();
#ifdef MULTITHREADED
  pthread_mutex_init(&taMutex,NULL);
#endif
}

template<class T>
TrackImp<T> *TrackAllocatorImp<T> :: getTrack(int index)
{
  return gTrack[index];
}

template<class T>
int TrackAllocatorImp<T> :: size()
{
  return (int)gTrack.size();
}

template<class T>
TrackImp<T> *TrackAllocatorImp<T> :: create(TrackImp<T> *precursor, int M, int res, int h, int index) 
{
  TrackImp<T> *t = new TrackImp<T>(precursor,M,res,h,index);
#ifdef MULTITHREADED
  pthread_mutex_lock(&taMutex);
#endif
  gTrack[index] = t;
#ifdef MULTITHREADED
  pthread_mutex_unlock(&taMutex);
#endif
  return t;
}

template<class T>
TrackImp<T> *TrackAllocatorImp<T> :: create(TrackImp<T> *precursor, int M, int res, int h) 
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&taMutex);
#endif
  if(gTrackIndex.empty()) {
    int n = (int)gTrack.size();
    gTrack.resize(n+SBSMS_TRACK_BLOCK);
    for(int k=n; k<n+SBSMS_TRACK_BLOCK;k++) {
      gTrackIndex.push(k);
      gTrack[k] = NULL;
    }
  }
  int index = gTrackIndex.front();
  gTrackIndex.pop();
#ifdef MULTITHREADED
  pthread_mutex_unlock(&taMutex);
#endif
  return create(precursor, M, res, h, index);
}

template<class T>
void TrackAllocatorImp<T> :: destroy(TrackImp<T> *t)
{
#ifdef MULTITHREADED
  pthread_mutex_lock(&taMutex);
#endif
  gTrack[t->getIndex()] = NULL;
  if(bManageIndex) gTrackIndex.push(t->getIndex());
  delete t;
#ifdef MULTITHREADED
  pthread_mutex_unlock(&taMutex);
#endif
}

enum TrackType {
  FileTrackType,
  MemoryTrackType,
  AnalysisTrackType
};

class TrackAllocator {
 public:
  TrackAllocator(TrackType type, int maxTracks = 0) {
    this->type = type;
    if(type == FileTrackType) {
      ta0 = new TrackAllocatorImp<TrackPoint>(maxTracks);
    } else if(type == MemoryTrackType) {
      ta1 = new TrackAllocatorImp<TrackPoint1>(maxTracks);
    } else {
      ta2 = new TrackAllocatorImp<TrackPoint2>();
    }
  }

  ~TrackAllocator() {
    if(type == FileTrackType) {
      delete ta0;
    } else if(type == MemoryTrackType) {
      delete ta1;
    } else {
      delete ta2;
    }
  }
  
  void init() {
    if(type == FileTrackType) {
      ta0->init();
    } else if(type == MemoryTrackType) {
      ta1->init();
    } else {
      ta2->init();
    }
  }

  int size() {
    if(type == FileTrackType) {
      return ta0->size();
    } else if(type == MemoryTrackType) {
      return ta1->size();
    } else {
      return ta2->size();
    }
  }

  int getType() {
    return type;
  }

  union {
    TrackAllocatorImp<TrackPoint> *ta0;
    TrackAllocatorImp<TrackPoint1> *ta1;
    TrackAllocatorImp<TrackPoint2> *ta2;
  };

 protected:
  TrackType type;
};

}

#endif
