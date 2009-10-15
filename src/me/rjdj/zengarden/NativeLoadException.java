package me.rjdj.zengarden;

/**
 * A <code>NativeLoadException</code> is thrown when a native component cannot be loaded.
 * 
 * @author Martin Roth (mhroth@rjdj.me)
 */
public class NativeLoadException extends Exception {

  private static final long serialVersionUID = 1L;
  
  public NativeLoadException() {
    super();
  }
  
  public NativeLoadException(String message) {
    super(message);
  }

}
