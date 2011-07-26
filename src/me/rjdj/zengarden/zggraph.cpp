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

#include "me_rjdj_zengarden_ZGGraph.h"
#include "ZenGarden.h"

JNIEXPORT jlong JNICALL Java_me_rjdj_zengarden_ZGGraph_addObject
    (JNIEnv *env, jobject jobj, jstring jinitString, jint canvasX, jint canvasY, jlong nativePtr) {
  ZGGraph *zgGraph = (ZGGraph *) nativePtr;
  const char *cinitString = env->GetStringUTFChars(jinitString, NULL);
  ZGObject *zgObject = zg_new_object(zgGraph, (char *) cinitString);
  env->ReleaseStringUTFChars(jinitString, cinitString);
  if (zgObject != NULL) {
    zg_add_object(zgGraph, zgObject, canvasX, canvasY);
    return (jlong) zgObject;
  } else {
    return (jlong) 0;
  }
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGGraph_addConnection
    (JNIEnv *env, jobject jobj, jlong fromObjectPtr, jint outletIndex, jlong toObjectPtr, jint inletIndex, jlong graphPtr) {
  zg_add_connection((ZGGraph *) graphPtr, (ZGObject *) fromObjectPtr, outletIndex, (ZGObject *) toObjectPtr, inletIndex);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGGraph_removeConnection
    (JNIEnv *env, jobject jobj, jlong fromObjectPtr, jint outletIndex, jlong toObjectPtr, jint inletIndex, jlong graphPtr) {
  zg_remove_connection((ZGGraph *) graphPtr, (ZGObject *) fromObjectPtr, outletIndex, (ZGObject *) toObjectPtr, inletIndex);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGGraph_attach
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  zg_graph_attach((ZGGraph *) nativePtr);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGGraph_unattach
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  zg_graph_unattach((ZGGraph *) nativePtr);
}
