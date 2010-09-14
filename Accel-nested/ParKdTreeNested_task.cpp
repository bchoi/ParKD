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

#include "ParKdTreeNested_task.h"
#include "ClassifyTriangles_task.h"
#include "MergeMembership_task.h"
#include "FilterGeom_presplit_task.h"
#include "FilterGeom_task.h"
#include "FindBestPlane_prescan_task.h"
#include "FindBestPlane_task.h"
//#include "common.h"
#include "timers.h"
#include "BoundingBox.h"
#include "options.h"

using namespace std;
using namespace tbb;

// parallel task execute
task* ParKdTreeNested_task::execute() {

  if (level == 0) {
    RECORD_TIME(
      accel->stats.build_start_usec,
      accel->stats.build_start);
  }

  unsigned int num_triangles = boxEdgeList[0].size()/2;

  // make this node a leaf (no triangle or reached the max depth)
  if (maxDepth == 0 || num_triangles == 0) {
    makeLeaf(newNode, nodeExtent, boxEdgeList);
    return NULL;
  } else {
    const BoxEdge *bestEdge = NULL;
   
    if (level == 0) {
      RECORD_TIME(
        accel->stats.findBestPlane_usec[0],
        accel->stats.findBestPlane[0]);
    }

    // 1. PRESCAN + FINAL-SAH *********************************************
    bestEdge = ll_nAnB_SAH(boxEdgeList, nodeExtent, accel, num_triangles);

    // make this node a leaf node, if worthwhile splitting plane was not found
    if (!bestEdge) {
      makeLeaf(newNode, nodeExtent, boxEdgeList);
      return NULL;
    }
  
    if (level == 0) {
      RECORD_TIME(
        accel->stats.findBestPlane_usec[1],
        accel->stats.findBestPlane[1]);
    }

    if (level == 0) {
      RECORD_TIME(
        accel->stats.classifyTriangles_usec[0],
        accel->stats.classifyTriangles[0]);
    }

    // 2. MEM *************************************************************
    unsigned int left_child = 0, right_child = 0, straddling = 0;
    unsigned int bestSplitAxis = bestEdge->axis;
    mem_type membership(mesh->triangleList.size()*2, 0);

    v_BoxEdge::const_iterator I = boxEdgeList[bestEdge->axis].begin();
    v_BoxEdge::const_iterator E = boxEdgeList[bestEdge->axis].end();

/* 
    // parallel MEM
    mem_type membership(mesh->triangleList.size()*2, 0);
    ll_mem(boxEdgeList[bestEdge->axis], membership, index_best, numThreads);
*/
    for (; (&(*I)) != bestEdge; I++) {
      BoxEdge edge = *I;
      if(edge.edgeType == START) {
        membership[edge.triangleIndex*2] = 1;
        left_child++;
      }
    }

    for (++I; I != E; I++) {
      BoxEdge edge = *I;
      if (edge.edgeType == END) {
        if (membership[edge.triangleIndex*2] == 1) straddling++;
        membership[edge.triangleIndex*2+1] = 1;
        right_child++;
      }
    }

    if (level == 0) {
      RECORD_TIME(
        accel->stats.classifyTriangles_usec[1],
        accel->stats.classifyTriangles[1]);
    }

    if (level == 0) {
      RECORD_TIME(
        accel->stats.filterGeom_usec[0],
        accel->stats.filterGeom[0]);
    }

    // 3. SPLIT ***********************************************************

    vv_BoxEdge left(3), right(3);
    if (numThreads == 1 && !g_superfluous_prescans) {
      sq_split(boxEdgeList, membership, left, right);
    } else {
      ll_split(boxEdgeList, membership, left, right, numThreads);
    }

    // ********************************************************************
    if (level == 0) {
      RECORD_TIME(
        accel->stats.filterGeom_usec[1],
        accel->stats.filterGeom[1]);
    }

    // define extent of children nodes
    BoundingBox leftNodeExtent(nodeExtent);
    leftNodeExtent.max[bestEdge->axis] = bestEdge->t;
    BoundingBox rightNodeExtent(nodeExtent);
    rightNodeExtent.min[bestEdge->axis] = bestEdge->t;

    // destroy things we don't need anymore
    //delete &boxEdgeList;
    //delete &nodeExtent;

    if (level == 0) {
      RECORD_TIME(
        accel->stats.recursiveTaskCreation_usec[0],
        accel->stats.recursiveTaskCreation[0]);
    }

    // recurse...
    newNode->extent = nodeExtent;
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
      // fork left task
      if (forkLeft) {
        KdTreeNode *newLeftNode = new KdTreeNode();
        tlist.push_back(*new(allocate_child()) ParKdTreeNested_task(mesh, leftNodeExtent,
                                                                    left, maxDepth-1, newLeftNode,
                                                                    accel, numThreads, level+1));
        
        newNode->left = newLeftNode;
      }
      
      // fork right task
      if (forkRight) {
        KdTreeNode *newRightNode = new KdTreeNode();
        tlist.push_back(*new(allocate_child()) ParKdTreeNested_task(mesh, rightNodeExtent,
                                                                    right, maxDepth-1, newRightNode,
                                                                    accel, numThreads, level+1));
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

    if (level == 0) {
      RECORD_TIME(
        accel->stats.recursiveTaskCreation_usec[1],
        accel->stats.recursiveTaskCreation[1]);
    }

    return NULL;
  }
}

// Make a leaf
void ParKdTreeNested_task::makeLeaf(KdTreeNode *newNode, const BoundingBox &nodeExtent,
                                    const vv_BoxEdge &boxEdgeList) {
  newNode->extent = nodeExtent;
  newNode->triangleIndices = new vector<int>();

  for (v_BoxEdge::const_iterator I=boxEdgeList[0].begin(),
         E=boxEdgeList[0].end(); I!=E; I++) {
    if ((*I).edgeType == START) {
      newNode->triangleIndices->push_back((*I).triangleIndex);
    }
  }
}

// serial SPLIT
void ParKdTreeNested_task::sq_split(const vv_BoxEdge &boxEdgeList,
                                    const mem_type &membership,
                                    vv_BoxEdge &left, vv_BoxEdge &right) {
  for(unsigned int i=0;i<3;i++) {
    for (size_t j=0;j<boxEdgeList[i].size();j++) {
      const BoxEdge &edge = boxEdgeList[i][j];

      if (membership[edge.triangleIndex*2]) {
        left[i].push_back(edge);
      } 
      if (membership[edge.triangleIndex*2+1]) {
        right[i].push_back(edge);
      }
    }
  }
}

// parallel MEM
void ParKdTreeNested_task::ll_mem(v_BoxEdge& boxEdge,
                                  mem_type& membership,
                                  size_t bestIndex,
                                  int numThreads) {
  task &pRootTask = *new(task::allocate_root()) empty_task;
  
  task_list tList;
  size_t incr = boxEdge.size() / numThreads;
  
  mem_type membership_local[numThreads];
  for (int i=0;i<numThreads-1;i++) {
    membership_local[i].resize(mesh->triangleList.size()*2);
  } 

  // parallel membership update
  size_t idx = 0;
  for (size_t i=0;i<numThreads-1;i++) {
    tList.push_back(*new(pRootTask.allocate_child())
                    ClassifyTriangles_task(membership_local[i], boxEdge, idx, 
                                           idx+incr, bestIndex));
    idx += incr;
  }
  tList.push_back(*new(pRootTask.allocate_child())
                  ClassifyTriangles_task(membership_local[numThreads-1], 
                                         boxEdge, idx, boxEdge.size(), 
                                         bestIndex));
  pRootTask.set_ref_count((numThreads)+1);
  pRootTask.spawn_and_wait_for_all(tList);

  // parallel merge
  idx = 0;
  incr = (mesh->triangleList.size() * 2) / numThreads;
  for (size_t i=0;i<numThreads-1;i++) {
    tList.push_back(*new(pRootTask.allocate_child())
                    MergeMembership_task(membership_local, membership, idx, 
                                         idx+incr, numThreads));
    idx += incr;
  }
  tList.push_back(*new(pRootTask.allocate_child())
                  MergeMembership_task(membership_local, membership, idx, 
                                       mesh->triangleList.size()*2, numThreads));
  
  pRootTask.set_ref_count((numThreads)+1);
  pRootTask.spawn_and_wait_for_all(tList);
}

// parallel SPLIT
void ParKdTreeNested_task::ll_split(const vv_BoxEdge& boxEdgeList,
                                    const mem_type& membership,
                                    vv_BoxEdge& left, vv_BoxEdge& right, 
                                    int numThreads) {
  PresplitTab pre_tab[3][numThreads+1];

  // init presplit_tab_t
  memset(pre_tab, 0, sizeof(PresplitTab)*3*(numThreads+1));
  
  task &pRootTask = *new(task::allocate_root()) empty_task;
  task_list tList;
  size_t incr = boxEdgeList[0].size() / numThreads;
  
  // pre-split
  for (unsigned int k=0;k<3;k++) {
    size_t idx = 0;
    for (size_t i=0;i<numThreads-1;i++) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FilterGeom_presplit_task(membership, boxEdgeList[k],  
                                               pre_tab[k][i+1], 
                                               idx, idx+incr));
      idx += incr;
    }
    tList.push_back(*new(pRootTask.allocate_child())
                    FilterGeom_presplit_task(membership, boxEdgeList[k], 
                                             pre_tab[k][numThreads],
                                             idx, boxEdgeList[k].size()));
  }
  
  pRootTask.set_ref_count((numThreads)*3+1);
  pRootTask.spawn_and_wait_for_all(tList);
  
  // sequential scan
  for (size_t i=0;i<3;i++) {
    for (size_t j=0;j<numThreads;j++) {
      pre_tab[i][j+1].leftBoxedges += pre_tab[i][j].leftBoxedges;
      pre_tab[i][j+1].rightBoxedges += pre_tab[i][j].rightBoxedges;
    }
  }
  
  for (size_t k=0;k<3;k++) {
    size_t idx = 0;
    left[k].resize(pre_tab[k][numThreads].leftBoxedges);
    right[k].resize(pre_tab[k][numThreads].rightBoxedges);
    
    tList.push_back(*new(pRootTask.allocate_child())
                    FilterGeom_task(boxEdgeList[k], membership, idx, idx+incr,
                                    pre_tab[k][0], left[k], right[k]));
    idx += incr;
    for (size_t i=1;i<numThreads-1;i++) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FilterGeom_task(boxEdgeList[k], membership, idx, idx+incr,
                                      pre_tab[k][i], left[k], right[k]));
      idx += incr;
    }
    if (numThreads > 1) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FilterGeom_task(boxEdgeList[k], membership, idx, boxEdgeList[k].size(),
                                      pre_tab[k][numThreads-1], left[k], right[k]));
    }
  }

  pRootTask.set_ref_count(numThreads*3+1);
  pRootTask.spawn_and_wait_for_all(tList);
}

