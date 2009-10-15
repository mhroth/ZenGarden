package me.rjdj.zengarden;

import java.io.File;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;
import javax.sound.sampled.TargetDataLine;

public class ExampleGarden {
  
  private static final int BLOCK_SIZE = 256;
  private static final int SAMPLE_RATE = 22050;
  private static final int NUM_INPUT_CHANNELS = 1;
  private static final int NUM_OUTPUT_CHANNELS = 2;
  
  public static void main(String[] args) {
    if (args.length == 0) {
      System.out.println("example commandline");
    } else {
      AudioFormat inputAudioFormat = new AudioFormat(SAMPLE_RATE, 16, NUM_OUTPUT_CHANNELS, true, true);
      AudioFormat outputAudioFormat = new AudioFormat(SAMPLE_RATE, 16, NUM_INPUT_CHANNELS, true, true);
      DataLine.Info inputLineInfo = new DataLine.Info(TargetDataLine.class, inputAudioFormat);
      DataLine.Info outputLineInfo = new DataLine.Info(SourceDataLine.class, outputAudioFormat);
      
      short[] inputBuffer = new short[BLOCK_SIZE * NUM_INPUT_CHANNELS];
      short[] outputBuffer = new short[BLOCK_SIZE * NUM_OUTPUT_CHANNELS];
      byte[] bInputBuffer = new byte[inputBuffer.length * 2]; // 2 bytes per sample
      byte[] bOutputBuffer = new byte[outputBuffer.length * 2];
   
      TargetDataLine targetDataLine = null;
      SourceDataLine sourceDataLine = null;
      try {
        // open the audio input (line-in or microphone)
        targetDataLine = (TargetDataLine) AudioSystem.getLine(inputLineInfo);
        targetDataLine.open(inputAudioFormat, bInputBuffer.length);
        targetDataLine.start();
        
        // open the audio output
        sourceDataLine = (SourceDataLine) AudioSystem.getLine(outputLineInfo);
        sourceDataLine.open(outputAudioFormat, bOutputBuffer.length);
        sourceDataLine.start();
      } catch (LineUnavailableException lue) {
        lue.printStackTrace(System.err);
        System.exit(1);
      }
      
      File pdFile = new File(args[0]);
      ZenGarden pdPatch = null;
      try {
        pdPatch = new ZenGarden(pdFile, pdFile.getParentFile(), BLOCK_SIZE, 
            2, NUM_OUTPUT_CHANNELS, SAMPLE_RATE);
      } catch (NativeLoadException nle) {
        nle.printStackTrace(System.err);
        System.exit(2);
      }
      
      while (true) {
        // run the patch in an infinite loop
        targetDataLine.read(bInputBuffer, 0, bInputBuffer.length); // read from the input
        // TODO(mhroth): un-interleave samples
        
        pdPatch.process(inputBuffer, outputBuffer);
        
        // interleave samples
        for (int i = 0, j = BLOCK_SIZE, k = 0; i < BLOCK_SIZE; i++, j++) {
          bOutputBuffer[k++] = (byte) ((outputBuffer[i] & 0xFF00) >> 8);
          bOutputBuffer[k++] = (byte) (outputBuffer[i] & 0x00FF);
          bOutputBuffer[k++] = (byte) ((outputBuffer[j] & 0xFF00) >> 8);
          bOutputBuffer[k++] = (byte) (outputBuffer[j] & 0x00FF);
        }
        // write to the output
        sourceDataLine.write(bOutputBuffer, 0, bOutputBuffer.length);
      }
    }
  }

}
