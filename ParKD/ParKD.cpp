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

// ParKD.cpp : Defines the entry point for the console application.
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <iterator>
#include <cstdlib>

#include <tbb/task_scheduler_init.h>
#include <tbb/tick_count.h>

#include "TriangleMesh.h"

// Interface header -- every implementation has this file!
#include "KdTreeAccel.h"

#include "timers.h"

using namespace std;

// This needs to be defined by each implementation
extern void impl_usage();

void usage() {
  string usage[] = {
    "Usage: ./parkd [Options] [input mesh]",
    "Build point-based KDTree using the given mesh in obj file format.",
    "",
    "* General Options: ",
    "",
    "   -h              help",
    "   -n <n>          # of threads to use (>0)",
    "   -o              Print the constructed tree (ascii)",
    "   --to            Print the constructed tree (Manta format)",
//     "  -v              Verbose mode -- debug info",
//     "  -v:level0       profile level0 construction",
    "   --csv-header    Print the csv file header and exit",
    "   --csv           Status output in csv format",
    "   -m              Maximum height (>0) of the KDTree (default = 8)",
    "   --graphviz      Print constructed tree in GraphViz-compatible .dot file",
    "   -q              Quiet",
    "   --seconds       Measure time in seconds",
    "   --rdtsc         Measure time in ticks using rdtsc instruction",
    "   --superfluous-prescans",
    "                   Performs pre-scan phase even with just a single thread",
    "",
//     "EXAMPLES:",
//     "  ./fast -n 16 --tbb teapot.obj",
//     "       Build the kdtree for teapot.obj with 16 TBB threads",
//     "",
//     "  ./fast -n 4 -o teapot.obj",
//     "       Build the kdtree for teapot.obj with 4 pthreads",
//     "       and output the result",
//     "",
//     "  ./fast -r -rn 10 --seq cornell_leftWall.obj cornell_centerWall.obj ",
//     "         cornell_rightWall.obj cornell_ceiling.obj cornell_floor.obj",
//     "         cornell_light.obj teapot.obj -z 1.1",
//     "       Produce raytrace image using 7 .obj files with 10 threads.",
//     "       teapot.obj is made 10% bigger",
//     "",
//     "NOTE:",
//     "   When using pthread as threading model, 2^n number of threads are",
//     "   used internally: maximum number of such that does not exceed ",
//     "   the given limit.",
    "==="
  };

  int i=0;
  while(usage[i] != "===") {
      cout << usage[i++] << endl;
  }

  impl_usage();
  exit(0);
}

// Global variables 
bool g_time_in_ticks;
bool g_superfluous_prescans;
bool g_verbose;

