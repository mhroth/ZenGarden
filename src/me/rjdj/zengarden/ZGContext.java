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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ZGContext {
  
  protected final long contextPtr;
  protected final int numInputChannels;
  protected final int numOutputChannels;
  protected final int blockSize;
  private List<ZenGardenListener> listenerList;
  
  /**
   * Once the <code>ZGContext</code> object is garbage collected, the native object is also destroyed.
   * @param numInputChannels
   * @param numOutputChannels
   * @param blockSize
   * @param sampleRate
   */
  public ZGContext(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate) {
    if (sampleRate < 0.0f) {
      throw new IllegalArgumentException("Sample rate must be positive: " + Float.toString(sampleRate));
    }
    if (!(sampleRate == 22050.0f || sampleRate == 44100.0f)) {
      System.err.println("It seems that you have chosen a slightly non-standard sample rate (like 22050 or 44100)." +
          "ZenGarden supports you, but you should be sure that you know what you are doing.");
    }
    if (blockSize <= 0) {
      throw new IllegalArgumentException("Block size should be positive: " + Integer.toString(blockSize));
    }
    // TODO(mhroth): check that blocksize is a power of two, and otherwise present a warning.
    if (!(numInputChannels == 1 || numInputChannels == 2)) {
      throw new IllegalArgumentException("The number of input channels must be 1 or 2: " + 
          Integer.toString(numInputChannels));
    }
    if (!(numOutputChannels == 1 || numOutputChannels == 2)) {
      throw new IllegalArgumentException("The number of output channels must be 1 or 2: " + 
          Integer.toString(numOutputChannels));
    }
    
    this.numInputChannels = numInputChannels;
    this.numOutputChannels = numOutputChannels;
    this.blockSize = blockSize;
    
    // instantiate the list of listeners for this context
    listenerList = new ArrayList<ZenGardenListener>();
    
    // create the native context object
    contextPtr = newContext(numInputChannels, numOutputChannels, blockSize, sampleRate);
    if (contextPtr == 0) {
      // TODO(mhroth): what is the correct exception to throw in this case. Something has gone very wrong.
      throw new NullPointerException();
    }
  }
  native private long newContext(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate);
  
  static {
    System.loadLibrary("jnizengarden");
  }
  
  @Override
  // The native context object is destroyed when the Java object is garbage collected.s
  protected void finalize() throws Throwable {
    try {
      deleteContext(contextPtr);
    } finally {
      super.finalize();
    }
  }
  native private void deleteContext(long nativePtr);
  
  /**
   * Create a new empty graph, unattached to the current context.
   */
  public ZGGraph newGraph() {
    return new ZGGraph(newGraph(contextPtr));
  }
  native private long newGraph(long nativePtr);
  
  /**
   * Create a new unattached graph based on the given Pd file.
   */
  public ZGGraph newGraph(File file) {
    if (file == null) {
      throw new NullPointerException("The file cannot be null.");
    }
    if (!file.exists()) {
      throw new IllegalArgumentException("The file does not exist: " + file.toString());
    }
    if (!file.isFile()) {
      throw new IllegalArgumentException("The file object must refer to a file: " + 
          file.toString());
    }
    
    long graphPtr = newGraph(file.getAbsoluteFile().getParent() + File.separator, file.getName(), contextPtr);
    if (graphPtr == 0) {
      // TODO(mhroth): is there a better exception to throw here?
      throw new NullPointerException("Something has done terribly wrong while loading the file " +
          file.getAbsoluteFile());
    }
    return new ZGGraph(graphPtr);
  }
  native private long newGraph(String filePath, String fileName, long nativePtr);
  
  /**
   * Register to receive all messages sent to the given receiver name.
   */
  public void registerReceiver(String receiverName) {
    if (receiverName != null) {
      registerReceiver(receiverName, contextPtr);
    }
  }
  native private void registerReceiver(String receiverName, long nativePtr);
  
  /**
   * Unregister a previously registered receiver.
   */
  public void unregisterReceiver(String receiverName) {
    if (receiverName != null) {
      unregisterReceiver(receiverName, contextPtr);
    }
  }
  native private void unregisterReceiver(String receiverName, long nativePtr);
  
  /**
   * Process the input buffer and return the results in the given output buffer. The buffers contain
   * <code>number of channels * block size</code> 16-bit (<code>short</code>) channel-interleaved 
   * samples.<br>
   * <br>
   * NOTE: The format of the input and output buffers is 16-bit channel-interleaved 
   * (e.g., left, right, left, right, etc.). The reason for this is that Java usually presents
   * audio data in this way (as does Android). <code>libZenGarden</code> processes its buffers
   * as floating-point arrays which are channel-order appended (i.e., left buffer, right buffer). The
   * conversion is done natively, as this is faster.<br>
   * <br>
   * WARNING: For the sake of speed, this method is not synchronized. There is no check to make sure
   * that the native component is still loaded, or that this object represents a valid Pd graph. The
   * developer is responsible for ensuring that the <code>ZenGarden</code> object is still valid
   * when calling <code>process()</code>. Undefined behaviour will result otherwise.
   * @param inputBuffer
   * @param outputBuffer
   */
  public void process(short[] inputBuffer, short[] outputBuffer) {
    process(numInputChannels, inputBuffer, numOutputChannels, outputBuffer, blockSize, contextPtr);
  }
  native private void process(int numInputChannels, short[] inputBuffer, int numOutputChannels,
      short[] outputBuffer, int blockSize, long nativePtr);
  
  /**
   * Send a message to the named receiver. The message will be delivered at the timestamp of the message.
   * If the timestamp is earlier than the current clock of the context, the message will be delivered
   * at the start of the next block. If the receiver does not exist, this function has no effect.
   * @param receiverName
   * @param message
   */
  public void sendMessage(String receiverName, Message message) {
    if (receiverName == null) {
      throw new NullPointerException("Receiver name may not be null.");
    }
    if (message == null) {
      throw new NullPointerException("Message may not be null.");
    }
    sendMessage(receiverName, message, contextPtr);
  }
  native private void sendMessage(String receiverName, Message message, long nativePtr);
  
  @Override
  public boolean equals(Object o) {
    if (ZGContext.class.isInstance(o)) {
      ZGContext zgContext = (ZGContext) o;
      return (contextPtr == zgContext.contextPtr);
    } else {
      return false;
    }
  }
  
  @Override
  public int hashCode() {
    return new Long(contextPtr).hashCode();
  }
  
  /**
   * Add a <code>ZenGardenListener</code> to this context.
   * @param listener
   */
  public void addListener(ZenGardenListener listener) {
    if (!listenerList.contains(listener)) {
      listenerList.add(listener);
    }
  }
  
  /**
   * Remove a <code>ZenGardenListener</code> from this context.
   * @param listener
   */
  public void removeListener(ZenGardenListener listener) {
    listenerList.remove(listener);
  }
  
  private void onPrintStd(String message) {
    for (ZenGardenListener listener : listenerList) {
      listener.onPrintStd(message);
    }
  }
  
  private void onPrintErr(String message) {
    for (ZenGardenListener listener : listenerList) {
      listener.onPrintErr(message);
    }
  }
  
  private void onMessage(String receiverName, Message message) {
    for (ZenGardenListener listener : listenerList) {
      listener.onMessage(receiverName, message);
    }
  }

}
