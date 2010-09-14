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

#ifndef KDTREEACCEL_H_
#define KDTREEACCEL_H_

#include <vector>
#include <ostream>

#include "KdTreeAccel_base.h"
#include "TriangleMesh.h"
#include "BoundingBox.h"
#include "common.h"

class KdTreeAccel : public KdTreeAccel_base {
public:
    KdTreeAccel(TriangleMesh * _mesh, unsigned int numThreads,
                unsigned int max_depth);
  
  /* Will take this out later - BC [2010-03-27 Sat 00:49]
    KdTreeAccel(TriangleMesh *mesh, BoundingBox &nodeExtent,
                std::vector<int> trianglesIndices, unsigned int maxDepth);
  */

    ~KdTreeAccel() { }

  // Mandatory functions
  void build();
  std::string impl_string();
  void printTimingStats(std::ostream &out);
  void printTimingStatsCSVHeader(std::ostream &out);
  void printTimingStatsCSV(std::ostream &out);

	// for sequential tree-building (boxedge-based)
  KdTreeNode * buildTree_boxEdges(const BoundingBox& nodeExtent,
                                    vv_BoxEdge& boxEdgeList,
                                    int maxDepth);
};

void impl_usage();

#endif // KDTREEACCEL_H_
