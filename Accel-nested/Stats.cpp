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

#include <iostream>
#include <cstring>

#include "Stats.h"
#include "options.h"

using namespace std;

Stats::Stats(uint maxDepth) : Stats_base(maxDepth) {
  memset(findBestPlane, 0, sizeof(uint64)*2);
  memset(classifyTriangles, 0, sizeof(uint64)*2);
  memset(filterGeom, 0, sizeof(uint64)*2);
  memset(recursiveTaskCreation, 0, sizeof(uint64)*2);
}

Stats::~Stats() { }

void Stats::printCSVHeader(std::ostream &out) {
  cerr << ",start,init_CreateEdges,init_sort"
       << ",init_finish,build_start,build_finish";

  // FindBestPlane, NewGen, ClassifyTriangles for the root level
  cerr << ",findBestPlane_start"
        << ",findBestPlane_end";

  cerr << ",classifyTriangles_start"
	<< ",classifyTriangles_end";

  cerr << ",filterGeom_start"
	<< ",filterGeom_end";

  cerr << ",recursiveTaskCreation_start"
	<< ",recursiveTaskCreation_end";

}

void Stats::printCSV(std::ostream &out) {
  if (g_time_in_ticks) {
    cerr << "," << start
         << "," << init_CreateEdges
         << "," << init_sort
         << "," << init_finish
         << "," << build_start
         << "," << build_finish;

    cerr << "," << findBestPlane[0] << "," << findBestPlane[1];
    cerr << "," << classifyTriangles[0] << "," << classifyTriangles[1];
    cerr << "," << filterGeom[0] << "," << filterGeom[1];
    cerr << "," << recursiveTaskCreation[0] << "," << recursiveTaskCreation[1];
  } else {
    cerr << "," << start_usec
         << "," << init_CreateEdges_usec
         << "," << init_sort_usec
         << "," << init_finish_usec
         << "," << build_start_usec
         << "," << build_finish_usec;

    cerr << "," << findBestPlane_usec[0] << "," << findBestPlane_usec[1];
    cerr << "," << classifyTriangles_usec[0] << "," << classifyTriangles_usec[1];
    cerr << "," << filterGeom_usec[0] << "," << filterGeom_usec[1];
    cerr << "," << recursiveTaskCreation_usec[0] << "," << recursiveTaskCreation_usec[1];
  }
}
