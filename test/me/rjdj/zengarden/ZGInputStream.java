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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

class ZGInputStream extends InputStream {
  
  private ZGContext context;
  short[] inputBuffer;
  short[] outputBuffer;
  int pos, byteIndex;
  
  /**
   * A wrapper around a ZenGarden context in the form of at {@link InputStream}.
   * @param pdFile
   * @param numInputChannels
   * @param numOutputchannels
   * @param blockSize
   * @param sampleRate
   */
  public ZGInputStream(File pdFile, int numInputChannels, int numOutputchannels, int blockSize, float sampleRate) {
    context = new ZGContext(numInputChannels, numOutputchannels, blockSize, sampleRate);
    context.addListener(new ZenGardenAdapter());
    ZGGraph graph = context.newGraph(pdFile);
    graph.attach();
    
    inputBuffer = new short[numInputChannels*blockSize];
    outputBuffer = new short[numOutputchannels*blockSize];
    
    context.process(inputBuffer, outputBuffer);
    
    pos = 0;
  }

  @Override
  public int available () {
    return Integer.MAX_VALUE;
  }

  @Override
  public boolean markSupported() {
    return false;
  }

  @Override
  public int read() throws IOException {
    short s = outputBuffer[pos];
    if (byteIndex == 0) {
      byteIndex = 1;
      return (int) (((int) s) & 0x000000FF);
    } else {
      byteIndex = 0;
      if (++pos == outputBuffer.length) {
        pos = 0;
        context.process(inputBuffer, outputBuffer);
      }
      return (int) ((((int) s) & 0x0000FF00) >> 8);
    }
  }
}