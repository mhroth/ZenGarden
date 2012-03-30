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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import me.rjdj.zengarden.Message.MessageType;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.util.HashSet;

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
  
  /**
   * A very basic test ensuring that a context can be created for nominal arguments.
   */
  @Test
  public void testNewContext() {
    ZGContext context = new ZGContext(NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, BLOCK_SIZE, SAMPLE_RATE);
    assertNotNull(context);
  }
  
  /**
   * Ensures that an object initialised with a valid string will create a non-<code>null</code> <code>ZGObject</code>.
   * An object added to a graph with an invalid string will return <code>null</code>
   */
  @Test
  public void testAddObject() {
    ZGContext context = new ZGContext(NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, BLOCK_SIZE, SAMPLE_RATE);
    ZGGraph graph = context.newGraph();
    ZGObject goodOscObj = graph.addObject("osc~ 440"); // this described a valid Pd object
    assertNotNull(goodOscObj);
    ZGObject badOscObj = graph.addObject("osc 440"); // this describes an invalid Pd object
    assertNull(badOscObj);
  }
  
  @Test
  public void testAddAndRemoveConnections() {
    ZGContext context = new ZGContext(NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, BLOCK_SIZE, SAMPLE_RATE);
    ZGGraph graph = context.newGraph();
    ZGObject obj0 = graph.addObject("osc~ 440");
    ZGObject obj1 = graph.addObject("dac~");
    graph.addConnection(obj0, 0, obj1, 0);
    assertEquals(1, obj0.getOutgoingConnections(0).size()); // 1 connection at osc~ outlet
    assertEquals(1, obj1.getIncomingConnections(0).size()); // 1 connection at left dac~ inlet
    assertEquals(0, obj1.getIncomingConnections(1).size()); // 0 connections at right dac~ inlet
    
    graph.addConnection(obj0, 0, obj1, 1);
    assertEquals(2, obj0.getOutgoingConnections(0).size()); // 2 connections at osc~ outlet
    assertEquals(1, obj1.getIncomingConnections(0).size()); // 1 connection at left dac~ inlet
    assertEquals(1, obj1.getIncomingConnections(1).size()); // 1 connection at right dac~ inlet

    graph.removeConnection(obj0, 0, obj1, 0);
    assertEquals(1, obj0.getOutgoingConnections(0).size()); // 1 connection at osc~ outlet
    assertEquals(0, obj1.getIncomingConnections(0).size()); // 0 connection at left dac~ inlet
    assertEquals(1, obj1.getIncomingConnections(1).size()); // 1 connections at right dac~ inlet
    
    obj0.remove(); // remove osc~ alltogether, should remove all remaining connections
    assertEquals(0, obj1.getIncomingConnections(1).size()); // 0 connections at right dac~ inlet
  }
  
  /**
   * Register the PATCH_TO_TEST receiver and send a message to the TEST_TO_PATCH receiver in the patch.
   * The patch should send the same message back to the test. The PATCH_TO_TEST receiver is then
   * unregistered, and another message is sent to the TEST_TO_PATCH receiver. No message should
   * be returned.
   */
  @Test
  public void testRegisterReceiver() {
    final Message message = new Message(0.0, "hello!");
    final HashSet<Boolean> hashSet = new HashSet<Boolean>();
    
    // create a new context
    ZGContext context = new ZGContext(NUM_INPUT_CHANNELS, NUM_OUTPUT_CHANNELS, BLOCK_SIZE, SAMPLE_RATE);
    
    // define a ZenGardenListener which asserts that the received message is equal to the one
    // which was sent, from the correct receiver
    ZenGardenListener zgListener = new ZenGardenAdapter() {
      @Override
      public void onMessage(String receiverName, Message receivedMessage) {
        assertEquals(PATCH_TO_TEST, receiverName);
        assertEquals(message, receivedMessage);
        hashSet.add(new Boolean(true));
      }
    };
    context.addListener(zgListener);
    
    // register the receiver
    context.registerReceiver(PATCH_TO_TEST);
    ZGGraph graph = context.newGraph(new File("./test/RegisterReceiver.pd"));
    
    // attach the graph and send the message
    graph.attach();
    context.sendMessage(TEST_TO_PATCH, message);
    context.process(INPUT_BUFFER, OUTPUT_BUFFER); // process once in order to process the message
    if (!hashSet.contains(new Boolean(true))) {
      fail("Context did not receive a message callback.");
    }
    
    // remove the listener
    context.removeListener(zgListener);
    
    // unregister the PATCH_TO_TEST receiver
    context.unregisterReceiver(PATCH_TO_TEST);
    
    // add a new listener which will fail if it receiver any message at all
    context.addListener(new ZenGardenAdapter() {
      @Override
      public void onMessage(String receiverName, Message receivedMessage) {
        fail("A message was received from \"" + receiverName  +
            "\" despite no receiver being registered: " + receivedMessage.toString());
      }
    });
    
    // send the message
    context.sendMessage(TEST_TO_PATCH, message);
    context.process(INPUT_BUFFER, OUTPUT_BUFFER);
  }
  
  @Test(expected=IllegalArgumentException.class)
  public void testMessage() {
    Message message = new Message(0.0, 0.5f);
    assertEquals("f", message.getTypeString());
    assertEquals(MessageType.FLOAT, message.getType(0));
    assertEquals(1, message.getNumElements());
    assertTrue(0.5f == message.getFloat(0));
    assertTrue(0.0 == message.getTimestamp());
    
    message = new Message(0.0, "hello");
    assertEquals("s", message.getTypeString());
    assertEquals(MessageType.SYMBOL, message.getType(0));
    assertEquals(1, message.getNumElements());
    assertEquals("hello", message.getSymbol(0));
    
    // this constructor will throw an IllegalArgumentException due to the space in the submitted
    // symbol. This exception is expected.
    message = new Message(0.0, "hello world");

    message = new Message(0.0, "!");
    assertEquals("b", message.getTypeString());
    assertEquals(MessageType.BANG, message.getType(0));
    assertEquals(1, message.getNumElements());
    
    message = new Message(0.0, new Object());
    assertEquals(MessageType.BANG, message.getType(0));
    
    Message anotherBangMessage = new Message(0.0, Message.BANG_STRING);
    assertEquals(message, anotherBangMessage);
  }
}
