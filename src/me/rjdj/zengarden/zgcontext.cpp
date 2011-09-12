/*
 *  Copyright 2011 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "me_rjdj_zengarden_ZGContext.h"
#include "ZenGarden.h"
#include "zgmessage.h"

#define JNI_VERSION JNI_VERSION_1_4

JavaVM *zg_jvm = NULL; // global variable

typedef struct {
  jobject zgObject;
  PdContext *context;
  float *finputBuffer;
  float *foutputBuffer;
  int blockSize;
  int numInputChannels; // the number of input channels supplied by the system
  int numOutputChannels; // the number of input channels supplied by the system
  float *shortToFloatLookupTable;
  int numBytesInBlock;
} PureDataMobileNativeVars;

extern "C" {
  void *zg_callback(ZGCallbackFunction function, void *userData, void *ptr) {
    JNIEnv *env = NULL;
    jint result = zg_jvm->GetEnv((void **)&env, JNI_VERSION);
    if (result == JNI_OK && env != NULL) {
      jobject zgContext = (jobject) userData;
      switch (function) {
        case ZG_PRINT_STD: {
          env->CallVoidMethod(zgContext,
              env->GetMethodID(env->GetObjectClass(zgContext),
                  "onPrintStd", "(Ljava/lang/String;)V"),
              env->NewStringUTF((char *) ptr));
          break;
        }
        case ZG_PRINT_ERR: {
          env->CallVoidMethod(zgContext,
              env->GetMethodID(env->GetObjectClass(zgContext),
                  "onPrintErr", "(Ljava/lang/String;)V"),
              env->NewStringUTF((char *) ptr));
          break;
        }
        case ZG_RECEIVER_MESSAGE: {
          ZGReceiverMessagePair *rmPair = (ZGReceiverMessagePair *) ptr;

          // create a new Message
          ZGMessage *zgMessage = rmPair->message;
          int numElements = zg_message_get_num_elements(zgMessage);
          jobjectArray jelements = env->NewObjectArray(numElements,
              env->FindClass("java/lang/Object"), NULL);

          for (int i = 0; i < numElements; i++) {
            switch (zg_message_get_element_type(zgMessage, i)) {
              case ZG_MESSAGE_ELEMENT_FLOAT: {
                jclass clazz = env->FindClass("java/lang/Float");
                jobject jfloatObject = env->NewObject(clazz,
                    env->GetMethodID(clazz, "<init>", "(F)V"),
                    zg_message_get_float(zgMessage, i));
                env->SetObjectArrayElement(jelements, i, jfloatObject);
                break;
              }
              case ZG_MESSAGE_ELEMENT_SYMBOL: {
                env->SetObjectArrayElement(jelements, i, env->NewStringUTF(zg_message_get_symbol(zgMessage, i)));
                break;
              }
              case ZG_MESSAGE_ELEMENT_BANG:
              default: {
                env->SetObjectArrayElement(jelements, i, env->NewStringUTF("!"));
                break;
              }
            }
          }

          jclass clazz = env->FindClass("me/rjdj/zengarden/Message");
          jobject jmessage = env->NewObject(clazz, env->GetMethodID(clazz, "<init>", "(D[Ljava/lang/Object;)V"),
              zg_message_get_timestamp(zgMessage), jelements);

          env->CallVoidMethod(zgContext,
              env->GetMethodID(env->GetObjectClass(zgContext),
                  "onMessage", "(Ljava/lang/String;Lme/rjdj/zengarden/Message;)V"),
              env->NewStringUTF(rmPair->receiverName), jmessage);
          break;
        }
        default: {
          break;
        }
      }
    }
    return NULL;
  }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *ur_jvm, void *reserved) {
  // cache the java vm pointer
  // used for getting a pointer to the java environment during callbacks
  zg_jvm = ur_jvm;
  
  return JNI_VERSION;
}

JNIEXPORT jlong JNICALL Java_me_rjdj_zengarden_ZGContext_newContext
    (JNIEnv *env, jobject jobj, jint numInputChannels, jint numOutputChannels, jint blockSize, jfloat sampleRate) {
  return (jlong) zg_context_new(numInputChannels, numOutputChannels, blockSize, sampleRate, zg_callback, env->NewWeakGlobalRef(jobj));
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGContext_deleteContext
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  // NOTE(mhroth): it isn't absolutely necessary to delete the weak reference to the object, as this
  // function is only called when the object is being garbage collected anyway. But it is good
  // policy to clean up after oneself and release the VM resources needed to maintain the weak reference.
  ZGContext *context = (ZGContext *) nativePtr;
  env->DeleteWeakGlobalRef((jweak) zg_context_get_userinfo(context));
  zg_context_delete(context);
}

JNIEXPORT jlong JNICALL Java_me_rjdj_zengarden_ZGContext_newGraph__J
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  return (jlong) zg_context_new_empty_graph((ZGContext *) nativePtr);
}

JNIEXPORT jlong JNICALL Java_me_rjdj_zengarden_ZGContext_newGraph__Ljava_lang_String_2Ljava_lang_String_2J
    (JNIEnv *env, jobject jobj, jstring jdirectory, jstring jfilename, jlong nativePtr) {
  char *cdirectory = (char *) env->GetStringUTFChars(jdirectory, NULL);
  char *cfilename = (char *) env->GetStringUTFChars(jfilename, NULL);
  ZGGraph *zgGraph = zg_context_new_graph_from_file((ZGContext *) nativePtr, cdirectory, cfilename);
  env->ReleaseStringUTFChars(jdirectory, cdirectory);
  env->ReleaseStringUTFChars(jfilename, cfilename);
  return (jlong) zgGraph;
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGContext_registerReceiver
    (JNIEnv *env, jobject jobj, jstring jreceiverName, jlong nativePtr) {
  const char *creceiverName = env->GetStringUTFChars(jreceiverName, NULL);
  zg_context_register_receiver((ZGContext *) nativePtr, creceiverName);
  env->ReleaseStringUTFChars(jreceiverName, creceiverName);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGContext_unregisterReceiver
    (JNIEnv *env, jobject jobj, jstring jreceiverName, jlong nativePtr) {
  const char *creceiverName = env->GetStringUTFChars(jreceiverName, NULL);
  zg_context_unregister_receiver((ZGContext *) nativePtr, creceiverName);
  env->ReleaseStringUTFChars(jreceiverName, creceiverName);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGContext_sendMessage
    (JNIEnv *env, jobject jobj, jstring jreceiverName, jobject jmessage, jlong nativePtr) {
  const char *creceiverName = env->GetStringUTFChars(jreceiverName, NULL);
  ZGMessage *zgMessage = zg_message_java_to_zg(env, jmessage);
  zg_context_send_message((ZGContext *) nativePtr, creceiverName, zgMessage);
  env->ReleaseStringUTFChars(jreceiverName, creceiverName);
  zg_message_delete(zgMessage);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGContext_process
    (JNIEnv *env, jobject jobj, jint numInputChannels, jshortArray jinputBuffer, jint numOutputChannels,
    jshortArray joutputBuffer, jint blockSize, jlong nativePtr) {

  short *cinputBuffer = (short *) env->GetPrimitiveArrayCritical(jinputBuffer, NULL);
  short *coutputBuffer = (short *) env->GetPrimitiveArrayCritical(joutputBuffer, NULL);

  zg_context_process_s((ZGContext *) nativePtr, cinputBuffer, coutputBuffer);
      
  // no need to copy back changes. release native buffer.
  env->ReleasePrimitiveArrayCritical(jinputBuffer, cinputBuffer, JNI_ABORT);
  env->ReleasePrimitiveArrayCritical(joutputBuffer, coutputBuffer, JNI_ABORT);
}
