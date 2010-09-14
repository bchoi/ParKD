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

#ifndef _SPLIT_TASK_H_
#define _SPLIT_TASK_H_

#include <iostream>

#include <tbb/task.h>

class Split_task : public tbb::task {
public:
  Split_task(v_Triangle_aux &tris, vp_KdTreeNode_inplace *live, 
             KdTreeNode_inplace *base, uint begin, uint end, uint inst_idx) 
    : tris(tris), live(live), base(base), begin(begin), end(end), inst_idx(inst_idx) {}

  tbb::task *execute() {
    for (uint i = begin; i < end; i++) {
      Triangle_aux &tri = tris[i];
      unsigned char old_membership_size = tri.membership_size;
      unsigned char old_membership[11];
      memcpy(old_membership, tri.membership, tri.membership_size);
      
      // reset membership
      memset(tri.membership, 0, tri.membership_size);
      tri.membership_size = 0;
      
      for (uint j=0;j<old_membership_size;j++) {
        uint l = old_membership[j];

        KdTreeNode_inplace *A, *B, *C; // A split into B and C
        A = (*live)[l];
        B = (KdTreeNode_inplace*)A->left;
        C = (KdTreeNode_inplace*)A->right;
        
        if (A->splitEdge) {
          // ** in the following, pointer comparison suffices because the
          // boxEdge/s are in-place sorted
          // if STR is left of bestEdge, then this goes into B
          BoxEdge *beg = tri.edges[2*A->splitEdge->axis];
          if (beg->edgeType == START && beg < A->splitEdge) {
            tri.membership[tri.membership_size++] = index(B,base)-1;
          }
          BoxEdge *end = tri.edges[2*A->splitEdge->axis + 1];
          // if END is right of bestEdge, then this goes into C
          if (end->edgeType == END && end > A->splitEdge) {
            tri.membership[tri.membership_size++] = index(C,base)-1;
          }
        } else { // not split
          A->cc_triangleIndices->push_back(tri.triangleIndex);
        }

        if (tri.membership_size > 11) {
          std::cout << "Fatal: Can't handle triangles belonging to more than 11 nodes. live: "
                    << live->size() << std::endl;
          exit(-1);
        }
      }
    }

    return NULL;
  }

private:
  const uint begin, end;
  v_Triangle_aux &tris;
  const vp_KdTreeNode_inplace *live;
  const KdTreeNode_inplace *const base;
  // for instrumentation
  uint inst_idx;
};

#endif // _SPLIT_TASK_H_