int main(int argc, char *argv[]) {
    // Load input mesh
    TriangleMesh * myMesh = new TriangleMesh();

    // Process input
    unsigned int nthreads = 1; 
    unsigned int maxdepth = 8; // maximum tree depth
    bool output = false, csv = false,
      csv_header = false, graphviz = false, 
      graphvizAccm = false, treeout = false,
      quiet = false;
    vector<std::string> input;

    g_time_in_ticks = false;
    g_superfluous_prescans = false;
    g_verbose = false;

    for (unsigned int i=1;i<argc;i++) {
      if (!strcmp(argv[i], "-h")) {
        usage();
      } else if (!strcmp(argv[i], "-n")) {
        i++;
        if (argc <= i) { usage(); }
        else {
          nthreads = atoi(argv[i]);
          if (nthreads < 1) {
            usage();
          }
        }
      } else if (!strcmp(argv[i], "-o")) {
        output = true;
      } else if (!strcmp(argv[i], "--to")) {
        treeout = true;
      } else if (!strcmp(argv[i], "-v")) {
        g_verbose = true;
      } else if (!strcmp(argv[i], "-q")) {
        quiet = true;
      } else if (!strcmp(argv[i], "--csv")) {
        csv = true;
      } else if (!strcmp(argv[i], "--csv-header")) {
        csv_header = true;
        csv = true;
      } else if (!strcmp(argv[i], "-m")) {
        i++;
        if (argc <= i) { usage(); }
        else {
          maxdepth = atoi(argv[i]);
          if (maxdepth < 1) {
            usage();
          }
        }
      } else if (!strcmp(argv[i], "--graphviz")) {
        graphviz = true;
      } else if (!strcmp(argv[i], "--graphvizAccm")) {
        graphvizAccm = true;
      } else if (!strcmp(argv[i], "--seconds")) {
        g_time_in_ticks = false;
      } else if (!strcmp(argv[i], "--rdtsc")) {
        g_time_in_ticks = true;
      } else if (!strcmp(argv[i], "--superfluous-prescans")) {
        g_superfluous_prescans = true;
      } else {
        if (argv[i][0] == '-') {
          cerr << "Unknown option : " << argv[i] << endl;
        } else {
          char *file = argv[i];
          input.push_back(std::string(file));
        }
      }
    }

    // Sanity check
    if (!csv_header && input.size() < 1) {
      if (quiet) {
        exit(-1);
      } else {
        usage();
      }
    }

    if (g_superfluous_prescans && nthreads > 1) {
      cerr << "--superfluous-prescans option is meaningless in non-sequential runs\n\n";
    }

    KdTreeAccel *myAccel = new KdTreeAccel(myMesh, nthreads, maxdepth);

    if (csv_header) {
      cerr << "Threads,Start time,Mesh load finish time,Build start time,Build finish time";
      myAccel->printTimingStatsCSVHeader(cerr);
      exit(0);
    }

    // Print headers here
    if (!quiet && !csv) {
      cerr << "ParKD - " << myAccel->impl_string() << "\n\n";
      string indent("    ");
      cerr << indent << setw(24) << "Input mesh" << " : ";
      copy(input.begin(), input.end(), ostream_iterator<std::string>(cerr, " "));
      cerr << "\n"
           << indent << setw(24) << " Threads" << " : " << nthreads << "\n"
           << indent << setw(24) << " MaxDepth" << " : " << maxdepth << "\n"
           << indent << setw(24) << " Superfluous Prescans" << " : " << (g_superfluous_prescans?"Yes":"no")
           << "\n\n";
    }

    // Process the input mesh
    if (g_verbose) {
      cerr << "==> Processing input mesh\n";
    }

    uint64 start_tick;
    long int start_usec;
    RECORD_TIME(start_usec, start_tick);

    for (unsigned int i=0;i<input.size();i++) {
      if (g_verbose) {
        cerr << "    " << input[i] << "\n";
      }
      string &file = input[i];
      string suffix_obj(".obj");
      if (file.substr(file.size() - string(".obj").size()) == ".obj") {
        myMesh->addTriangles(file);
      } else if (file.substr(file.size() - string(".blob").size()) == ".blob") {
        ifstream ifs(input[i].c_str());
        myMesh->deserialize(ifs);
      } else {
        cerr << "Unknown file format: " << file << "\n";
        exit(-1);
      }
    }

    if (g_verbose) {
      cerr << "\n";
    }

    uint64 mesh_finish_tick;
    long int mesh_finish_usec;
    RECORD_TIME(mesh_finish_usec, mesh_finish_tick);

    uint64 build_start_tick;
    long int build_start_usec;
    RECORD_TIME(build_start_usec, build_start_tick);

    // Entry point
    myAccel->build();

    uint64 build_finish_tick;
    long int build_finish_usec;
    RECORD_TIME(build_finish_usec, build_finish_tick);

    if (!quiet && !csv) {
      if (g_time_in_ticks) {
        cerr << "              TIMING INFORMATION (in CPU ticks)\n\n";
        
        cerr << setw(34) << left << "Start time" << ": " << setw(20) << right << start_tick << "\n"
             << setw(34) << left << "Mesh load finish time" << ": ";
        
        char buf[128];
        sprintf(buf, "+%lu", mesh_finish_tick - start_tick);
        cerr << setw(20) << right << buf << "\n";
        
        cerr << setw(34) << left << "Build start time" << ": " 
             << setw(20) << right << build_start_tick << "\n";
        
        memset(buf, 0, 128);
        sprintf(buf, "+%lu", build_finish_tick - build_start_tick);
        cerr << setw(34) << left << "Build finish time" << ": " 
             << setw(20) << right << buf << "\n"
             << "\n";
        
        myAccel->printTimingStats(cerr);
      }
      else {
        cerr << "              TIMING INFORMATION (in microseconds)\n\n"
             << setw(34) << left << "Mesh load time" << ": "
             << setw(20) << right << (mesh_finish_usec - start_usec) << "\n";
        
        cerr << setw(34) << left << "Build time" << ": " 
             << setw(20) << right << (build_finish_usec - build_start_usec) << "\n\n";
        
        myAccel->printTimingStats(cerr);
      } 
    } else if (csv) {
      if (g_time_in_ticks) {
        cerr << nthreads << ","
             << start_tick << ","
             << mesh_finish_tick << ","
             << build_start_tick << ","
             << build_finish_tick;
        myAccel->printTimingStatsCSV(cerr);
      }
      else {
        cerr << nthreads << ","
             << start_usec << ","
             << mesh_finish_usec << ","
             << build_start_usec << ","
             << build_finish_usec;
        myAccel->printTimingStatsCSV(cerr);
      }
    }
    
    if (output) {
      myAccel->printTree();
    }
    
    // write tree to file
    if (treeout) {
      cerr << "Writing out the tree (for Manta)" << endl;
      myAccel->writeToFile("kdtree.binary");
    }
    
    if (graphviz) {
      myAccel->printGraphviz();
    } else if (graphvizAccm) {
      myAccel->printGraphvizAccm();
    }
    
    delete myAccel;

	return 0;
}
