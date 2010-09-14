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

#ifndef _COMMON_INPLACE_H_
#define _COMMON_INPLACE_H_

#include <vector>

#include <tbb/scalable_allocator.h>

#include "KdTreeNode_inplace.h"
#include "Triangle_aux.h"

typedef unsigned int uint;

typedef std::vector<KdTreeNode_inplace, tbb::cache_aligned_allocator<KdTreeNode_inplace> > v_KdTreeNode_inplace;
typedef std::vector<KdTreeNode_inplace*, tbb::scalable_allocator<KdTreeNode_inplace*> > vp_KdTreeNode_inplace;
typedef std::vector<vp_KdTreeNode_inplace, tbb::scalable_allocator<vp_KdTreeNode_inplace> > vvp_KdTreeNode_inplace;
typedef std::vector<Triangle_aux, tbb::cache_aligned_allocator<Triangle_aux> > v_Triangle_aux;
typedef std::vector<BoxEdge_inplace, tbb::cache_aligned_allocator<BoxEdge_inplace> > v_BoxEdge_inplace;
typedef std::vector<BoxEdge_inplace*, tbb::cache_aligned_allocator<BoxEdge_inplace*> > vp_BoxEdge_inplace;

// this is how you sort boxEdge/s
namespace std {
  template<>
    class less<BoxEdge_inplace> {
  public:
    bool operator()(const BoxEdge_inplace& x, const BoxEdge_inplace& y) const {
      if (x.t == y.t) {
        if (x.triangleIndex != y.triangleIndex) {
          return x.triangleIndex < y.triangleIndex;
        } else {
          return (int)x.edgeType < (int)y.edgeType;
        }
      } else {
        return x.t < y.t;
      }
    }
    bool operator()(const BoxEdge_inplace* x, const BoxEdge_inplace* y) const {
      if (x->t == y->t) {
        if (x->triangleIndex != y->triangleIndex) {
          return x->triangleIndex < y->triangleIndex;
        } else {
          return (int)x->edgeType < (int)y->edgeType;
        }
      } else {
        return x->t < y->t;
      }
    }
  };

  // sort by Xs which is edges[0]
  template<>
  class less<Triangle_aux> {
  public:
    bool operator()(const Triangle_aux& x, const Triangle_aux& y) const {
      return x.edges[0]->t < y.edges[0]->t;
    }
    bool operator()(const Triangle_aux* x, const Triangle_aux* y) const {
      return x->edges[0]->t < y->edges[0]->t;
    }
  };
}

inline uint index(const KdTreeNode *idx, const KdTreeNode *base) {
  return ((KdTreeNode_inplace*)idx)-((KdTreeNode_inplace*)base);
}

// if node falls within live, then it's live
// can get away with pointer comparison because of the way KdTreeNode_inplace/s
// are laid out
inline bool isLive(const KdTreeNode *node, const vp_KdTreeNode_inplace *live) {
  uint idx = ((KdTreeNode_inplace*)node)-((KdTreeNode_inplace*)*live->begin());
  return idx >= 0 && idx < live->size();
}

// TBB parallel classes ==================================================

// setting up boxEdge/s
// I. ll_create_edges
//   0. setup edges (no tri info necessary here)
//   1. setup tri -> x,y,z
// II. sequential
//   2. sort tri by Xs
//   3. Let x,y,z know of the new location of tri
// III. ll_neighbor
//   4. sort x, update tri.x field -|
//   4. sort y, update tri.y field  |-- can be done in parallel
//   4. sort z, update tri.z field -|

#endif // _COMMON_INPLACE_H_

