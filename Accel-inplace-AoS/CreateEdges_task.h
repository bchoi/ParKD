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

#ifndef _CREATEEDGES_TASK_H_
#define _CREATEEDGES_TASK_H_

#include "common.h"

class CreateEdges_task {
public:
  v_BoxEdge_inplace &proxy;
  v_Triangle_aux &tris;
  KdTreeNode_inplace *node;
  const std::vector<Triangle> &triangleList;
  uint axis; // which axis are we working on?
  uint axis_offset;

  CreateEdges_task(v_BoxEdge_inplace &proxy, v_Triangle_aux &tris,
                  KdTreeNode_inplace *node, const std::vector<Triangle> &triangleList,
                  uint axis, uint axis_offset)
    : proxy(proxy), node(node), triangleList(triangleList),
      tris(tris), axis(axis), axis_offset(axis_offset) {}

  CreateEdges_task(CreateEdges_task &other, tbb::split)
    : proxy(other.proxy), node(other.node), triangleList(other.triangleList),
      tris(other.tris), axis(other.axis), axis_offset(other.axis_offset) {}

  // go from 0 to n per axis
  void operator()(const tbb::blocked_range<size_t> &r) const {
    for (size_t j=r.begin(); j<r.end(); j++) {
      size_t start = axis_offset+2*(j-axis_offset);
      size_t end = axis_offset+2*(j-axis_offset) + 1;
      new (&proxy[start]) BoxEdge_inplace(triangleList[j-axis_offset].bound.min[axis],
                                      j-axis_offset, START, axis);
      new (&proxy[end]) BoxEdge_inplace(triangleList[j-axis_offset].bound.max[axis],
                                    j-axis_offset, END, axis);
      // Triangle_aux layout [ Xs, Xe, Ys, Ye, Zs, Ze ]
      // just the index into the proxy array (which will be the same in tab/s)
      tris[j-axis_offset].edges[2*axis] = &proxy[start];
      tris[j-axis_offset].edges[2*axis+1] = &proxy[end];
      tris[j-axis_offset].triangleIndex = j-axis_offset;

      // TODO
      // this is unnecessary in the release build -- remove!
      // backpointers set here will always be overwritten after tri/s are
      // sorted - this is just for in-development sanity check
      proxy[start].tri = &tris[j-axis_offset];
      proxy[end].tri = &tris[j-axis_offset];
    }
  }
};

#endif // _CREATEEDGES_TASK_H_
