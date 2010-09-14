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

#ifndef _STATS_H_
#define _STATS_H_

#include <ostream>

#include "Stats_base.h"
#include "common.h"

class Stats : public Stats_base {
public:
  Stats(uint maxDepth);
  ~Stats();

  // Mandatory functions
  void printCSVHeader(std::ostream &out);
  void printCSV(std::ostream &out);

  uint64 start, init_CreateEdges, init_sort;
  uint64 init_finish, build_start, build_finish;

  long int start_usec, init_CreateEdges_usec, init_sort_usec;
  long int init_finish_usec, build_start_usec, build_finish_usec;

  // measure only for the first level of the tree
  // [Start/End]
  uint64 findBestPlane[2], classifyTriangles[2], filterGeom[2], recursiveTaskCreation[2];

  long int findBestPlane_usec[2], classifyTriangles_usec[2], 
    filterGeom_usec[2], recursiveTaskCreation_usec[2];
};

#endif // _STATS_H_
