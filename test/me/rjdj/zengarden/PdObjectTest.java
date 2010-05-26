/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

/**
 * This class is a test suite for all implemented objects.
 * 
 * @author Martin Roth (mhroth@rjdj.me)
 */
public class PdObjectTest implements ZenGardenListener {
  
  private static final int BLOCK_SIZE = 64;
  private static final int NUM_INPUT_CHANNELS = 2;
  private static final int NUM_OUTPUT_CHANNELS = 2;
  private static final float SAMPLE_RATE = 44100.0f;
  private static final short[] INPUT_BUFFER = new short[BLOCK_SIZE * NUM_INPUT_CHANNELS];
  private static final short[] OUTPUT_BUFFER = new short[BLOCK_SIZE * NUM_OUTPUT_CHANNELS];
  private static final String TEST_PATHNAME = "./test";
  
  private StringBuilder stringBuilderStd;
  private StringBuilder stringBuilderErr;

  @Before
  public void setUp() throws Exception {
    stringBuilderStd = new StringBuilder();
    stringBuilderErr = new StringBuilder();
  }

  @After
  public void tearDown() throws Exception {
    // nothing to do
  }
  
  @Test
  public void testMessageEqualsEquals() {
    genericMessageTest("MessageEqualsEquals.pd");
  }
  
  @Test
  public void testMessageNotEquals() {
    genericMessageTest("MessageNotEquals.pd");
  }
  
  @Test
  public void testMessagePack() {
    genericMessageTest("MessagePack.pd");
  }
  
  /**
   * Encompasses a generic test for message objects. It processes the graph once and compares the
   * standard output to the golden file, and ensures that the error output is empty.
   * @param testFilename
   */
  private void genericMessageTest(String testFilename) {
    ZenGarden graph = null;
    try {
      graph = new ZenGarden(new File(TEST_PATHNAME, testFilename),
          BLOCK_SIZE, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, SAMPLE_RATE);
    } catch (Exception e) {
      fail(e.toString());
    }
    graph.addListener(this);
    
    graph.process(INPUT_BUFFER, OUTPUT_BUFFER);
    
    String messageStdOutput = stringBuilderStd.toString();
    String messageErrOutput = stringBuilderErr.toString();
    String goldenOutput = readTextFile(new File(TEST_PATHNAME,
        testFilename.split("\\.")[0] + ".golden"));
    
    // ensure that message standard output is same as golden file
    assertEquals(messageStdOutput, goldenOutput);
    
    // ensure that message error output is empty
    assertEquals(messageErrOutput, "");
    
    graph.unloadNativeComponentIfStillLoaded();
  }
  
  private String readTextFile(File file) {
    StringBuilder contents = new StringBuilder();
    try {
      BufferedReader input = new BufferedReader(new FileReader(file));
      try {
        String line = null;
        while (( line = input.readLine()) != null){
          contents.append(line);
          contents.append(System.getProperty("line.separator"));
        }
      }
      finally {
        input.close();
      }
    }
    catch (IOException ioe){
      fail(ioe.toString());
    }
    return contents.toString();
  }

  public void onPrintErr(String message) {
    stringBuilderErr.append(message);
  }

  public void onPrintStd(String message) {
    stringBuilderStd.append(message);
  }

}
