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

#include <stdlib.h>
#include "me_rjdj_zengarden_ZGObject.h"
#include "zgmessage.h"
#include "ZenGarden.h"

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGObject_remove
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  zg_object_remove((ZGObject *) nativePtr);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGObject_sendMessage
    (JNIEnv *env, jobject jobj, jint inletIndex, jobject jmessage, jlong nativePtr) {
  ZGObject *zgObject = (ZGObject *) nativePtr;
  ZGMessage *zgMessage = zg_message_java_to_zg(env, jmessage);
  zg_object_send_message(zgObject, inletIndex, zgMessage);
  zg_message_delete(zgMessage);
}

JNIEXPORT jobjectArray JNICALL Java_me_rjdj_zengarden_ZGObject_getIncomingConnections
    (JNIEnv *env, jobject jobj, jint inletIndex, jlong nativePtr) {
  ZGObject *zgObject = (ZGObject *) nativePtr;
  unsigned int n = 0;
  ZGConnectionPair *pairs = zg_object_get_connections_at_inlet(zgObject, inletIndex, &n);
  
  // create an array of ConnectionPair objects and fill it
  jclass clazzCP = env->FindClass("me/rjdj/zengarden/ConnectionPair");
  jobjectArray jpairArray = env->NewObjectArray(n, clazzCP, NULL);
  for (unsigned int i = 0; i < n; i++) {
    jobject jpair = env->NewObject(clazzCP, env->GetMethodID(clazzCP, "<init>", "(JI)V"),
        (jlong) pairs[i].object, (jint) pairs[i].letIndex);
    env->SetObjectArrayElement(jpairArray, i, jpair);
  }
      
  free(pairs);
  return jpairArray;
}

JNIEXPORT jobjectArray JNICALL Java_me_rjdj_zengarden_ZGObject_getOutgoingConnections
    (JNIEnv *env, jobject jobj, jint outletIndex, jlong nativePtr) {
  ZGObject *zgObject = (ZGObject *) nativePtr;
  unsigned int n = 0;
  ZGConnectionPair *pairs = zg_object_get_connections_at_outlet(zgObject, outletIndex, &n);

  // create an array of ConnectionPair objects and fill it
  jclass clazzCP = env->FindClass("me/rjdj/zengarden/ConnectionPair");
  jobjectArray jpairArray = env->NewObjectArray(n, clazzCP, NULL);
  for (unsigned int i = 0; i < n; i++) {
    jobject jpair = env->NewObject(clazzCP, env->GetMethodID(clazzCP, "<init>", "(JI)V"),
        (jlong) pairs[i].object, (jint) pairs[i].letIndex);
    env->SetObjectArrayElement(jpairArray, i, jpair);
  }

  free(pairs);
  return jpairArray;
}

JNIEXPORT jstring JNICALL Java_me_rjdj_zengarden_ZGObject_toString
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  // TODO(mhroth): in the future this function should in fact return a full string representation of the object 
  return env->NewStringUTF(zg_object_get_label((ZGObject *) nativePtr));
}
