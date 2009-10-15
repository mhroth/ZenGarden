#ifndef _DSP_INDEX_H_
#define _DSP_INDEX_H_

class DspOutputObject;

/**
 * Defines a simple struct to combine a DspObject and an inlet/outlet index
 */
typedef struct {
  DspOutputObject *dspObject;
  int letIndex;
} DspLetIndex;

#endif // _DSP_INDEX_H_
