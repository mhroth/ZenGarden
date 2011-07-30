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
  private String typeString;
  
  /**
   * The <code>String</code> representation of a bang.
   */
  public static final String BANG_STRING = "!";
 
  /**
   * Creates a new message. Any number of elements (> 0) may be supplied to the <code>Message</code>
   * either as floats (or <code>Float</code>) or <code>String</code>s. Bangs may also be entered
   * in the form of a <code>String</code>, <code>"!"</code>. For instance, a message with one
   * float element would be instantiated as <code>new Message(0.0, 1.0f)</code>. A message with
   * one string is <code>new Message(0.0, "hello")</code>, and a message with a bang is
   * <code>new Message(0.0, "!")</code>. The form of the bang as a <code>String</code> necessarily
   * means that no strings in the form of "!" may be sent into ZenGarden. Hopefully this will not
   * be a problem.
   * @param timestamp  The time in milliseconds at which this message is created.
   * @param elements  The constituent elements of the message.
   */
  public Message(double timestamp, Object... elements) {
    if (timestamp < 0.0) {
      throw new IllegalArgumentException("A Message may not be created with a negatime timestamp: " +
          Double.toString(timestamp));
    }
    if (elements == null) {
      throw new NullPointerException("The submitted message elements may not be null.");
    }
    if (elements.length == 0) {
      throw new IllegalArgumentException("The submitted message element array must have at least one element.");
    }
    this.timestamp = timestamp;
    this.elements = elements;
    
    // construct the typeString which is used to quickly identify the elements of the message in JNI
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < elements.length; i++) {
      switch (getType(i)) {
        case FLOAT: sb.append("f"); break;
        case SYMBOL: {
          if (getSymbol(i).indexOf(" ") >= 0) {
            throw new IllegalArgumentException("Symbols sent to ZenGarden may not contain spaces: " + 
                getSymbol(i));
          }
          sb.append("s");
          break;
        }
        default: sb.append("b"); break;
      }
    }
    typeString = sb.toString();
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
    return String.class.isInstance(elements[index]) && !BANG_STRING.equals(elements[index]);
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
    // an element is a bang if it is either the special bang string, or it is an unexpected object
    return BANG_STRING.equals(elements[index]) || !(isFloat(index) || isSymbol(index));
  }
  
  @Override
  public String toString() {   
    return "{" + Double.toString(timestamp) + "ms " + Arrays.toString(elements) + "}";
  }
  
  // protected only such that the function can be used in tests
  protected String getTypeString() {
    return typeString;
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
  
  public enum MessageType {
    FLOAT,
    SYMBOL,
    BANG
  }

}
