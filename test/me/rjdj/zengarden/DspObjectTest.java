/*
 *  Copyright 2012 Reality Jockey, Ltd.
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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;

/**
 * This class is a test suite for all dsp objects.
 * 
 * @author Martin Roth (mhroth@gmail.com)
 */
public class DspObjectTest implements ZenGardenListener {
  
  private static final int BLOCK_SIZE = 64;
  private static final int NUM_INPUT_CHANNELS = 0;
  private static final int NUM_OUTPUT_CHANNELS = 1;
  private static final float SAMPLE_RATE = 44100.0f;
  private static final short[] INPUT_BUFFER = new short[BLOCK_SIZE * NUM_INPUT_CHANNELS];
  private static final short[] OUTPUT_BUFFER = new short[BLOCK_SIZE * NUM_OUTPUT_CHANNELS];
  private static final String TEST_PATHNAME = "./test/dsp";
  
  private AudioInputStream ais;
  
  @Before
  public void setUp() throws Exception {
    ais = null; // ensure that the audio input stream is clear before beginning a test
  }

  @After
  public void tearDown() throws Exception {
    if (ais != null) ais.close(); // no matter what, be sure to close the audio input stream
  }

  @Test
  public void testDspOsc() {
    genericDspTest("DspOsc.pd");
  }
  
  /**
   * Encompasses a generic test for audio objects. It processes the graph for one second once and
   * compares the standard output to the golden file.
   * @param testFilename
   */
  private void genericDspTest(String testFilename) {
    genericDspTest(testFilename, 1000.0f);
  }
  
  /**
   * Executes the generic message test for at least the given minimum runtime (in milliseconds).
   */
  private void genericDspTest(String testFilename, float minmumRuntimeMs) {
    ZGContext context = new ZGContext(NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, BLOCK_SIZE, SAMPLE_RATE);
    context.addListener(this);
    ZGGraph graph = context.newGraph(new File(TEST_PATHNAME, testFilename));
    graph.attach();
    
    // process at least as many blocks as necessary to cover the given runtime
    int numBlocksToProcess = (int) (Math.floor(((minmumRuntimeMs/1000.0f)*SAMPLE_RATE)/BLOCK_SIZE)+1);
    
    try {
      ais = AudioSystem.getAudioInputStream(new File(TEST_PATHNAME,
          testFilename.split("\\.")[0] + ".golden.wav"));
    } catch (Exception e) {
      fail(e.toString());
    }
    
    assertEquals("The golden file does not have same length as the audio that will be produced.",
        numBlocksToProcess*BLOCK_SIZE, ais.getFrameLength());
    
    byte[] buffer = new byte[2*BLOCK_SIZE];
    short[] goldenBuffer = new short[BLOCK_SIZE];
    
    for (int i = 0; i < numBlocksToProcess; i++) {
      // process the context and fill the output buffer
      context.process(INPUT_BUFFER, OUTPUT_BUFFER);
      
      // read the next part of the golden buffer
      try {
        ais.read(buffer);
      } catch (IOException e) {
        fail(e.toString());
      }
      
      // convert the read bytes to a short array (assume little endian formatting)
      for (int j = 0; j < goldenBuffer.length; j++) {
        goldenBuffer[j] = (short) (((buffer[(2*j)+1] & 0x000000FF) << 8) | (buffer[2*j] & 0x000000FF));
      }
      
      // ensure that the output and expected buffers are the same
      float blockTimeSec = numBlocksToProcess*BLOCK_SIZE/SAMPLE_RATE;
      assertEquals("Output not equal to golden file at time " + blockTimeSec + "s.",
          goldenBuffer, OUTPUT_BUFFER);
    }
  }
 
  public void onPrintStd(String message) {
    // TODO Auto-generated method stub
  }

  public void onPrintErr(String message) {
    // TODO Auto-generated method stub
  }

  public void onMessage(String receiverName, Message message) {
    // TODO Auto-generated method stub
  }

}
