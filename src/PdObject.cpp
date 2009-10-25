#include "PdObject.h"

#include "DspAdd.h"
#include "DspAdc.h"
#include "DspBandpassFilter.h"
#include "DspClip.h"
#include "DspDac.h"
#include "DspDelayRead.h"
#include "DspDelayWrite.h"
#include "DspDivide.h"
#include "DspHighpassFilter.h"
#include "DspInletOutlet.h"
#include "DspMultiply.h"
#include "DspLine.h"
#include "DspLowpassFilter.h"
#include "DspNoise.h"
#include "DspOsc.h"
#include "DspPhasor.h"
#include "DspSendReceive.h"
#include "DspSig.h"
#include "DspSubtract.h"
#include "DspTable.h"
#include "DspTableRead.h"
#include "DspVariableDelay.h"
#include "DspVCF.h"
/*
#include "DspCosine.h"
*/

#include "MessageAbsoluteValue.h"
#include "MessageAdd.h"
#include "MessageArcTangent2.h"
#include "MessageBang.h"
#include "MessageChange.h"
#include "MessageClip.h"
#include "MessageCosine.h"
#include "MessageDbToPow.h"
#include "MessageDbToRms.h"
#include "MessageDelay.h"
#include "MessageDivide.h"
#include "MessageEnv.h"
#include "MessageExp.h"
#include "MessageGreaterThan.h"
#include "MessageInletOutlet.h"
#include "MessageInteger.h"
#include "MessageLessThan.h"
#include "MessageLessThanOrEqualTo.h"
#include "MessageLine.h"
#include "MessageLoadbang.h"
#include "MessageLogicalAnd.h"
#include "MessageLogicalOr.h"
#include "MessageMessageBox.h"
#include "MessageMetro.h"
#include "MessageMidiToFrequency.h"
#include "MessageMoses.h"
#include "MessageMultiply.h"
#include "MessageNotEquals.h"
#include "MessageEqualsEquals.h"
#include "MessageFloat.h"
#include "MessagePack.h"
#include "MessagePow.h"
#include "MessagePowToDb.h"
#include "MessagePrint.h"
#include "MessageRandom.h"
#include "MessageRmsToDb.h"
#include "MessageRoute.h"
#include "MessageSelect.h"
#include "MessageSendReceive.h"
#include "MessageSine.h"
#include "MessageSpigot.h"
#include "MessageSoundfiler.h"
#include "MessageSqrt.h"
#include "MessageSubtract.h"
#include "MessageSwap.h"
#include "MessageSymbol.h"
#include "MessageTangent.h"
#include "MessageTimer.h"
#include "MessageToggle.h"
#include "MessageTrigger.h"
#include "MessageUnpack.h"
#include "MessageGreaterThan.h"
#include "MessageModulus.h"
/*
#include "MessageSnapshot.h"
*/

PdObject::PdObject(char *initString) {
  this->initString = StaticUtils::copyString(initString);
  isMarkedForEvaluation = false;
}

PdObject::~PdObject() {
  free(initString);
}

PdNodeType PdObject::getNodeType() {
  return OBJECT;
}

char *PdObject::getInitString() {
  return initString;
}

PdObject *PdObject::getObjectAtInlet(int inletIndex) {
  return this;
}

PdObject *PdObject::getObjectAtOutlet(int outletIndex) {
  return this;
}

