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

#ifndef _FINDBESTPLANE_AOS_PRESCAN_TASK_H_
#define _FINDBESTPLANE_AOS_PRESCAN_TASK_H_

#include "common_inplace.h"
#include "PrescanTab.h"

class FindBestPlane_AoS_prescan_task : public tbb::task {
public:
  FindBestPlane_AoS_prescan_task(v_BoxEdge_inplace &boxEdges, v_Triangle_aux &tris,
                                 vp_KdTreeNode_inplace *live, PrescanTab *tab,
                                 uint begin, uint end): 
    boxEdges(boxEdges), tris(tris), live(live), tab(tab),
    begin(begin), end(end) {};

  tbb::task *execute() {
    // doing this in reverse might improve temporal locality
    for (uint i=begin;i<end;i++) {
      for (uint j=0;j<boxEdges[i].tri->membership_size;j++) {
        unsigned char l=boxEdges[i].tri->membership[j];
        if (boxEdges[i].edgeType == END) {
          tab[l].nB++;
        }
        if (boxEdges[i].edgeType == START) {
          tab[l].nA++;
        }
      }
    }
    return NULL;
  }

private:
  uint begin, end;
  const v_BoxEdge_inplace &boxEdges;
  const v_Triangle_aux &tris;
  const vp_KdTreeNode_inplace *const live;
  PrescanTab *tab;
};

#endif // _FINDBESTPLANE_AOS_PRESCAN_TASK_H_
