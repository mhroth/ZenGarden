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

import java.util.Arrays;

public class Message {
  
  private double timestamp;
  private Object[] elements;
 
  /**
   * Creates a new message.
   * @param timestamp  The time in milliseconds at which this message is created.
   * @param elements  The constituent elements of 
   */
  public Message(double timestamp, Object... elements) {
    if (timestamp < 0.0) {
      throw new IllegalArgumentException("A Message may not be created with a negatime timestamp" +
          Double.toString(timestamp));
    }
    if (elements == null) {
      throw new NullPointerException("The submitted message elements cannot be null");
    }
    if (elements.length == 0) {
      throw new IllegalArgumentException("The submitted message element array must have at least one element.");
    }
    this.timestamp = timestamp;
    this.elements = elements;
  }

  double getTimestamp() {
    return timestamp;
  }
  
  int getNumElements() {
    return elements.length;
  }
  
  public MessageType getType(int index) {
    if (isFloat(index)) return MessageType.FLOAT;
    if (isSymbol(index)) return MessageType.SYMBOL;
    return MessageType.BANG;
  }
  
  boolean isFloat(int index) {
    return Float.class.isInstance(elements[index]);
  }
  
  float getFloat(int index) {
    Float f = (Float) elements[index];
    return f.floatValue();
  }
  
  boolean isSymbol(int index) {
    return String.class.isInstance(elements[index]);
  }
  
  boolean isSymbol(int index, String symbol) {
    if (isSymbol(index)) {
      String s = (String) elements[index];
      return s.equals(symbol);
    } else {
      return false;
    }
  }
  
  String getSymbol(int index) {
    return (String) elements[index];
  }
  
  boolean isBang(int index) {
    return Bang.class.isInstance(elements[index]);
  }
  
  @Override
  public String toString() {
    return "{" + Double.toString(timestamp) + "ms " + Arrays.toString(elements) + "}";
  }
  
  @Override
  public boolean equals(Object o) {
    if (Message.class.isInstance(o)) {
      Message message = (Message) o;
      return (timestamp == message.timestamp && Arrays.equals(elements, message.elements));
    } else {
      return false;
    }
  }
  
  @Override
  public int hashCode() {
    return ((new Double(timestamp).hashCode()) + Arrays.hashCode(elements));
  }
  
  public class Bang {
    @Override
    public String toString() {
      return "!bang";
    }
  }
  
  public enum MessageType {
    FLOAT,
    SYMBOL,
    BANG
  }

}
