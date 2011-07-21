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

}
