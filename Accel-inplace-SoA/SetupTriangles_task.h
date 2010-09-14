/*
   Copyright (c) 2010 University of Illinois
   All rights reserved.

   Developed by:           DeNovo group, Graphis@Illinois
                           University of Illinois
                           http://denovo.cs.illinois.edu
                           http://graphics.cs.illinois.edu

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the
   "Software"), to deal with the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following disclaimers
      in the documentation and/or other materials provided with the
      distribution.

    * Neither the names of DeNovo group, Graphics@Illinois, 
      University of Illinois, nor the names of its contributors may be used to 
      endorse or promote products derived from this Software without specific 
      prior written permission.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
   ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/

#ifndef _SETUPTRIANGLES_TASK_H_
#define _SETUPTRIANGLES_TASK_H_

class SetupTriangles_task {
public:
  v_BoxEdge_inplace &proxy;

  SetupTriangles_task(v_BoxEdge_inplace &proxy) : proxy(proxy) {}
  SetupTriangles_task(SetupTriangles_task &other, tbb::split ) : proxy(other.proxy) {}

  // triangle_tbb2 setup [ Xs, Xe, Ys, Ye, Zs, Ze ]
  void operator()( const tbb::blocked_range<size_t>& r ) const {
    for(size_t I=r.begin(); I!=r.end(); I++) {
      BoxEdge_inplace &edge = proxy[I];
      uint offset = (edge.edgeType == START)?0:1;
      // update my own slot with the new address
      edge.tri->edges[edge.axis*2+offset] = &edge;
    }
  }
};

#endif // _SETUPTRIANGLES_TASK_H_
