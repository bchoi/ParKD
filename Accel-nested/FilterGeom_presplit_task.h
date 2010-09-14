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

#ifndef PARPRESPLIT_TASK_H_
#define PARPRESPLIT_TASK_H_

#include <vector>

#include <tbb/task.h>

#include "common.h"
#include "PresplitTab.h"

/**
 * parallel pre-split of boxedges
 * count the # of boxedges that will go into left and right respectively
 * and store them in the local table (will be updated to the global value later)
 */
class FilterGeom_presplit_task : public tbb::task {

public:
  FilterGeom_presplit_task(const mem_type &membership,
                           const v_BoxEdge &boxEdges,
                           PresplitTab &tab,
                           size_t begin,
                           size_t end) :
    membership(membership), boxEdges(boxEdges), 
    split_tab(tab), begin(begin), end(end) {};

  tbb::task* execute() {
    for (size_t i=begin;i<end;i++) {
      // split the range into left and right
      if (membership[boxEdges[i].triangleIndex*2]) split_tab.leftBoxedges++;
      if (membership[boxEdges[i].triangleIndex*2+1]) split_tab.rightBoxedges++;
    }
    
    return NULL;
  }
  
private:
  const mem_type &membership;
  const v_BoxEdge &boxEdges;
  PresplitTab &split_tab;
  size_t begin, end;
};

#endif /* PA2RPRESPLIT_TASK_H_ */
