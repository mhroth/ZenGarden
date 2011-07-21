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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class ZGSystemTest {
  
  private static final int BLOCK_SIZE = 64;
  private static final int NUM_INPUT_CHANNELS = 2;
  private static final int NUM_OUTPUT_CHANNELS = 2;
  private static final float SAMPLE_RATE = 44100.0f;
  private static final short[] INPUT_BUFFER = new short[BLOCK_SIZE * NUM_INPUT_CHANNELS];
  private static final short[] OUTPUT_BUFFER = new short[BLOCK_SIZE * NUM_OUTPUT_CHANNELS];
  private static final String TEST_TO_PATCH = "#TEST_TO_PATCH";
  private static final String PATCH_TO_TEST = "#PATCH_TO_TEST";
  
  @Before
  public void setUp() throws Exception {
    // nothing to do
  }

  @After
  public void tearDown() throws Exception {
    // nothing to do
  }
  
  @Test
  public void testAddAndRemoveConnections() {
    ZGContext context = new ZGContext(SAMPLE_RATE, BLOCK_SIZE, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS);
    ZGGraph graph = context.newEmptyGraph();
    ZGObject obj0 = graph.addObject("osc~ 440");
    ZGObject obj1 = graph.addObject("dac~");
    graph.addConnection(obj0, 0, obj1, 0);
    graph.addConnection(obj0, 0, obj1, 1);
    graph.removeConnection(obj0, 0, obj1, 0);
    graph.removeConnection(obj0, 0, obj1, 1);
    obj0.remove();
    obj1.remove();
    //graph.removeObject(obj0); // ???
    //graph.removeObject(obj1);
    graph.unattach();
  }
  
  /**
   * Register the PATCH_TO_TEST receiver and send a message to the TEST_TO_PATCH receiver in the patch.
   * The patch should send the same message back to the test. The PATCH_TO_TEST receiver is then
   * unregistered, and another message is sent to the TEST_TO_PATCH receiver. No message should
   * be returned.
   */
  @Test
  public void testRegisterReceiver() {
    final Message message = new Message(0.0, "Hello World!");
    
    // create a new context
    ZGContext context = new ZGContext(SAMPLE_RATE, BLOCK_SIZE, NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS);
    
    // define a ZenGardenListener which asserts that the received message is equal to the one
    // which was sent, from the correct receiver
    ZenGardenListener zgListener = new ZenGardenAdapter() {
      @Override
      public void onMessage(String receiverName, Message receivedMessage) {
        assertEquals(PATCH_TO_TEST, receiverName);
        assertEquals(message, receivedMessage);
      }
    };
    context.addListener(zgListener);
    
    // register the receiver
    context.registerReceiver(PATCH_TO_TEST);
    //ZGGraph graph = context.newGraphFromFile("");
    
    // attach the graph and send the message
    graph.attach();
    context.sendMessage(TEST_TO_PATCH, message);
    context.process(INPUT_BUFFER, OUTPUT_BUFFER); // process once in order to process the message
    
    // remove the listener
    context.removeListener(zgListener);
    
    // unregister the PATCH_TO_TEST receiver
    context.unregisterReceiver(PATCH_TO_TEST);
    
    // add a new listener which will fail if it receiver any message at all
    context.addListener(new ZenGardenAdapter() {
      @Override
      public void onMessage(String receiverName, Message receivedMessage) {
        fail("A message was received despite no receiver being registered: " + receivedMessage.toString());
      }
    });
    
    // send the message
    context.sendMessage(TEST_TO_PATCH, message);
    context.process(INPUT_BUFFER, OUTPUT_BUFFER);
  }
  
}
