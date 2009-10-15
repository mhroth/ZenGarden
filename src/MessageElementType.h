#ifndef _MESSAGE_ELEMENT_TYPE_H_
#define _MESSAGE_ELEMENT_TYPE_H_

enum MessageElementType {
  FLOAT,
  BANG,
  SYMBOL,
  ANYTHING, // these last two types are used exlusively for trigger
  LIST      // which can cast messages to the former types
};

#endif // _MESSAGE_ELEMENT_TYPE_H_
