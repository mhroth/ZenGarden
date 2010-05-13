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

#ifndef _ZENGARDEN_H_
#define _ZENGARDEN_H_

/**
 * This header file defines the C interface to ZenGarden to the outside world. Include this header
 * along with the <code>libzengarden</code> library in your project in order to integrate it.
 */
#ifdef __cplusplus
class PdGraph;
typedef PdGraph ZGGraph;
extern "C" {
#else
  typedef void ZGGraph;
#endif
  
  /** Create a new graph with the given parameters. */
  ZGGraph *zg_new_graph(char *directory, char *filename, int blockSize, 
      int numInputChannels, int numOutputChannels, float sampleRate);
  
  /** Delete the given graph. */
  void zg_delete_graph(ZGGraph *graph);
  
  /** Process the given graph. */
  void zg_process(ZGGraph *graph, float *inputBuffers, float *outputBuffers);
  
  /**
   * Send a message to the named receiver with the given format at the beginning of the next audio block.
   * E.g., zg_send_message(graph, "#accelerate", "fff", 0.0f, 0.0f, 0.0f);
   * sends a message containing three floats, each with value 0.0f, to all receivers named "#accelerate".
   * Messages may also be formatted with "s" and "b" for symbols and bangs, respectively.
   * E.g., zg_send_message(graph, "test", "s", "hello");
   * E.g., zg_send_message(graph, "test", "b");
   */
  void zg_send_message(ZGGraph *graph, const char *receiverName, const char *messageFormat, ...);
  
  /**
   * Send a message to the named receiver with the given format at the given block index. If the
   * block index is negative or greater than the block size (given when instantiating the graph)
   * the the message will be sent at the very beginning of the next block. A fractional block index
   * may be given, and the message will be evaluated between rendered samples. If the given block
   * index falls outside of the block size (either positive or negative), then the message will be
   * delivered at the beginning of the block.
   * This function is equivalent to e.g., zg_send_message(graph, "#accelerate", 0.0, "fff", 0.0f, 0.0f, 0.0f)
   * E.g., zg_send_message_at_blockindex(graph, "#accelerate", 56.3, "fff", 0.0f, 0.0f, 0.0f);
   * sends a message containing three floats, each with value 0.0f, to all receivers named "#accelerate"
   * between samples 56th and 57th samples (counting from zero) of the block.
   */
  void zg_send_message_at_blockindex(ZGGraph *graph, const char *receiverName, double blockIndex,
      const char *messageFormat, ...);
  
#ifdef __cplusplus
}
#endif

#endif // _ZENGARDEN_H_