PdObject *PdObject::newInstance(char *objectType, char *objectInitString, int blockSize, int sampleRate, PdGraph *pdGraph) {
  if (strcmp(objectType, "obj") == 0) {
    char *token = strtok(objectInitString, " ");
    if (strcmp(token, "*~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspMultiply(blockSize, objectInitString);
      } else {
        float constant = (float) atof(token);
        return new DspMultiply(constant, blockSize, objectInitString);
      }
    } else if (strcmp(token, "/~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspDivide(blockSize, objectInitString);
      } else {
        float constant = (float) atof(token);
        return new DspDivide(constant, blockSize, objectInitString);
      }
    } else if (strcmp(token, "+~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspAdd(blockSize, objectInitString);
      } else {
        float constant = (float) atof(token);
        return new DspAdd(constant, blockSize, objectInitString);
      }
    } else if (strcmp(token, "-~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspSubtract(blockSize, objectInitString);
      } else {
        float constant = (float) atof(token);
        return new DspSubtract(constant, blockSize, objectInitString);
      }
    } else if (strcmp(token, "adc~") == 0 ||
               strcmp(token, "soundinput") == 0) { // soundinput is from rjlib but doesn't do much for us
      return new DspAdc(blockSize, objectInitString);
    } else if (strcmp(token, "bp~") == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL || nextToken == NULL) {
        return new DspBandpassFilter(blockSize, sampleRate, objectInitString);
      } else {
        float cutoffFrequency = (float) atof(token);
        float q = (float) atof(nextToken);
        return new DspBandpassFilter(cutoffFrequency, q, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp(token, "clip~") == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL) {
        return new DspClip(blockSize, objectInitString);
      } else if (nextToken == NULL) {
        float lowerBound = (float) atof(token);
        return new DspClip(lowerBound, blockSize, objectInitString);
      } else {
        float lowerBound = (float) atof(token);
        float upperBound = (float) atof(nextToken);
        return new DspClip(lowerBound, upperBound, blockSize, objectInitString);
      }
    } else if (strcmp(token, "dac~") == 0 ||
               strcmp(token, "soundoutput") == 0) {
      return new DspDac(blockSize, objectInitString);
    } else if (strcmp("delread~", token) == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL || nextToken == NULL) {
        return NULL; // ERROR!!!
      } else {
        float delayInMs = (float) atof(nextToken);
        return new DspDelayRead(delayInMs, token, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp("delwrite~", token) == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL || nextToken == NULL) {
        return NULL; // ERROR!!!
      } else {
        float delayInMs = (float) atof(nextToken);
        return new DspDelayWrite(delayInMs, token, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp("env~", token) == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageEnv(blockSize, objectInitString);
      } else if (nextToken == NULL) {
        int windowSize = atoi(token);
        return new MessageEnv(windowSize, blockSize, objectInitString);
      } else {
        int windowSize = atoi(token);
        int windowInterval = atoi(nextToken);
        return new MessageEnv(windowSize, windowInterval, blockSize, objectInitString);
      }
    } else if (strcmp(token, "hip~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspHighpassFilter(blockSize, sampleRate, objectInitString);
      } else {
        float cutoffFrequency = (float) atof(token);
        return new DspHighpassFilter(cutoffFrequency, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp(token, "inlet~") == 0 ||
               strcmp(token, "outlet~") == 0) {
      return new DspInletOutlet(blockSize, objectInitString);
    } else if (strcmp(token, "line~") == 0 ||
               strcmp(token, "vline~") == 0) { // TODO(mhroth): WARNING! Implement vline~ correctly!
      return new DspLine(blockSize, sampleRate, objectInitString);
    } else if (strcmp(token, "lop~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspLowpassFilter(blockSize, sampleRate, objectInitString);
      } else {
        float cutoffFrequency = (float) atof(token);
        return new DspLowpassFilter(cutoffFrequency, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp(token, "noise~") == 0) {
      return new DspNoise(blockSize, objectInitString);
    } else if (strcmp(token, "osc~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspOsc(blockSize, sampleRate, objectInitString);
      } else {
        float frequency = (float) atof(token);
        return new DspOsc(frequency, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp(token, "phasor~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspPhasor(blockSize, sampleRate, objectInitString);
      } else {
        float frequency = (float) atof(token);
        return new DspPhasor(frequency, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp(token, "send~") == 0 ||
               strcmp(token, "s~") == 0 ||
               strcmp(token, "receive~") == 0 ||
               strcmp(token, "r~") == 0) {
      token = strtok(NULL, ";");
      return new DspSendReceive(token, blockSize, objectInitString);
    } else if (strcmp(token, "sig~") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspSig(blockSize, objectInitString);
      } else {
        float frequency = (float) atof(token);
        return new DspSig(frequency, blockSize, objectInitString);
      }
    } else if (strcmp("tabread4~", token) == 0) {
      token = strtok(NULL, ";");
      return new DspTableRead(token, blockSize, pdGraph, objectInitString);
    } else if (strcmp("vcf~", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new DspVCF(blockSize, sampleRate, objectInitString);
      } else {
        float q = (float) atof(token);
        return new DspVCF(q, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp("vd~", token) == 0) {
      token = strtok(NULL, " ");
      return new DspVariableDelay(token, blockSize, sampleRate, objectInitString);
    } else if (strcmp("==", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageEqualsEquals(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageEqualsEquals(constant, objectInitString);
      }
    } else if (strcmp("!=", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageNotEquals(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageNotEquals(constant, objectInitString);
      }
    } else if (strcmp(">", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageGreaterThan(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageGreaterThan(constant, objectInitString);
      }
    } else if (strcmp("<", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageLessThan(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageLessThan(constant, objectInitString);
      }
    } else if (strcmp("<=", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageLessThanOrEqualTo(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageLessThanOrEqualTo(constant, objectInitString);
      }
    } else if (strcmp("+", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageAdd(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageAdd(constant, objectInitString);
      }
    } else if (strcmp("-", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageSubtract(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageSubtract(constant, objectInitString);
      }
    } else if (strcmp("*", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageMultiply(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageMultiply(constant, objectInitString);
      }
    } else if (strcmp("/", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageDivide(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageDivide(constant, objectInitString);
      }
    } else if (strcmp("%", token) == 0 ||
               strcmp("mod", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageModulus(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageModulus(constant, objectInitString);
      }
    } else if (strcmp("&&", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageLogicalAnd(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageLogicalAnd(constant, objectInitString);
      }
    } else if (strcmp("||", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageLogicalOr(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageLogicalOr(constant, objectInitString);
      }
    } else if (strcmp("abs", token) == 0) {
      return new MessageAbsoluteValue(objectInitString);
    } else if (strcmp("atan2", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageArcTangent2(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageArcTangent2(constant, objectInitString);
      }
    } else if (strcmp("bang", token) == 0 ||
               strcmp("b", token) == 0 ||
               strcmp("bng", token) == 0) {
      return new MessageBang(objectInitString);
    } else if (strcmp("change", token) == 0) {
      return new MessageChange(objectInitString);
    } else if (strcmp("clip", token) == 0) {
      token = strtok(NULL, " ");
      char *nextToken = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageClip(objectInitString);
      } else if (nextToken == NULL) {
        float lowerBound = (float) atof(token);
        return new MessageClip(lowerBound, objectInitString);
      } else {
        float lowerBound = (float) atof(token);
        float upperBound = (float) atof(nextToken);
        return new MessageClip(lowerBound, upperBound, objectInitString);
      }
    } else if (strcmp("cos", token) == 0) {
      return new MessageCosine(objectInitString);
    } else if (strcmp("dbtorms", token) == 0) {
      return new MessageDbToPow(objectInitString);
    } else if (strcmp("dbtorms", token) == 0) {
      return new MessageDbToRms(objectInitString);
    } else if (strcmp("delay", token) == 0 ||
               strcmp("del", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageDelay(blockSize, sampleRate, objectInitString);
      } else {
        float delayInMs = (float) atof(token);
        return new MessageDelay(delayInMs, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp("exp", token) == 0) {
      return new MessageExp(objectInitString);
    } else if (strcmp("float", token) == 0 ||
               strcmp("f", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageFloat(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageFloat(constant, objectInitString);
      }
    } else if (strcmp("inlet", token) == 0 ||
               strcmp("outlet", token) == 0) {
      return new MessageInletOutlet(objectInitString);
    } else if (strcmp("int", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageInteger(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageInteger(constant, objectInitString);
      }
    } else if (strcmp("line", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageLine(blockSize, sampleRate, objectInitString);
      } else {
        char *nextToken = strtok(NULL, ";");
        if (nextToken == NULL) {
          float initialValue = (float) atof(token);
          return new MessageLine(initialValue, blockSize, sampleRate, objectInitString);
        } else {
          float initialValue = (float) atof(token);
          float grainDelay = (float) atof(nextToken);
          return new MessageLine(initialValue, grainDelay, blockSize, sampleRate, objectInitString);
        }
      }
    } else if (strcmp("loadbang", token) == 0) {
      return new MessageLoadbang(objectInitString);
    } else if (strcmp("metro", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageMetro(blockSize, sampleRate, objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageMetro(constant, blockSize, sampleRate, objectInitString);
      }
    } else if (strcmp("mtof", token) == 0) {
      return new MessageMidiToFrequency(objectInitString);
    } else if (strcmp("moses", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageMoses(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageMoses(constant, objectInitString);
      }
    } else if (strcmp("pack", token) == 0) {
      token = strtok(NULL, ";");
      List *messageElementList = MessageElement::toList(token);
      return new MessagePack(messageElementList, objectInitString);
    } else if (strcmp("pow", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessagePow(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessagePow(constant, objectInitString);
      }
    } else if (strcmp("powtodb", token) == 0) {
      return new MessagePowToDb(objectInitString);
    } else if (strcmp("print", token) == 0) {
      return new MessagePrint(objectInitString);
    } else if (strcmp("random", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageRandom(objectInitString);
      } else {
        int N = atoi(token);
        return new MessageRandom(N, objectInitString);
      }
    } else if (strcmp("rmstodb", token) == 0) {
      return new MessageRmsToDb(objectInitString);
    } else if (strcmp("route", token) == 0) {
      token = strtok(NULL, ";");
      List *routeList = MessageElement::toList(token);
      return new MessageRoute(routeList, objectInitString);
    } else if (strcmp("select", token) == 0 ||
               strcmp("sel", token) == 0) {
      token = strtok(NULL, ";"); // select string
      List *messageElementList = MessageElement::toList(token);
      return new MessageSelect(messageElementList, objectInitString);
    } else if (strcmp("send", token) == 0 ||
               strcmp("s", token) == 0 ||
               strcmp("receive", token) == 0  ||
               strcmp("r", token) == 0 ) {
      token = strtok(NULL, " ");
      return new MessageSendReceive(token, objectInitString);
    } else if (strcmp("sin", token) == 0) {
      return new MessageSine(objectInitString);
    } else if (strcmp("soundfiler", token) == 0) {
      return new MessageSoundfiler(pdGraph, objectInitString);
    } else if (strcmp("spigot", token) == 0) {
      token = strtok(NULL, ";");
      if (token == NULL) {
        return new MessageSpigot(objectInitString);
      } else {
        float initialValue = (float) atof(token);
        return new MessageSpigot(initialValue, objectInitString);
      }
    } else if (strcmp("sqrt", token) == 0) {
      return new MessageSqrt(objectInitString);
    } else if (strcmp("swap", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageSwap(objectInitString);
      } else {
        float constant = (float) atof(token);
        return new MessageSwap(constant, objectInitString);
      }
    } else if (strcmp("symbol", token) == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        return new MessageSymbol(objectInitString);
      } else {
        // TODO(mhroth): should this be a MessageMessageBox or a MessageSymbol?
        // What is the difference?
        return new MessageSymbol(token, objectInitString);
      }
    } else if (strcmp("table", token) == 0) {
      token = strtok(NULL, " ");
      if (token != NULL) {
        char *nextToken = strtok(NULL, ";");
        if (nextToken != NULL) {
          int bufferLengthInSamples = atoi(nextToken);
          return new DspTable(bufferLengthInSamples, token, blockSize, objectInitString);
        } else {
          return new DspTable(token, blockSize, objectInitString);
        }
      } else {
        printf("WARNING: Table initialisation string not recognised. Missing parameters: %s\n", objectInitString);
        return NULL; // ERROR
      }
    } else if (strcmp("tan", token) == 0) {
      return new MessageTangent(objectInitString);
    } else if (strcmp("timer", token) == 0) {
      return new MessageTimer(blockSize, sampleRate, objectInitString);
    } else if (strcmp("toggle", token) == 0 ||
               strcmp("tgl", token) == 0) {
      return new MessageToggle(objectInitString);
    } else if (strcmp("trigger", token) == 0 ||
               strcmp("t", token) == 0) {
      token = strtok(NULL, ";");
      // TODO(mhroth): just pass the messageElementList and let the MessageTrigger
      // deal with storing it and deleting it
      List *messageElementList = MessageElement::toList(token);
      PdObject *pdObject = new MessageTrigger(messageElementList, objectInitString);
      for (int i = 0; i < messageElementList->getNumElements(); i++) {
        delete (MessageElement *) messageElementList->get(i);
      }
      delete messageElementList;
      return pdObject;
    } else if (strcmp("unpack", token) == 0) {
      token = strtok(NULL, ";");
      List *messageElementList = MessageElement::toList(token);
      return new MessageUnpack(messageElementList, objectInitString);
    } else if (strcmp("vsl", token) == 0) {
      // a vslider is just a float box with a graphical element
      // (which is not represented here)
      return new MessageFloat(objectInitString);
    } else {
      return NULL;
    }
  } else if (strcmp(objectType, "msg") == 0) {
    return new MessageMessageBox(objectInitString);
  } else {
    printf("WARNING: Object not recognised: %s\n", objectInitString);
    return NULL;
  }
}
