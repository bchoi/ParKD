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

#include <iomanip>
#include <limits>

#include "KdTreeAccel.h"
#include "options.h"
#include "PrescanTab.h"
#include "FindBestPlane_prescan_task.h"
#include "FindBestPlane_task.h"
#include "Split_task.h"
#include "timers.h"

using namespace std;

void KdTreeAccel::parallel_build(v_BoxEdge_inplace &boxEdges, TAB &table, 
                                 v_Triangle_aux &tris,
                                 uint maxDepth) {
  RECORD_TIME(
    stats.build_start_usec,
    stats.build_start);

  for (uint i=0;i<tris.size();i++) {
    tris[i].membership_size = 1;
    tris[i].membership[0] = 0;
  }

  uint n = m_mesh->triangleList.size();

  // x, y, z
  for (uint i=0;i<3;i++) {
    begin_idx[i] = 2*n*i;
    end_idx[i] = 2*n*(i+1);
  }

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
    findBestPlane(table, tris, live, memo);

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
          newNode->cc_triangleIndices = new tbb::concurrent_vector<int>();
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
          newNode->cc_triangleIndices = new tbb::concurrent_vector<int>();
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

    classifyTriangles(tris, live, memo, base);
    
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
               << " = " << memo[i].SAH << " " << endl;
        } else {
          cerr << level << " not split" << endl;
        }
      }
    }

    live = newLive;
    base = (*live)[live->size()-1];
  }

  // final pass to fill in the tree
  RECORD_TIME(
    stats.fill_usec[0],
    stats.fill[0]);

  fill(tris, live);

  RECORD_TIME(
    stats.fill_usec[1],
    stats.fill[1]);

  m_root = root_;

  RECORD_TIME(
    stats.build_finish_usec,
    stats.build_finish);

  // hackery -- move ints from cc_triangleIndices to triangleIndices
  moveTriangles((KdTreeNode_inplace*)m_root);
  return;
}

void KdTreeAccel::findBestPlane(TAB &table, v_Triangle_aux &tris,
                                vp_KdTreeNode_inplace *live, SplitMemo *memo) {
  // nAnB prescan
  tbb::task_list tList;
  // [axis][chunk][live node]
  PrescanTab pre_tab[3][m_numThreads][live->size()];
  memset(pre_tab, 0, sizeof(PrescanTab)*3*(m_numThreads)*live->size());
  uint incr = end_idx[0]/m_numThreads;
  
  for (uint k=0;k<3;k++) {
    uint idx = begin_idx[k];
    for (uint i=0;i<m_numThreads-1;i++) {
      tList.push_back(*new(pRootTask->allocate_child()) 
                      FindBestPlane_prescan_task(table, tris, live, 
                                                 pre_tab[k][i], idx, idx+incr));
      idx += incr;
    }
  }

  if (g_verbose) {
    pRootTask->set_ref_count(m_numThreads*3+1);
  } else {
    pRootTask->set_ref_count((m_numThreads-1)*3+1);
  }
  pRootTask->spawn_and_wait_for_all(tList);

  // sequential merge -- pass prescan results forward
  for (uint i=0;i<3;i++) {
    for (uint j=0;j<m_numThreads-2;j++) {
      for (uint k=0;k<live->size();k++) {
        pre_tab[i][j+1][k].nA += pre_tab[i][j][k].nA;
        pre_tab[i][j+1][k].nB += pre_tab[i][j][k].nB;
      }
    }
  }

  // nAnB final-scan + SAH
  SplitMemo memos[3][m_numThreads][live->size()];
  memset(memos, 0, sizeof(SplitMemo)*3*(m_numThreads)*live->size());

  for (uint k=0;k<3;k++) {
    uint idx = begin_idx[k];
    tList.push_back(*new(pRootTask->allocate_child())
                    FindBestPlane_task(table, tris, live, NULL, memos[k][0], k, idx, idx+incr, 
                                      this));
    idx += incr;
    for (uint i=1;i<m_numThreads;i++) {
      tList.push_back(*new(pRootTask->allocate_child())
                      FindBestPlane_task(table, tris, live, pre_tab[k][i-1], memos[k][i], k, 
                                        idx, idx+incr, this));
      idx += incr;
    }
  }
  pRootTask->set_ref_count(m_numThreads*3+1);
  pRootTask->spawn_and_wait_for_all(tList);

  // sequentially merge memos into memo
  memcpy(memo, memos[0][0], sizeof(SplitMemo)*live->size());
  for (uint k=0;k<3;k++) {
    for (uint t=0;t<m_numThreads;t++) {
      for (uint l=0;l<live->size();l++) {
        if (memo[l].SAH > memos[k][t][l].SAH) {
          memcpy(&memo[l], &memos[k][t][l], sizeof(SplitMemo));
        }
      }
    }
  }
}

void KdTreeAccel::classifyTriangles(v_Triangle_aux &tris, vp_KdTreeNode_inplace *live, 
                           SplitMemo *memo, KdTreeNode_inplace *base) {
  tbb::task_list tList;
  uint incr = tris.size()/m_numThreads;
  uint idx = 0;
  uint stat[m_numThreads-1][live->size()][3]; // left, straddle, right
  uint task_id = 0;
  for (uint t=0;t<m_numThreads-1;t++) {
    tList.push_back(*new(pRootTask->allocate_child()) Split_task(tris, live, base, idx, idx+incr, task_id++));
    idx += incr;
  }
  tList.push_back(*new(pRootTask->allocate_child()) Split_task(tris, live, base, idx, tris.size(), task_id++));

  pRootTask->set_ref_count(m_numThreads+1);
  pRootTask->spawn_and_wait_for_all(tList);
}

void KdTreeAccel::fill(v_Triangle_aux &tris, vp_KdTreeNode_inplace *live) {
    for (v_Triangle_aux::iterator I=tris.begin(), E=tris.end();
       I!=E; I++) {
    Triangle_aux &tri = *I;
    for (uint j=0;j<tri.membership_size;j++) {
      unsigned char l=tri.membership[j];
      (*live)[l]->cc_triangleIndices->push_back(tri.triangleIndex);
    }
  }
}

void KdTreeAccel::moveTriangles(KdTreeNode_inplace *node) {
  if (!node->triangleIndices) {
    node->triangleIndices = new vector<int>();
  }
  if (node->cc_triangleIndices && node->cc_triangleIndices->size()) {
    copy(node->cc_triangleIndices->begin(), node->cc_triangleIndices->end(),
         insert_iterator<vector<int> >(*node->triangleIndices, node->triangleIndices->begin()));
  }
  if (node->left) moveTriangles((KdTreeNode_inplace*)node->left);
  if (node->right) moveTriangles((KdTreeNode_inplace*)node->right);
}
