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

#ifndef PARKDTREENESTED_TASK_H_
#define PARKDTREENESTED_TASK_H_

#include <tbb/task.h>

#include "KdTreeAccel.h"
#include "TriangleMesh.h"
#include "BoundingBox.h"

class ParKdTreeNested_task : public tbb::task {

public:
  const TriangleMesh *mesh;
  BoundingBox& nodeExtent;
  vv_BoxEdge& boxEdgeList;
  int maxDepth;
  KdTreeNode* newNode;
  KdTreeAccel *accel;
  const unsigned int numThreads;
  const unsigned int level; // current level in the tree (root == 0)
  int nA, nB;

  // constructor
  ParKdTreeNested_task(const TriangleMesh *mesh_, BoundingBox& nodeExtent_,
                       vv_BoxEdge& boxEdgeList,
                       int maxDepth_, KdTreeNode *newNode_, KdTreeAccel *accel,
                       unsigned int numThreads, unsigned int level) :
    mesh(mesh_), nodeExtent(nodeExtent_),  boxEdgeList(boxEdgeList),
    maxDepth(maxDepth_), newNode(newNode_), accel(accel), numThreads(numThreads),
    level(level) {}

  tbb::task *execute();

  // Make a leaf
  void makeLeaf(KdTreeNode *newNode, const BoundingBox &nodeExtent,
                const vv_BoxEdge &boxEdgeList);
  
private:
  // serial SPLIT
  void sq_split(const vv_BoxEdge &boxEdgeList,
                const mem_type &membership,
                vv_BoxEdge &left, vv_BoxEdge &right);
  
  // parallel nAnB prescan + final-SAH
  const BoxEdge* ll_nAnB_SAH(const vv_BoxEdge& boxEdgeList,
                             const BoundingBox &nodeExtent,
                             KdTreeAccel* accel, unsigned int triangles);
  
  // parallel SPLIT
  void ll_split(const vv_BoxEdge& boxEdgeList,
                const mem_type& membership,
                vv_BoxEdge& left, vv_BoxEdge& right, int numThreads) ;
  
  // seq SPLIT 2
  void sq_split2(const vv_BoxEdge& boxEdgeList,
                 const mem_type& membership,
                 vv_BoxEdge& left, vv_BoxEdge& right, int left_s, int right_s);
  
  void ll_mem(v_BoxEdge& boxEdge,
              mem_type& membership,
              size_t bestIndex,
              int numThreads);
};

#endif /* PARKDTREENESTED_TASK_H_ */
