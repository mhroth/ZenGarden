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

public class ZGGraph {
  
  protected final long graphPtr;
  
  protected ZGGraph(long nativePtr) {
    graphPtr = nativePtr;
  }
  
  /**
   * Attaches this graph to its context.
   */
  void attach() {
    attach(graphPtr);
  }
  private native void attach(long nativePtr);
  
  /**
   * Unattaches this graph from its context.
   */
  void unattach() {
    unattach(graphPtr);
  }
  private native void unattach(long nativePtr);
  
  boolean isAttached() {
    return isAttached(graphPtr);
  }
  private native boolean isAttached(long nativePtr);
  
  /**
   * Create a new object based on the string description, and add it to the graph.
   * @param initString  E.g. "osc~ 440" or "float 1"
   * @return
   */
  ZGObject addObject(String initString) {
    return addObject(graphPtr, initString);
  }
  private native ZGObject addObject(long nativePtr, String initString);
  
  /**
   * Add a connection between two objects.
   * @param fromObject  The object where the connection starts.
   * @param outletIndex  The outlet index at the object from where the connection starts.
   * @param toObject  The object where the connection ends.
   * @param inletIndex  The inlet at the object to which the connection is made.
   */
  void addConnection(ZGObject fromObject, int outletIndex, ZGObject toObject, int inletIndex) {
    if (fromObject == null) {
      throw new NullPointerException("fromObject may not be null.");
    }
    if (outletIndex < 0) {
      throw new IllegalArgumentException("outletIndex must be non-negative: " + outletIndex);
    }
    if (toObject == null) {
      throw new NullPointerException("toObject may not be null.");
    }
    if (inletIndex < 0) {
      throw new IllegalArgumentException("inletIndex must be non-negative: " + inletIndex);
    }
    
    addConnection(fromObject.objectPtr, outletIndex, toObject.objectPtr, inletIndex);
  }
  native private void addConnection(long fromPtr, int outletIndex, long toPtr, int inletIndex);
  
  /**
   * Remove a connection between two objects.
   * @param fromObject  The object where the connection starts.
   * @param outletIndex  The outlet index at the object from where the connection starts.
   * @param toObject  The object where the connection ends.
   * @param inletIndex  The inlet at the object to which the connection is made.
   */
  void removeConnection(ZGObject fromObject, int outletIndex, ZGObject toObject, int inletIndex) {
    if (fromObject == null) {
      throw new IllegalArgumentException("fromObject may not be null.");
    }
    if (outletIndex < 0) {
      throw new IllegalArgumentException("outletIndex must be non-negative: " + outletIndex);
    }
    if (toObject == null) {
      throw new IllegalArgumentException("toObject may not be null.");
    }
    if (inletIndex < 0) {
      throw new IllegalArgumentException("inletIndex must be non-negative: " + inletIndex);
    }
    
    removeConnection(fromObject.objectPtr, outletIndex, toObject.objectPtr, inletIndex);
  }
  native private void removeConnection(long fromPtr, int outletIndex, long toPtr, int inletIndex);

}
