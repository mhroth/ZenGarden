package me.rjdj.zengarden;

import java.io.File;

/**
 * <code>ZenGarden</code> provides a Java interface to <code>libZenGarden</code>. It allows a Pd
 * patch to be loaded, processed, and later unloaded. Garbage collection is handled automatically
 * by the virtual machine, as usual, though it may be better to manually direct the unloading of
 * the native component via <code>unloadNativeComponentIfStillLoaded()</code>. The JVM does not
 * keep track of the memory occupied by native code, which may be substantial, and thus may not
 * garbage collect the Java-side object (which is lightweight) in a timely fashion.
 * 
 * @author Martin Roth (mhroth@rjdj.me)
 */
public class ZenGarden {
  
  private long nativePtr;

  /**
   * 
   * @param patchFile  The patch to load.
   * @param libDirectory  The directory location of additional auxiliary patches (besides the local
   * directory of the patch itself).
   * @param blockSize  The audio block size which Pd should use.
   * @param numInputChannels  The number of input channels provided by the device's audio system. 
   * This number must be 1 or 2. This is an arbitrary limit meant to cover most cases and simplify
   * early development.
   * @param numOutputChannels  The number of output channels provided by the device's audio system.
   * This number must be 1 or 2. This is an arbitrary limit meant to cover most cases and simplify
   * early development.
   * @param sampleRate  The sample rate at which Pd should run.
   * @throws NativeLoadException  Thrown if the given scene cannot be loaded. An explanation is
   * <i>ideally</i> given. Use <code>getMessage()</code>.
   */
  public ZenGarden(File patchFile, File libDirectory, int blockSize, int numInputChannels, 
      int numOutputChannels, int sampleRate) throws NativeLoadException {
    if (!patchFile.isFile()) {
      throw new IllegalArgumentException("The file object must refer to a file: " + 
          patchFile.toString());
    }
    if (!libDirectory.isDirectory()) {
      throw new IllegalArgumentException("The rjlibDirectory is not a directory: " + 
          libDirectory.toString());
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
        patchFile.getAbsoluteFile().getParent() + File.separator, patchFile.getName(), 
        libDirectory.getAbsolutePath() + File.separator, blockSize, numInputChannels, numOutputChannels, 
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
      unloadPdPatch(nativePtr);
      nativePtr = 0;
    }
  }
  
  private native void unloadPdPatch(long nativePointer);
  
  public synchronized boolean isNativeComponentLoaded() {
    return (nativePtr != 0);
  }
  
}
