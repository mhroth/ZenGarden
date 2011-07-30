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

import java.util.ArrayList;
import java.util.List;

public class ZGObject {
  
  /**
   * The native pointer to the ZGObject object.
   */
  final protected long objectPtr;
  
  /**
   * This object represents a convenient way to address
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
  public void sendMessage(int inletIndex, Message message) {
    if (inletIndex < 0) {
      throw new IllegalArgumentException();
    }
    if (message == null) {
      throw new NullPointerException();
    }
    sendMessage(inletIndex, message, objectPtr);
  }
  native private void sendMessage(int inletIndex, Message message, long nativePtr);
  
  /**
   * Returns a <code>List</code> of objects connecting to the given inlet, and from which outlets.
   * @param inletIndex  The inlet index of this object to which other objects connect to.s
   */
  List<ConnectionPair> getIncomingConnections(int inletIndex) {
    ConnectionPair[] pairs = getIncomingConnections(inletIndex, objectPtr);
    ArrayList<ConnectionPair> list = new ArrayList<ConnectionPair>(pairs.length);
    for (ConnectionPair pair : pairs) {
      list.add(pair);
    }
    return list;
  }
  native private ConnectionPair[] getIncomingConnections(int inletIndex, long nativePtr);
  
  /**
   * Returns a <code>List</code> of objects connecting from the given outlet, and from which inlets.
   * @param outletIndex  The outlet index of this object from which other objects connect.
   */
  List<ConnectionPair> getOutgoingConnections(int outletIndex) {
    ConnectionPair[] pairs = getOutgoingConnections(outletIndex, objectPtr);
    ArrayList<ConnectionPair> list = new ArrayList<ConnectionPair>(pairs.length);
    for (ConnectionPair pair : pairs) {
      list.add(pair);
    }
    return list;
  }
  native private ConnectionPair[] getOutgoingConnections(int outletIndex, long nativePtr);
  
  @Override
  public String toString() {
    return toString(objectPtr);
  }
  native private String toString(long nativePtr);
  
  @Override
  public boolean equals(Object o) {
    if (ZGObject.class.isInstance(o)) {
      ZGObject zgObject = (ZGObject) o;
      return (objectPtr == zgObject.objectPtr);
    } else {
      return false;
    }
  }
  
  @Override
  public int hashCode() {
    return new Long(objectPtr).hashCode();
  }

}
