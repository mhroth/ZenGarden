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

public class ZGContext {
  
  protected final long contextPtr;
  private List<ZenGardenListener> listenerList;
  
  public ZGContext(float sampleRate, int blockSize, int numInputChannels, int numOutputChannels)
      throws NativeLoadException {
    if (sampleRate < 0f) {
      throw new IllegalArgumentException("Sample rate must be positive: " + Float.toString(sampleRate));
    }
    if (sampleRate != 22050.0f || sampleRate != 44100.0f) {
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
    
    listenerList = new ArrayList<ZenGardenListener>();
    
    contextPtr = newContext(sampleRate, blockSize, numInputChannels, numOutputChannels);
  }
  
  private native long newContext(float sampleRate, int blockSize, int numInputChannels, int numOutputChannels)
      throws NativeLoadException;
  
  @Override
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
  public ZGGraph newEmptyGraph() {
    return newEmptyGraph(contextPtr);
  }
  
  private native ZGGraph newEmptyGraph(long nativePtr);
  
  /**
   * Register to receive all messages sent to the given receiver name.
   */
  public void registerReceiver(String receiverName) {
    if (receiverName != null) {
      registerReceiver(contextPtr, receiverName);
    }
  }
  native private void registerReceiver(long nativePtr, String receiverName);
  
  /**
   * Unregister a previously registered receiver.
   */
  public void unregisterReceiver(String receiverName) {
    if (receiverName != null) {
      unregisterReceiver(contextPtr, receiverName);
    }
  }
  native private void unregisterReceiver(long nativePtr, String receiverName);
  
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
    process(inputBuffer, outputBuffer, contextPtr);
  }
  
  private native void process(short[] inputBuffer, short[] outputBuffer, long nativePtr);
  
  /**
   * Add a <code>ZenGardenListener</code> to this graph.
   * @param listener
   */
  public void addListener(ZenGardenListener listener) {
    if (!listenerList.contains(listener)) {
      listenerList.add(listener);
    }
  }
  
  /**
   * Remove a <code>ZenGardenListener</code> from this graph.
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
