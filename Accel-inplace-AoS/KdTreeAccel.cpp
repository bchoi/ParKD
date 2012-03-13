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

#include <iomanip>
#include <stdio.h>

#include <tbb/task_scheduler_init.h>
#include <tbb/partitioner.h>
#include <tbb/parallel_for.h>

#include "KdTreeAccel.h"
#include "CreateEdges_task.h"
#include "SetupTriangles_task.h"
#include "PrescanTab.h"
#include "FindBestPlane_AoS_prescan_task.h"
#include "FindBestPlane_AoS_task.h"
#include "Split_task.h"
#include "parallel_mergesort.h"
#include "timers.h"
#include "options.h"

using namespace std;
using namespace tbb;


void impl_usage() {
  string impl_usage[] = {
    "* In-place (AoS) Options: ",
    "==="
  };

  int i=0;
  while(impl_usage[i] != "===") {
    cout << impl_usage[i++] << endl;
  }
}

bool ver_splitedge;

std::string KdTreeAccel::impl_string() {
  return string("In-place (AoS)");
}

KdTreeAccel::KdTreeAccel(TriangleMesh *mesh, 
                         uint numThreads, uint maxDepth)
  : KdTreeAccel_base(mesh, numThreads, maxDepth), stats(maxDepth) { }

void KdTreeAccel::build() {
  RECORD_TIME(
    stats.start_usec,
    stats.start);

  // init task scheduler
  task_scheduler_init init(m_numThreads);
  uint n = m_mesh->triangleList.size(); // number of triangles
  // x, y, z edges concatenated
  xbegin_idx = 0;
  xend_idx = xbegin_idx + 2*n;
  ybegin_idx = xend_idx;
  yend_idx = ybegin_idx + 2*n;
  zbegin_idx = yend_idx;
  zend_idx = zbegin_idx + 2*n;
  v_Triangle_aux &tris = *new v_Triangle_aux(n);

  // build consecutive array of kdTreeNode/s -- assume full-tree
  kdTreeNodeObj = new v_KdTreeNode_inplace(1<<(m_maxDepth+1));
  // root_ is the first one
  root_ = &((*kdTreeNodeObj)[0]);

  // all triangles
  root_->triangleCount = m_mesh->triangleList.size();
  root_->extent = m_mesh->boundingBox;
  root_->triangleIndices = new vector<int>();

  proxy.resize(6*n); // sort proxy.. sort this to find out right index
                            // for boxEdge/s (unpacked) -- index into tab/s

  parallel_for(blocked_range<size_t>(xbegin_idx, xbegin_idx+n),
               CreateEdges_task(proxy, tris, root_, m_mesh->triangleList, 0, 0),
               auto_partitioner());
  parallel_for(blocked_range<size_t>(ybegin_idx, ybegin_idx+n),
               CreateEdges_task(proxy, tris, root_, m_mesh->triangleList, 1, 2*n),
               auto_partitioner());
  parallel_for(blocked_range<size_t>(zbegin_idx, zbegin_idx+n),
               CreateEdges_task(proxy, tris, root_, m_mesh->triangleList, 2, 4*n),
               auto_partitioner());

  RECORD_TIME(
    stats.init_CreateEdges_usec,
    stats.init_CreateEdges);

  // sort + tri setup on x,y, and z
  v_BoxEdge_inplace scratch(2*n);
  parallel_mergesort(proxy.begin(), proxy.begin()+2*n,
                     scratch.begin(), scratch.end());
  parallel_mergesort(proxy.begin()+4*n, proxy.begin()+6*n,
                     scratch.begin(), scratch.end());
  parallel_mergesort(proxy.begin()+2*n, proxy.begin()+4*n,
                     scratch.begin(), scratch.end());

  RECORD_TIME(
    stats.init_sort_usec,
    stats.init_sort);

  parallel_for(blocked_range<size_t>(xbegin_idx, xend_idx),
               SetupTriangles_task(proxy), auto_partitioner());
  parallel_for(blocked_range<size_t>(ybegin_idx, yend_idx),
               SetupTriangles_task(proxy), auto_partitioner());
  parallel_for(blocked_range<size_t>(zbegin_idx, zend_idx),
               SetupTriangles_task(proxy), auto_partitioner());

  RECORD_TIME(
    stats.init_SetupTriangles_usec,
    stats.init_SetupTriangles);
  if (g_time_in_ticks) {
    stats.init_finish = stats.init_SetupTriangles;
  } else {
    stats.init_finish_usec = stats.init_SetupTriangles_usec;
  }

  if (m_numThreads > 1) {
    pRootTask = new(task::allocate_root()) empty_task;
    parallel_build(proxy, tris, m_maxDepth); // w/ unpacked objects
  } else {
    sequential_build(proxy, tris, m_maxDepth);
  }
}

