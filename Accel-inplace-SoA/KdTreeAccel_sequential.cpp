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

#include <iostream>
#include <iomanip>
#include <limits>

#include "options.h"
#include "KdTreeAccel.h"
#include "timers.h"

using namespace std;

void KdTreeAccel::sequential_build(v_BoxEdge_inplace &boxEdges, TAB &table,
                                   v_Triangle_aux &tris, uint maxDepth) {
  RECORD_TIME(
    stats.build_start_usec,
    stats.build_start);

  for (uint i=0;i<tris.size();i++) {
    tris[i].membership_size = 1;
    tris[i].membership[0] = 0;
  }

  uint n = m_mesh->triangleList.size();

  // x, y, z
  int begin_idx[3] = { 0, 2*n, 4*n };
  int end_idx[3] = { 2*n, 4*n, 6*n };

  v_KdTreeNode_inplace &nodeObjs = *kdTreeNodeObj;
  vp_KdTreeNode_inplace *live = new vp_KdTreeNode_inplace();
  live->push_back(root_);

  KdTreeNode_inplace *base = root_;

  // each iteration builds a level
  for (uint level=0; level<maxDepth; level++) {

    // FindBestPlane
    RECORD_TIME(
      stats.findBestPlane_usec[level][0],
      stats.findBestPlane[level][0]);

    SplitMemo memo[live->size()];
    uint running[live->size()][2]; // 0 : nA, 1 : nB
    
    // superfluous pre-scan -- for comparison purposes only
    if (g_superfluous_prescans) {
      for (uint axis=0;axis<3;axis++) {
        for (uint i=begin_idx[axis]; i<end_idx[axis]; i++) {
          for (uint j=0;j<table.tri_tab[i]->membership_size;j++) {
            unsigned char l=table.tri_tab[i]->membership[j];
            if (table.edgeType_tab[i] == END) {
              running[l][1]--;
            }

            if (table.edgeType_tab[i] == START) {
              running[l][0]++;
            }
          }
        }
        for (uint l=0;l<live->size();l++) {
          running[l][0] = 0;
          running[l][1] = (*live)[l]->triangleCount;
        }
      }
    }

    // final + SAH
    for (uint l=0;l<live->size();l++) {
      running[l][0] = 0;
      running[l][1] = (*live)[l]->triangleCount;
      memo[l].SAH = numeric_limits<float>::max();
      memo[l].left = 0;
      memo[l].right = 0;
      memo[l].straddle = 0;
    }
    
    for (uint axis=0;axis<3;axis++) {
      for (uint i=begin_idx[axis]; i<end_idx[axis]; i++) {
        for (uint j=0;j<table.tri_tab[i]->membership_size;j++) {
          unsigned char l=table.tri_tab[i]->membership[j];
          if (table.edgeType_tab[i] == END) {
            running[l][1]--;
          }
          float SAH = this->sah((*live)[l]->extent, axis,
                                running[l][0], running[l][1],
                                boxEdges[i].t);
          
          if (SAH < memo[l].SAH) {
            memo[l].SAH = SAH;
            memo[l].nA = running[l][0];
            memo[l].nB = running[l][1];
            memo[l].split = i;
            memo[l].axis = axis;
          }                                 
          if (table.edgeType_tab[i]== START) {
            running[l][0]++;
          }
        }
      }
      for (uint l=0;l<live->size();l++) {
        running[l][0] = 0;
        running[l][1] = (*live)[l]->triangleCount;
      }
    }
    
    RECORD_TIME(
      stats.findBestPlane_usec[level][1],
      stats.findBestPlane[level][1]);
    
    // NEWGEN
    RECORD_TIME(
      stats.newGen_usec[level][0],
      stats.newGen[level][0]);

    vp_KdTreeNode_inplace *newLive = new vp_KdTreeNode_inplace(); // next gen live
    KdTreeNode_inplace *newNode;
    uint frontier = index((*live)[live->size()-1], root_) + 1;
    for(uint i=0;i<live->size();i++) {
      // if it's worth splitting
      if ((*live)[i]->triangleCount > 0 
          && sah.m_Ci * (*live)[i]->triangleCount > memo[i].SAH) {
        // set splitEdge
        (*live)[i]->splitEdge = &boxEdges[memo[i].split];
        
        // pull two at the end and make them left and right for this kdTreeNode
        // keep out the ones that are empty from newLive
        // if nA of bestEdge for (*live)[i] is 0, then left child is empty!
        if (memo[i].nA != 0) {
          newNode = &(nodeObjs[frontier++]);
          newNode->extent = (*live)[i]->extent;
          newNode->extent.max[(*live)[i]->splitEdge->axis]
            = (*live)[i]->splitEdge->t;
          newNode->triangleIndices = new vector<int>();
          newNode->triangleCount = memo[i].nA;
          newLive->push_back(newNode);
          (*live)[i]->left = newNode;
        }
        
        // if nB of bestEdge for (*live)[i] is 0, then right child is empty!
        if (memo[i].nB != 0) {
          newNode = &(nodeObjs[frontier++]);
          newNode->extent = (*live)[i]->extent;
          newNode->extent.min[(*live)[i]->splitEdge->axis]
            = (*live)[i]->splitEdge->t;
          newNode->triangleIndices = new vector<int>();
          newNode->triangleCount = memo[i].nB;
          newLive->push_back(newNode);
          (*live)[i]->right = newNode;
        }
      }
    }

    RECORD_TIME(
      stats.newGen_usec[level][1],
      stats.newGen[level][1]);
    
    // ClassifyTriangles - done over tris (array of _triangles_)
    RECORD_TIME(
      stats.classifyTriangles_usec[level][0],
      stats.classifyTriangles[level][0]);

    for (v_Triangle_aux::iterator I=tris.begin(), E=tris.end(); I!=E; I++) {
      Triangle_aux &tri = *I;
      unsigned char old_membership_size = tri.membership_size;
      unsigned char old_membership[11];
      memcpy(old_membership, tri.membership, tri.membership_size);

      // reset membership
      memset(tri.membership, 0, tri.membership_size);
      tri.membership_size = 0;

      for (uint j=0;j<old_membership_size;j++) {
        uint l = old_membership[j];
        unsigned char both = 0;
        
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
            memo[l].left++;
            both++;
          }
          BoxEdge *end = tri.edges[2*A->splitEdge->axis + 1];
          // if END is right of bestEdge, then this goes into C
          if (end->edgeType == END && end > A->splitEdge) {
            tri.membership[tri.membership_size++] = index(C,base)-1;
            memo[l].right++;
            both++;
          }
          if (both > 1) {
            memo[l].left--;
            memo[l].right--;
            memo[l].straddle++;
          }
        } else { // not split
          A->triangleIndices->push_back(tri.triangleIndex);
        }
        if (tri.membership_size > 11) {
          cout << "Fatal: Can't handle triangles belonging to more than 11 nodes." << endl;
          exit(-1);
        }
      }
    }

    RECORD_TIME(
      stats.classifyTriangles_usec[level][1],
      stats.classifyTriangles[level][1]);

    // print the split edge
    if (ver_splitedge) {
      for (uint i=0;i<live->size();i++) {
        if (memo[i].split < boxEdges.size() && 
            memo[i].SAH < numeric_limits<float>::max()) {
          cerr << level << " "
               << setprecision(3) << fixed << " [@ " 
               << boxEdges[memo[i].split].t << " "
               << ((memo[i].axis==0)?"X":(memo[i].axis==1)?"Y":"Z") << " "
               << (boxEdges[memo[i].split].edgeType?"END":"STR") << " "
               << "tri#:" << boxEdges[memo[i].split].triangleIndex << " " 
               <<  memo[i].nA << ":" << memo[i].nB
               << " = " << memo[i].SAH << "\n";
        } else {
          cerr << level << " not split" << endl;
        }
      }
    }

    live = newLive;
    base = (*live)[live->size()-1];
  }

  // FILL - final pass to fill in the tree
  RECORD_TIME(
    stats.fill_usec[0],
    stats.fill[0]);

  for (v_Triangle_aux::iterator I=tris.begin(), E=tris.end();
       I!=E; I++) {
    Triangle_aux &tri = *I;
    for (uint j=0;j<tri.membership_size;j++) {
      unsigned char l=tri.membership[j];
      (*live)[l]->triangleIndices->push_back(tri.triangleIndex);
    }
  }
  
  RECORD_TIME(
    stats.fill_usec[1],
    stats.fill[1]);

  m_root = root_;

  RECORD_TIME(
    stats.build_finish_usec,
    stats.build_finish);

  return;
}
