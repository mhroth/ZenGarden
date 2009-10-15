package me.rjdj.zengarden;

import java.io.File;

/**
 * @author Martin Roth (mhroth@rjdj.me)
 */
public class ZenGarden {
  
  private long nativePtr;

  /**
   * 
   * @param file  The Pd patch to load.
   * @param rjlibDirectory  The location of additional auxiliary patches.
   * @param blockSize  The audio block size which Pd should use.
   * @param numInputChannels  The number of input channels provided by the device's audio system. 
   * Note that in this implementation the native audio system always has two input channels. 
   * @param numOutputChannels  The number of output channels provided by the device's audio system. 
   * Note that in this implementation the native audio system always has two output channels.
   * @param sampleRate  The sample rate at which Pd should run.
   * @throws NativeLoadException  Thrown if the given scene cannot be loaded.
   */
  public ZenGarden(File file, File rjlibDirectory, int blockSize, int numInputChannels, 
      int numOutputChannels, int sampleRate) throws NativeLoadException {
    if (!file.isFile()) {
      throw new IllegalArgumentException("The file object must refer to a file: " + 
          file.toString());
    }
    if (!rjlibDirectory.isDirectory()) {
      throw new IllegalArgumentException("The rjlibDirectory is not a directory: " + 
          rjlibDirectory.toString());
    }
    if (!(numInputChannels == 1 || numInputChannels == 2)) {
      throw new IllegalArgumentException("The number of input channels must be 1 or 2: " + 
          Integer.toString(numInputChannels));
    }
    if (!(numOutputChannels == 1 || numOutputChannels == 2)) {
      throw new IllegalArgumentException("The number of output channels must be 1 or 2: " + 
          Integer.toString(numOutputChannels));
    }
    nativePtr = loadPdPatch(
        file.getParent() + File.separator, file.getName(), 
        rjlibDirectory.toString() + File.separator, blockSize, numInputChannels, numOutputChannels, 
        sampleRate);
  }
  
  static {
    System.loadLibrary("zengarden");
  }
  
  private native long loadPdPatch(
      String directory, String filename, String libraryDirectory, int blockSize, 
      int numInputChannels, int numOutputChannels, int sampleRate) throws NativeLoadException;
  
  @Override
  protected void finalize() throws Throwable {
    try {
      // Automatically unloads the native component if not already done.
      unloadNativeComponentIfStillLoaded();
    } finally {
      super.finalize();
    }
  }
  
  /**
   * Process the input buffer and return the results in the given output buffer. The buffers contain
   * <code>number of channels * block size</code> 16-bit (<code>short</code>) samples.
   * WARNING: For the sake of speed, this method is not synchronized. There is no check to make sure
   * that the native component is still loaded, or that this object represents a valid Pd graph. The
   * developer is responsible for ensuring that the <code>ZenGarden</code> object is still valid
   * when calling <code>process()</code>. Undefined behaviour will result otherwise.
   * @param inputBuffer
   * @param outputBuffer
   */
  public void process(short[] inputBuffer, short[] outputBuffer) {
    process(inputBuffer, outputBuffer, nativePtr);
  }
  
  private native void process(short[] inputBuffer, short[] outputBuffer, long naitvePointer);
  
  /**
   * This method allows the native component to be manually unloaded.
   * This allows memory to be better managed.
   */
  public synchronized void unloadNativeComponentIfStillLoaded() {
    if (isNativeComponentLoaded()) {
      unloadPureData(nativePtr);
      nativePtr = 0;
    }
  }
  
  private native void unloadPureData(long nativePointer);
  
  public synchronized boolean isNativeComponentLoaded() {
    return (nativePtr != 0);
  }
  
}
