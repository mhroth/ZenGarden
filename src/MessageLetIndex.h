#ifndef _MESSAGE_INDEX_H_
#define _MESSAGE_INDEX_H_

class MessageOutputObject;

/**
 * Defines a simple struct to combine a DspObject and an inlet/outlet index
 */
typedef struct {
  MessageOutputObject *messageObject;
  int letIndex;
} MessageLetIndex;

#endif // _MESSAGE_INDEX_H_
