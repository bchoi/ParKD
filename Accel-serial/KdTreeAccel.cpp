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

#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "KdTreeAccel.h"
#include "timers.h"
#include "TriangleMesh.h"
#include "BoundingBox.h"

using namespace std;

int grain_size = -1;
bool verbose_level0 = false;

#define level(code) if (verbose_level0 && level==0) { code ; }

string stats[6];

KdTreeAccel::KdTreeAccel(TriangleMesh * mesh, unsigned int numThreads,
                         unsigned int maxDepth) 
  : KdTreeAccel_base(mesh, numThreads, maxDepth) {
  // this is a serial version (numThread > 1 is an error)
  if (numThreads > 1) {
    cerr << "Accel-serial is a serial version; n=1 only is allowed." << endl;
    cerr << " Will run with 1 thread." << endl;
  }
 }

string KdTreeAccel::impl_string() {
  return string("Serial version");
}

void KdTreeAccel::build() {

  // root node
  m_root = new KdTreeNode();
  // bounding box for the root node
  BoundingBox &nodeExtent = *new BoundingBox(m_mesh->boundingBox);

  // list of boxedges
  vv_BoxEdge boxEdgeList(3);

  // construct list of triangle indices (0...n-1),
  // where n is number of triangles
  vector<int, tbb::scalable_allocator<int> > triangleIndices;
  for (int i = 0; i < (int)(m_mesh->triangleList.size()); i++) {
      triangleIndices.push_back(i);
  }

  unsigned int n = triangleIndices.size();
  for (unsigned int i=0;i<3;i++) {
      boxEdgeList[i].resize(2*n);
  }

  // init boxedge lists
  for (unsigned int i = 0; i < 3; i++) {
    for (unsigned int j = 0; j < n; j++) {
      boxEdgeList[i][j*2] = BoxEdge(m_mesh->triangleList[triangleIndices[j]].bound.min[i], triangleIndices[j], START, i);
      boxEdgeList[i][j*2+1] = BoxEdge(m_mesh->triangleList[triangleIndices[j]].bound.max[i], triangleIndices[j], END, i);
    }
  }

  // sort the boxedges
  sort(boxEdgeList[0].begin(), boxEdgeList[0].end());
  sort(boxEdgeList[1].begin(), boxEdgeList[1].end());
  sort(boxEdgeList[2].begin(), boxEdgeList[2].end());

  // serial tree construction using box-edges
  m_root = buildTree_boxEdges(nodeExtent, boxEdgeList, m_maxDepth);
}

KdTreeNode *KdTreeAccel::buildTree_boxEdges(const BoundingBox& nodeExtent,
                                                vv_BoxEdge& boxEdgeList,
                                                int maxDepth) {
  KdTreeNode * newNode = new KdTreeNode();
  unsigned int triangles = boxEdgeList[0].size()/2;
  if (maxDepth == 0 || triangles == 0) {
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->extent = nodeExtent;
    newNode->triangleIndices = new vector<int>();

    for (v_BoxEdge::const_iterator I=boxEdgeList[0].begin(),
        E=boxEdgeList[0].end(); I!=E; I++) {
      if ((*I).edgeType == START) {
        newNode->triangleIndices->push_back((*I).triangleIndex);
      }
    }
    return newNode;
  } else {
    float SAH_best = triangles * sah.m_Ci;
    BoxEdge *bestEdge = NULL;

    for (unsigned int i=0;i<3;i++) {
      unsigned int nA = 0, nB = triangles;
      for (unsigned int j=0;j<boxEdgeList[i].size(); j++) {

        BoxEdge edge = boxEdgeList[i][j];
        if (edge.edgeType == END) {
          nB--;
        }

        float SAH_now = sah(nodeExtent, i, nA, nB, edge.t);

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
      newNode->right = NULL;
      newNode->left = NULL;
      newNode->extent = nodeExtent;
      newNode->triangleIndices = new vector<int>();

      for (v_BoxEdge::const_iterator I=boxEdgeList[0].begin(),
          E=boxEdgeList[0].end(); I!=E; I++) {
        if ((*I).edgeType == START) {
          newNode->triangleIndices->push_back((*I).triangleIndex);
        }
      }
      return newNode;
    }

    vector<char> membership(m_mesh->triangleList.size(), 0);
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
    newNode->splitEdge = bestEdge;

    //    if (ver_splitedge) {
//    if (true) {
//      cerr << 8-maxDepth << " "
//           << setprecision(3) << fixed << " [@ " 
//           << bestEdge->t << " "
//           << (char)(bestEdge->axis + 'X') << " "
//           << (bestEdge->edgeType?"END":"STR") << " "
//           << "tri#:" << bestEdge->triangleIndex << " " 
////            <<  memo[i].nA << ":" << memo[i].nB
//           << " = " << SAH_best << " " << endl;
//    }
    
    newNode->left = buildTree_boxEdges(leftNodeExtent, left, maxDepth-1);
    newNode->right = buildTree_boxEdges(rightNodeExtent, right, maxDepth-1);
    return newNode;
  }
}

void impl_usage() {
  // TODO
}

void KdTreeAccel::printTimingStats(ostream &out) {
  // TODO
}

void KdTreeAccel::printTimingStatsCSVHeader(std::ostream &out) {
  cerr << "\n";
}

void KdTreeAccel::printTimingStatsCSV(std::ostream &out) {
  cerr << "\n";

}
