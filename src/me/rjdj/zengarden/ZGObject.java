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

package me.rjdj.zengarden;

public class ZGObject {
  
  /**
   * The native pointer to the ZGGraph object.
   */
  final protected long objectPtr;
  
  /**
   * This constructor should only be called by JNI. It represents a convenient way to address
   * objects in ZenGarden. Even if the Java object is garbage collected, the native object remains
   * in memory and in use.
   * @param nativePtr  The native pointer to the ZGGraph object.
   */
  protected ZGObject(long nativePtr) {
    objectPtr = nativePtr;
  }
  
  /**
   * Remove this object from its parent graph. All connections to and from this object are also
   * removed.
   */
  public void remove() {
    remove(objectPtr);
  }
  native private void remove(long nativePtr);
  
  /**
   * Send a {@link Message} to the {@link ZGObject}. The message will be executed at the beginning
   * of the next block <i>before</i> any other previously scheduled messages for that block are
   * executed. The message will be executed at the beginning of the next block regardless of its
   * timestamp. If the message should be executed at some time in the future, or if it should be
   * executed after all of previously scheduled messaged for the next block, use ZGContext's sendMessage,
   * and send the message to a named receiver.
   * @param message  The message which will be sent to the object.
   */
  public void sendMessage(Message message) {
    sendMessage(message, objectPtr);
  }
  native private void sendMessage(Message message, long nativePtr);

}
