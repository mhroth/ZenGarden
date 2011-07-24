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

#include "me_rjdj_zengarden_ZGObject.h"
#include "ZenGarden.h"

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGObject_remove
    (JNIEnv *env, jobject jobj, jlong nativePtr) {
  zg_object_remove((ZGObject *) nativePtr);
}

JNIEXPORT void JNICALL Java_me_rjdj_zengarden_ZGObject_sendMessage
    (JNIEnv *env, jobject jobj, jint inletIndex, jobject jmessage, jlong nativePtr) {
  ZGObject *zgObject = (ZGObject *) nativePtr;
  ZGMessage *zgMessage = NULL; // TODO(mhroth): convert the java message into a ZG one
  zg_object_send_message(zgObject, inletIndex, zgMessage);
}
