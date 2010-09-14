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

#ifndef PARSAH_TASK_H_
#define PARSAH_TASK_H_

#include <limits>

#include <tbb/task.h>

#include "PrescanTab.h"
#include "BoundingBox.h"

// parallel SAH
class FindBestPlane_task : public tbb::task {
public:
  FindBestPlane_task(const v_BoxEdge &boxEdge, PrescanTab &tab, 
                     const BoundingBox& nodeExtent, KdTreeAccel* accel, 
                     size_t axis, size_t begin, size_t end, 
                     unsigned int triangles) :
    boxEdge(boxEdge), prescan_tab(tab), axis(axis), begin(begin), end(end), 
    accel(accel), nodeExtent(nodeExtent), triangles(triangles) {};

    tbb::task *execute() {
      prescan_tab.SAH = std::numeric_limits<float>::max();
      
      // update nB when seeing an END edge (local)
      for (size_t i=begin;i<end;i++) {
        if (boxEdge[i].edgeType == END) {
          prescan_tab.nB++;
        }
        
        // SAH computation
        float SAH = accel->sah(nodeExtent, axis, prescan_tab.nA, 
                               triangles - prescan_tab.nB, boxEdge[i].t);
        
		// updating the local best SAH
        if (SAH < prescan_tab.SAH) {
          prescan_tab.SAH = SAH;
          prescan_tab.bestEdge = &boxEdge[i];
          prescan_tab.bestIndex = i;
        }
        
        // update nB when seeing a START edge (local)
        if (boxEdge[i].edgeType == START) {
          prescan_tab.nA++;
        }
      }
      return NULL;
    }
  
private:
  const size_t axis, begin, end, triangles;
  const v_BoxEdge &boxEdge;
  PrescanTab &prescan_tab;
  const KdTreeAccel* accel;
  const BoundingBox& nodeExtent;
};

#endif /* ParSAH_task_H_ */
