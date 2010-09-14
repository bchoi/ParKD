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

#include <vector>

#include <tbb/task.h>

#include "ParKdTreeNested_np_task.h"
#include "common.h"
#include "BoundingBox.h"

using namespace std;
using namespace tbb;

task* ParKdTreeNested_np_task::execute() {

  unsigned int num_triangles = boxEdgeList[0].size()/2;
  if (maxDepth == 0 || num_triangles == 0) {
    makeLeaf(newNode, nodeExtent, boxEdgeList);
    return NULL;
  } else {
    float SAH_best = num_triangles * accel->sah.m_Ci;
    BoxEdge *bestEdge = NULL;
    
    for (unsigned int i=0;i<3;i++) {
      unsigned int nA = 0, nB = num_triangles;
      for (unsigned int j=0;j<boxEdgeList[i].size(); j++) {
        BoxEdge edge = boxEdgeList[i][j];
        if (edge.edgeType == END) {
          nB--;
        }
        
        float SAH_now = accel->sah(nodeExtent, i, nA, nB, edge.t);
        if (SAH_now < SAH_best) {
          SAH_best = SAH_now;
          bestEdge = &boxEdgeList[i][j];
        }

        if (edge.edgeType == START) {
          nA++;
        }
      }
    }
    // not worth splitting
    if (!bestEdge) {
      makeLeaf(newNode, nodeExtent, boxEdgeList);
      return NULL;
    }

    vector<char> membership(mesh->triangleList.size(), 0);

    vv_BoxEdge left(3), right(3);
    unsigned int left_s = 0, right_s = 0;
    v_BoxEdge::const_iterator I = boxEdgeList[bestEdge->axis].begin(),
    E = boxEdgeList[bestEdge->axis].end();
    for (; (&(*I)) != bestEdge; I++) {
      BoxEdge edge = *I;
      if (edge.edgeType == START) {
        membership[edge.triangleIndex] += 1;
        left_s++;
      }
    }
    for (++I; I != E; I++) {
      BoxEdge edge = *I;
      if (edge.edgeType == END) {
        membership[edge.triangleIndex] += 2;
        right_s++;
      }
    }
    for (unsigned int i=0;i<3;i++) {
      for(v_BoxEdge::const_iterator I=boxEdgeList[i].begin(),
          E=boxEdgeList[i].end(); I!=E; I++) {
        BoxEdge edge = *I;
        if (membership[edge.triangleIndex] & 1) {
          left[i].push_back(edge);
        }
        if (membership[edge.triangleIndex] & 2) {
          right[i].push_back(edge);
        }
      }
    }

    BoundingBox leftNodeExtent(nodeExtent), rightNodeExtent(nodeExtent);
    leftNodeExtent.max[bestEdge->axis] = bestEdge->t;
    rightNodeExtent.min[bestEdge->axis] = bestEdge->t;

    // recurse..
    newNode->extent = nodeExtent;
    // newNode->isLeaf = false;
    // newNode->splitAxis = bestEdge->axis;
    // newNode->splitValue = bestEdge->t;
    newNode->splitEdge = bestEdge;

    // dynamic load-balancing
    unsigned int threshold = 0;

    // decide what to do first..
    bool forkLeft = false, forkRight = false;
    unsigned ref = 0;
    if (left[0].size() > threshold)  {
      forkLeft = true;
      ref++;
    }
    if (right[0].size() > threshold) {
      forkRight = true;
      ref++;
    }

    if (ref > 0) {
      set_ref_count(ref+1);

      task_list tlist;
      // left task
      if (forkLeft) {
          KdTreeNode *newLeftNode = new KdTreeNode();
          tlist.push_back(*new(allocate_child()) ParKdTreeNested_np_task(mesh, leftNodeExtent, left, maxDepth-1, newLeftNode, accel, numThreads, level+1));
          newNode->left = newLeftNode;
      }
      // right task
      if (forkRight) {
          KdTreeNode *newRightNode = new KdTreeNode();
          tlist.push_back(*new(allocate_child()) ParKdTreeNested_np_task(mesh, rightNodeExtent, right, maxDepth-1, newRightNode, accel, numThreads, level+1));
          newNode->right = newRightNode;
      }
      spawn(tlist);

      // for non-forking ones, this task builds the rest of the subtrees
      if (!forkLeft) {
        newNode->left = accel->buildTree_boxEdges(leftNodeExtent, left, maxDepth-1);
      } else if (!forkRight) {
        newNode->right = accel->buildTree_boxEdges(rightNodeExtent, right, maxDepth-1);
      }
      wait_for_all();
    } else {
      newNode->left = accel->buildTree_boxEdges(leftNodeExtent, left, maxDepth-1);
      newNode->right = accel->buildTree_boxEdges(rightNodeExtent, right, maxDepth-1);

    }

    return NULL;
  }
}
