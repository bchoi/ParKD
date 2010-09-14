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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>

#include <tbb/scalable_allocator.h>

#include "BoxEdge.h"

// override global new/delete to use scalable_allocator
inline void *operator new(size_t size) {
  return scalable_malloc(size);
}

inline void operator delete(void *p) {
  scalable_free(p);
}

#define START 0
#define END   1

enum Threading { SEQ, SEQ_BOXEDGES, PTHREAD, TBB, TBB2, RAKESH, INTEL };

typedef std::vector<BoxEdge, tbb::scalable_allocator<BoxEdge> > v_BoxEdge;
typedef std::vector<v_BoxEdge, tbb::scalable_allocator<v_BoxEdge> > vv_BoxEdge;
typedef std::vector<BoxEdge*, tbb::scalable_allocator<BoxEdge*> > vp_BoxEdge;
typedef std::vector<vp_BoxEdge, tbb::scalable_allocator<vp_BoxEdge> > vvp_BoxEdge;

#ifdef KDTREE_DEBUG
extern ofstream outfile;
#endif

typedef unsigned long long uint64;

#endif // _COMMON_H_
