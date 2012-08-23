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

#include "BufferPool.h"
#include "DspObject.h"

BufferPool::BufferPool(unsigned short size) {
  bufferSize = size;
 
  zeroBuffer = ALLOC_ALIGNED_BUFFER(bufferSize * sizeof(float));
  memset(zeroBuffer, 0, bufferSize*sizeof(float)); // zero the zero buffer!
}

BufferPool::~BufferPool() {
  // free all buffers, reserved and available
  for (list<std::pair<float *, unsigned int> >::iterator it = reserved.begin(); it != reserved.end(); ++it) {
    FREE_ALIGNED_BUFFER((*it).first);
  }
  while (!pool.empty()) {
    FREE_ALIGNED_BUFFER(pool.top());
    pool.pop();
  }
  FREE_ALIGNED_BUFFER(zeroBuffer);
}

float *BufferPool::getBuffer(unsigned int numDependencies) {
  float *buffer = NULL;
  if (pool.size() > 0) {
    buffer = pool.top();
    pool.pop();
  } else {
    buffer = ALLOC_ALIGNED_BUFFER(bufferSize * sizeof(float));
    memset(buffer, 0, bufferSize * sizeof(float));
  }
  reserved.push_back(std::pair<float *, unsigned int>(buffer, numDependencies));
//  printf("%i/%i buffer used.\n", getNumReservedBuffers(), getNumTotalBuffers());
  return buffer;
}

void BufferPool::releaseBuffer(float *buffer) {
  // an object may try to release the zero buffer. This should not be possible.
  if (buffer == zeroBuffer) return;
  
  for (list<std::pair<float *, unsigned int> >::iterator it = reserved.begin(); it != reserved.end(); ++it) {
    if ((*it).first == buffer) {
      --((*it).second);
      if ((*it).second <= 0) {
        reserved.erase(it);
        pool.push(buffer);
//        printf("%i/%i buffer used.\n", getNumReservedBuffers(), getNumTotalBuffers());
        break;
      }
    }
  }
  // if the buffer is not in the reserved pool, nothing changes. Untracked buffers are left alone.
}

void BufferPool::reserveBuffer(float *buffer, unsigned int reserveCount) {
  if (buffer == zeroBuffer) return; // no need to reserve the zero buffer
  
  for (list<std::pair<float *, unsigned int> >::iterator it = reserved.begin(); it != reserved.end(); ++it) {
    if ((*it).first == buffer) {
      (*it).second += reserveCount;
      return;
    }
  }
  
  printf("Attempt to reserve unreserved buffer %p +%i.\n  "
      "This may be ok if the buffer is global such as an adc~ input buffer.\n", buffer, reserveCount);
}

/*
void BufferPool::resizeBuffers(unsigned int newBufferSize) {
  for (list<std::pair<float *, unsigned int> >::iterator it = reserved.begin(); it != reserved.end(); ++it) {
    float *buffer = (*it).first;
    float *newBuffer = (float *) realloc(buffer, newBufferSize * sizeof(float));
    if (newBuffer != buffer) { // buffer should start at same address so that it is still 16-byte aligned
      free(buffer);
      free(newBuffer);
      (*it).first = (float *) valloc(newBufferSize * sizeof(float));
    }
  }
  
  // TODO(mhroth): resize all buffers in pool
}
*/
