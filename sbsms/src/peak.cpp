#include "peak.h"
#include <stdlib.h>
#include <string.h>

namespace _sbsms_ {

PeakAllocator :: PeakAllocator()
{
  size = 0;
  n = 0;
  peaks = NULL;
}

PeakAllocator :: ~PeakAllocator()
{
  if(peaks) free(peaks);
}

peak *PeakAllocator :: create() 
{
  if(n >= size) {
    if(size == 0)
      size = 1024;
    else
      size *= 2;
    
    peak *newpeaks = (peak*)malloc(size*sizeof(peak));
    if(peaks) {
      memcpy(newpeaks,peaks,n*sizeof(peak));
      free(peaks);
    }
    peaks = newpeaks;
  }
  return peaks+(n++);
}

void PeakAllocator :: destroyAll()
{
  n = 0;
}

}
