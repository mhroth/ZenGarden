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

#include "zgmessage.h"

ZGMessage *zg_message_java_to_zg(JNIEnv *env, jobject jmessage) {
  double timestamp = env->CallDoubleMethod(jmessage,
      env->GetMethodID(env->GetObjectClass(jmessage), "getTimestamp", "()D"));
  jint numElements = env->CallIntMethod(jmessage,
      env->GetMethodID(env->GetObjectClass(jmessage), "getNumElements", "()I"));
  ZGMessage *zgMessage = zg_message_new(timestamp, numElements);
  jstring jtypeString = (jstring) env->CallObjectMethod(jmessage, env->GetMethodID(
      env->GetObjectClass(jmessage), "getTypeString", "()Ljava/lang/String;"));
  const char *ctypeString = env->GetStringUTFChars(jtypeString, NULL);
  for (int i = 0; i < numElements; i++) {
    switch (ctypeString[i]) {
      case 'f': {
        zg_message_set_float(zgMessage, i, env->CallFloatMethod(jmessage, env->GetMethodID(
            env->GetObjectClass(jmessage), "getFloat", "(I)F"), i));
        break;
      }
      case 's': {
        jstring jstr = (jstring) env->CallObjectMethod(jmessage, env->GetMethodID(
            env->GetObjectClass(jmessage), "getSymbol", "(I)Ljava/lang/String;"), i);
        const char *cstr = env->GetStringUTFChars(jstr, NULL);
        zg_message_set_symbol(zgMessage, i, cstr); // the string is copied to the message
        env->ReleaseStringUTFChars(jstr, cstr);
        break;
      }
      case 'b':
      default: {
        zg_message_set_bang(zgMessage, i);
        break;
      }
    }
  }
  env->ReleaseStringUTFChars(jtypeString, ctypeString);
  return zgMessage;
}