void KdTreeAccel::printTimingStats(ostream &out) {
  if (g_time_in_ticks) {
    out << "     In-place (AoS) TIMING INFORMATION (in CPU ticks)\n\n";
  
    char buf[128];
  
    out << setw(34) << left << "Start time" << ": " 
        << setw(20) << right << stats.start << "\n";
  
    sprintf(buf, "+%lu", stats.init_CreateEdges - stats.start);
    out << setw(34) << left << "Init (CreateEdges)" << ": " 
        << setw(20) << right << buf << "\n";
  
    memset(buf, 0, 128);
    sprintf(buf, "+%lu", stats.init_sort - stats.init_CreateEdges);
    out << setw(34) << left << "Init (parallel_sort)" << ": " 
        << setw(20) << right << buf << "\n";
  
    memset(buf, 0, 128);
    sprintf(buf, "+%lu", stats.init_SetupTriangles - stats.init_sort);
    out << setw(34) << left << "Init (SetupTriangles)" << ": " 
        << setw(20) << right << buf << "\n";
  
    memset(buf, 0, 128);
    sprintf(buf, "+%lu", stats.init_finish - stats.start);
    out << setw(34) << left << "Init (Total)" << ": " 
        << setw(20) << right << buf << "\n";
  
    out << "\n";
  
    out << setw(34) << left << "Build start time" << ": "
        << setw(20) << right << stats.build_start << "\n";
  
    // Average over all iterations
    uint64 total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.findBestPlane[i][1] - stats.findBestPlane[i][0];
    }
    out << setw(34) << left << "FindBestPlane time (Avg)" << ": "
        << setw(20) << right << total/m_maxDepth << "\n";
  
    total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.newGen[i][1] - stats.newGen[i][0];
    }
    out << setw(34) << left << "NewGen time (Avg)" << ": " 
        << setw(20) << right << total/m_maxDepth << "\n";
  
    total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.classifyTriangles[i][1] - stats.classifyTriangles[i][0];
    }
    out << setw(34) << left << "ClassifyTriangles time (Avg)" << ": " 
        << setw(20) << right << total/m_maxDepth << "\n";
  
    out << setw(34) << left << "Fill time" << ": " 
        << setw(20) << right << stats.fill[1] - stats.fill[0] << "\n";
  
    // Overall
    memset(buf, 0, 128);
    sprintf(buf, "%lu", stats.build_finish - stats.start);
    out << setw(34) << left << "Total build time" << ": " 
        << setw(20) << right << buf << "\n";
  
    memset(buf, 0, 128);
    sprintf(buf, "%lu", stats.build_finish - stats.build_start);
    out << setw(34) << left << "Total build time (w/o Init)" << ": " 
        << setw(20) << right << buf << "\n";
  } else {
    out << "     In-place (SoA) TIMING INFORMATION (in microseconds)\n\n";

    long int dt;

    dt = stats.init_CreateEdges_usec - stats.start_usec;
    out << setw(34) << left << "Init (CreateEdges)" << ": " 
        << setw(20) << right << dt << "\n";
  
    dt = stats.init_sort_usec - stats.init_CreateEdges_usec;
    out << setw(34) << left << "Init (parallel_sort)" << ": " 
        << setw(20) << right << dt << "\n";
  
    dt = stats.init_SetupTriangles_usec - stats.init_sort_usec;
    out << setw(34) << left << "Init (SetupTriangles)" << ": " 
        << setw(20) << right << dt << "\n";
  
    dt = stats.init_finish_usec - stats.start_usec;
    out << setw(34) << left << "Init (Total)" << ": " 
        << setw(20) << right << dt << "\n";
  
    out << "\n";
  
    // Average over all iterations
    uint64 total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.findBestPlane_usec[i][1] - stats.findBestPlane_usec[i][0];
    }
    out << setw(34) << left << "FindBestPlane time (Avg)" << ": "
        << setw(20) << right << total/m_maxDepth << "\n";
  
    total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.newGen_usec[i][1] - stats.newGen_usec[i][0];
    }
    out << setw(34) << left << "NewGen time (Avg)" << ": " 
        << setw(20) << right << total/m_maxDepth << "\n";
  
    total = 0L;
    for (uint i=0;i<m_maxDepth;i++) {
      total += stats.classifyTriangles_usec[i][1] - 
        stats.classifyTriangles_usec[i][0];
    }
    out << setw(34) << left << "ClassifyTriangles time (Avg)" << ": " 
        << setw(20) << right << total/m_maxDepth << "\n";
  
    out << setw(34) << left << "Fill time" << ": " 
        << setw(20) << right << stats.fill_usec[1] - stats.fill_usec[0] << "\n";
  
    // Overall
    dt = stats.build_finish_usec - stats.start_usec;
    out << setw(34) << left << "Total build time" << ": " 
        << setw(20) << right << dt << "\n";
  
    dt = stats.build_finish_usec - stats.build_start_usec;
    out << setw(34) << left << "Total build time (w/o Init)" << ": " 
        << setw(20) << right << dt << "\n";

  }
}

void KdTreeAccel::printTimingStatsCSVHeader(std::ostream &out) {
  stats.printCSVHeader(out);
  cerr << "\n";
}

void KdTreeAccel::printTimingStatsCSV(std::ostream &out) {
  stats.printCSV(out);
  cerr << "\n";
}
