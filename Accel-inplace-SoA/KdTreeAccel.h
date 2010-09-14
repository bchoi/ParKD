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

#ifndef _KDTREEACCEL_H_
#define _KDTREEACCEL_H_

#include <tbb/task.h>

#include "TAB.h"
#include "common_inplace.h"
#include "SplitMemo.h"
#include "KdTreeAccel_base.h"
#include "Stats.h"

class KdTreeAccel : public KdTreeAccel_base {
 public:
  KdTreeAccel(TriangleMesh *mesh, uint numThreads, uint max_depth);

  ~KdTreeAccel() {
//     if (kdTreeNodeObj) delete kdTreeNodeObj;
//     kdTreeNodeObj = NULL;
    kdTreeNodeObj->clear();
  }

  // Mandatory functions
  void build();
  std::string impl_string();
  void printTimingStats(std::ostream &out);
  void printTimingStatsCSVHeader(std::ostream &out);
  void printTimingStatsCSV(std::ostream &out);

private:
  v_KdTreeNode_inplace *kdTreeNodeObj;
  KdTreeNode_inplace *root_;
  size_t xbegin_idx, xend_idx, ybegin_idx, yend_idx, zbegin_idx, zend_idx;

  // root task used by _ll functions below
  tbb::task *pRootTask;

  // helper functions
  void sequential_build(v_BoxEdge_inplace &boxEdges, TAB &table, v_Triangle_aux &tris, uint maxDepth);
  void parallel_build(v_BoxEdge_inplace &boxEdges, TAB &table, v_Triangle_aux &tris, uint maxDepth);

  void findBestPlane(TAB &table, v_Triangle_aux &tris,
                   vp_KdTreeNode_inplace *live, SplitMemo *memo);
  void classifyTriangles(v_Triangle_aux &tris, vp_KdTreeNode_inplace *live, 
                SplitMemo *memo, KdTreeNode_inplace *base);
  void fill(v_Triangle_aux &tris, vp_KdTreeNode_inplace *live);
  void moveTriangles(KdTreeNode_inplace *node);

  int begin_idx[3], end_idx[3];

  v_BoxEdge_inplace proxy;

  Stats stats;
};

void impl_usage();

extern bool ver_splitedge;

#endif // _KDTREEACCEL_H_
