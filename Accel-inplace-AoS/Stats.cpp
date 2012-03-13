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
  memset(findBestPlane, 0, sizeof(uint64)*32*2);
  memset(newGen, 0, sizeof(uint64)*32*2);
  memset(classifyTriangles, 0, sizeof(uint64)*32*2);
  memset(fill, 0, sizeof(uint64)*2);
}

Stats::~Stats() { }

void Stats::printCSVHeader(std::ostream &out) {
  cerr << ",start,init_CreateEdges,init_sort,init_SetupTriangles"
       << ",init_finish,build_start,build_finish";
  
  // FindBestPlane, NewGen, ClassifyTriangles per level
  for (uint i=0;i<m_maxDepth;i++) {
    cerr << ",findBestPlane_" << i << "_start"
         << ",findBestPlane_" << i << "_end";
  }

  for (uint i=0;i<m_maxDepth;i++) {
    cerr << ",newGen_" << i << "_start"
         << ",newGen_" << i << "_end";
  }

  for (uint i=0;i<m_maxDepth;i++) {
    cerr << ",classifyTriangles_" << i << "_start"
         << ",classifyTriangles_" << i << "_end";
  }

  // Fill phase (one-time)
  cerr << ",fill_start,fill_end";
}

void Stats::printCSV(std::ostream &out) {
  if (g_time_in_ticks) {
    cerr << "," << start
         << "," << init_CreateEdges
         << "," << init_sort
         << "," << init_SetupTriangles
         << "," << init_finish
         << "," << build_start
         << "," << build_finish;
    
    // FindBestPlane, NewGen, ClassifyTriangles per level
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << findBestPlane[i][0] 
           << "," << findBestPlane[i][1];
    }
  
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << newGen[i][0] 
           << "," << newGen[i][1];
    }
  
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << classifyTriangles[i][0] 
           << "," << classifyTriangles[i][1];
    }
  
    // Fill phase (one-time)
    cerr << "," << fill[0] << "," << fill[1];
  } else {
    cerr << "," << start_usec
         << "," << init_CreateEdges_usec
         << "," << init_sort_usec
         << "," << init_SetupTriangles_usec
         << "," << init_finish_usec
         << "," << build_start_usec
         << "," << build_finish_usec;
    
    // FindBestPlane, NewGen, ClassifyTriangles per level
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << findBestPlane_usec[i][0] 
           << "," << findBestPlane_usec[i][1];
    }
  
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << newGen_usec[i][0] 
           << "," << newGen_usec[i][1];
    }
  
    for (uint i=0;i<m_maxDepth;i++) {
      cerr << "," << classifyTriangles_usec[i][0] 
           << "," << classifyTriangles_usec[i][1];
    }
  
    // Fill phase (one-time)
    cerr << "," << fill_usec[0] << "," << fill_usec[1];
  }
}
