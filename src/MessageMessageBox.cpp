#include "MessageElement.h"
#include "MessageMessageBox.h"

MessageMessageBox::MessageMessageBox(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // this constructor is a bit pained, but so be it
  listOfMessageElementLists = new List();
  listofVarableIndicies = new List();
  
  {
    List *initList = MessageElement::toList(initString);
    List *messageElementList = new List(); // WARNING: it is assumed that the first element is not "\\,"
    listOfMessageElementLists->add((void *) messageElementList);
    for (int i = 0; i < initList->getNumElements(); i++) {
      MessageElement *messageElement = (MessageElement *) initList->get(i);
      if (messageElement->getType() == SYMBOL &&
          strcmp(messageElement->getSymbol(), "\\,") == 0) {
        delete messageElement;
        messageElementList = new List();
        listOfMessageElementLists->add((void *) messageElementList);
      } else {
        messageElementList->add(messageElement);
      }
    }
    delete initList;
  }
  
  for (int i = 0; i < listOfMessageElementLists->getNumElements(); i++) {
    List *messageElementList = (List *) listOfMessageElementLists->get(i);
    int *variableIndex = (int *) malloc(messageElementList->getNumElements() * sizeof(int));
    listofVarableIndicies->add(variableIndex);
    for (int i = 0; i < messageElementList->getNumElements(); i++) {
      MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
      if (messageElement->getType() == SYMBOL &&
          strncmp(messageElement->getSymbol(), "\\$", strlen("\\$")) == 0) {
        // -1 to account for zero-based indexing
        variableIndex[i] = atoi(strtok(messageElement->getSymbol(), "\\$")) - 1;
      } else {
        variableIndex[i] = -1;
      }
    }
  }
}

MessageMessageBox::~MessageMessageBox() {
  for (int i = 0; i < listOfMessageElementLists->getNumElements(); i++) {
    List *messageElementList = (List *) listOfMessageElementLists->get(i);
    for (int j = 0; j < messageElementList->getNumElements(); j++) {
      delete (MessageElement *) messageElementList->get(j);
    }
    delete messageElementList;
  }
  delete listOfMessageElementLists;
  
  for (int i = 0; i < listofVarableIndicies->getNumElements(); i++) {
    free(listofVarableIndicies->get(i));
  }
  delete listofVarableIndicies;
}

inline void MessageMessageBox::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    for (int i = 0; i < listOfMessageElementLists->getNumElements(); i++) {
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndex(message->getBlockIndex());
      List *messageElementList = (List *) listOfMessageElementLists->get(i);
      int *variableIndex = (int *) listofVarableIndicies->get(i);
      for (int j = 0; j < messageElementList->getNumElements(); j++) {
        if (variableIndex[j] > -1) {
          MessageElement *messageElement = message->getElement(variableIndex[j]);
          if (messageElement != NULL) {
            MessageElement *outgoingMessageElement = outgoingMessage->getElement(j);
            switch (messageElement->getType()) {
              case FLOAT: {
                outgoingMessageElement->setFloat(messageElement->getFloat());
                break;
              }
              case SYMBOL: {
                outgoingMessageElement->setSymbol(messageElement->getSymbol());
                break;
              }
              case BANG: {
                // or should this be outgoingMessageElement->setFloat(1.0f)?
                outgoingMessageElement->setBang();
                break;
              }
              default: {
                break;
              }
            }
          }
        }
      }
    }
  }
}

/*
 * This is a clever hack (I swear!). processMessage() always calls for messages from
 * getNextOutgoingMessage() in the same order. newCanonicalMessage() keeps track of
 * how many times it has been called and implicitly returns the correct type of message
 * to getNextOutgoingMessage().
 */
PdMessage *MessageMessageBox::newCanonicalMessage() {
  static int messageIndex = 0;
  
  List *messageElementList = (List *) listOfMessageElementLists->get(messageIndex);
  PdMessage *message = new PdMessage();
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    message->addElement(((MessageElement *) messageElementList->get(i))->copy());
  }
  
  if (++messageIndex >= listOfMessageElementLists->getNumElements()) {
    messageIndex = 0;
  }
  
  return message;
}
