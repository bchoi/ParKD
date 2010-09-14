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

#ifndef _FINDBESTPLANE_TASK_H_
#define _FINDBESTPLANE_TASK_H_

#include <limits>

#include <tbb/task.h>

#include "PrescanTab.h"

class FindBestPlane_task : public tbb::task {
public:
  FindBestPlane_task(const TAB &table, const v_Triangle_aux &tris,
                     const vp_KdTreeNode_inplace *live, PrescanTab *tab, SplitMemo *memo,
                     uint axis, uint begin, uint end, KdTreeAccel *accel) :
    table(table), tris(tris), live(live), tab(tab), memo(memo), axis(axis),
    begin(begin), end(end), accel(accel) {};

  ~FindBestPlane_task() {};

  tbb::task *execute() {
    if (!tab) { // this means this is the first chunk -- no prescan needed
      tab = (PrescanTab*)scalable_calloc(live->size(), sizeof(PrescanTab));
    }
    // use tab as running tab
    for (uint l=0;l<live->size();l++) {
      memo[l].SAH = std::numeric_limits<float>::max();
    }
    
    for (uint i=begin;i<end;i++) {
      for (uint j=0;j<table.tri_tab[i]->membership_size;j++) {
        unsigned char l = table.tri_tab[i]->membership[j];
        if (table.edgeType_tab[i] == END) {
          tab[l].nB++;
        }
        float SAH = accel->sah((*live)[l]->extent, axis,
                               tab[l].nA, (*live)[l]->triangleCount-tab[l].nB,
                               table.t_tab[i]);
        if (SAH < memo[l].SAH) {
          memo[l].SAH = SAH;
          memo[l].nA = tab[l].nA;
          memo[l].nB = (*live)[l]->triangleCount-tab[l].nB;
          memo[l].split = i;
          memo[l].axis = axis;
        }
        if (table.edgeType_tab[i] == START) {
          tab[l].nA++;
        }
      }
    }

    return NULL;
  }

private:
  const uint axis, begin, end;
  const TAB &table;
  const v_Triangle_aux &tris;
  const vp_KdTreeNode_inplace *const live;
  PrescanTab *tab;
  SplitMemo *memo;
  const KdTreeAccel *accel;
};

#endif // _FINDBESTPLANE_TASK_H_