const BoxEdge* ParKdTreeNested_task::ll_nAnB_SAH(const vv_BoxEdge &boxEdgeList,
                                                 const BoundingBox &nodeExtent,
                                                 KdTreeAccel *accel,
                                                 unsigned int triangles) {
  int index_best = -1;
  float SAH_best = triangles * accel->sah.m_Ci;
  const BoxEdge *bestEdge = NULL;

  // [axis][chunk]
  PrescanTab pre_tab[3][numThreads+1];
  memset(pre_tab, 0, sizeof(PrescanTab)*3*(numThreads+1));

  task &pRootTask = *new(task::allocate_root()) empty_task;

  task_list tList;
  size_t incr = boxEdgeList[0].size() / numThreads;

  // pre-scan
  if (true || numThreads > 1) {
  for (size_t k=0;k<3;k++) {
    size_t idx = 0;
    for (size_t i=1;i<numThreads;i++) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FindBestPlane_prescan_task(boxEdgeList[k], // per axis
                                   pre_tab[k][i],  // per chunk
                                   idx, idx+incr));// range
      idx += incr;
    }
  }

  pRootTask.set_ref_count((numThreads-1)*3+1);
  pRootTask.spawn_and_wait_for_all(tList);

  // sequential merge
  for (size_t i=0;i<3;i++) {
    for (size_t j=1;j<numThreads-1;j++) {
      pre_tab[i][j+1].nA += pre_tab[i][j].nA;
      pre_tab[i][j+1].nB += pre_tab[i][j].nB;
    }
  }
  }

  // nAnB final scan + SAH
  for (size_t k=0;k<3;k++) {
    size_t idx = 0;
    // for single thread?
    tList.push_back(*new(pRootTask.allocate_child())
                    FindBestPlane_task(boxEdgeList[k], pre_tab[k][0], nodeExtent, accel,
                             k, idx, idx+incr, triangles));
    idx += incr;
    for (size_t i=1;i<numThreads-1;i++) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FindBestPlane_task(boxEdgeList[k], pre_tab[k][i], nodeExtent, accel,
                               k, idx, idx+incr, triangles));
      idx += incr;
    }

    if (numThreads > 1) {
      tList.push_back(*new(pRootTask.allocate_child())
                      FindBestPlane_task(boxEdgeList[k], pre_tab[k][numThreads-1], nodeExtent,
                               accel, k, idx, boxEdgeList[k].size(), triangles));
    }
  }

  pRootTask.set_ref_count(numThreads*3+1);
  pRootTask.spawn_and_wait_for_all(tList);

  // sequential scan for the global best SAH
  for (size_t k=0;k<3;k++) {
    for (size_t t=0;t<numThreads;t++) {
      if (pre_tab[k][t].SAH < SAH_best) {
        SAH_best = pre_tab[k][t].SAH;
        bestEdge = pre_tab[k][t].bestEdge;
        index_best = pre_tab[k][t].bestIndex;
      }
    }
  }

  return bestEdge;
}

