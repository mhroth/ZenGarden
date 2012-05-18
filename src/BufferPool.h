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

#ifndef _BUFFER_POOL_
#define _BUFFER_POOL_

#include <list>
#include <stack>
using namespace std;

class BufferPool {
  public:
    BufferPool(unsigned short bufferSize);
    ~BufferPool();
  
    /**
     * If a buffer is available, it is reserved with a number of dependencies. Otherwise a new
     * buffer is created.
     */
    float *getBuffer(unsigned int numDependencies);
  
    /** The reference count of the buffer is reduced by one, possibly made available for other objects. */
    void releaseBuffer(float *buffer);
  
    /** Add to the reserve cound of the given buffer. */
    void reserveBuffer(float *buffer, unsigned int reserveCount);
  
    /** Resizes all buffers in the pool (reserved and available). */
//    void resizeBuffers(unsigned int newBufferSize);
  
    float *getZeroBuffer() { return zeroBuffer; }
  
    unsigned int getNumReservedBuffers() { return reserved.size(); }
    unsigned int getNumAvailableBuffers() { return pool.size(); }
    unsigned int getNumTotalBuffers() { return (pool.size() + reserved.size()); }
  
  private:
    /** A pool of reserved buffers, paired with the number of outstanding dependencies. */
    list<std::pair<float *, unsigned int> > reserved;
  
    /** A pool of available buffers. */
    stack<float *> pool;
  
    float *zeroBuffer;
  
    unsigned short bufferSize;
};

#endif // _BUFFER_POOL_
