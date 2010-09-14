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

#ifndef _KDTREEACCEL_BASE_H_
#define _KDTREEACCEL_BASE_H_

#include <vector>
#include <iostream>

#include "KdTreeNode.h"
#include "TriangleMesh.h"
#include "MantaKDTreeNode.h"
#include "SAH.h"

class KdTreeAccel_base {
public:
  KdTreeAccel_base(TriangleMesh *mesh, 
                   uint numThreads, uint max_depth, 
                   float Ct = Ct_DEFAULT,
                   float Ci = Ci_DEFAULT,
                   float emptyBonus = emptyBonus_DEFAULT);
  
  ~KdTreeAccel_base() { 
    // don't do anything here, kdTreeAccel_tbb2 uses custom alloc
  }
  
  // Pure virtual functions to be overridden by implementations
  virtual void build() = 0;
  virtual std::string impl_string() = 0;
  virtual void printTimingStats(std::ostream &out) = 0;
  virtual void printTimingStatsCSVHeader(std::ostream &out) = 0;
  virtual void printTimingStatsCSV(std::ostream &out) = 0;

  bool writeToFile(char * filename);
  
  void printTree() const {printTreeHelper(m_root);}
  void printGraphviz() const;
  void printGraphvizAccm() const;

  const SAH sah;

protected:
  KdTreeNode *m_root;
  const TriangleMesh *m_mesh;

  uint m_numThreads;
  uint m_maxDepth;

  void printTreeHelper(KdTreeNode *node) const;
  void printGraphvizHelper(KdTreeNode *node, std::ostream &out, unsigned int level) const;
  // this version accumulates branch node triangles down to children
  void printGraphvizHelper(KdTreeNode *node, std::ostream &out, unsigned int level, uint accm) const;

  // useful for writing kd-tree to file
  void packNodesAndItems(KdTreeNode * nodePtr, const int nodeIdx, 
                         std::vector<int> & itemList, std::vector<MantaKDTreeNode> & nodeList);
};



#endif // _KDTREEACCEL_BASE_H_
