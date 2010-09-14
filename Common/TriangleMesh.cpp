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

#include <vector>
#include <math.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "Triangle.h"
#include "TriangleMesh.h"

using namespace std;

void TriangleMesh::addTriangles(const string & filename) {
  ifstream in(filename.c_str());
  if (in.is_open()) {
    addTriangles(in);
  }
}

void TriangleMesh::addTriangles(istream &in) {
  Triangle tri;
  BoundingBox triBound;

  int vcnt_orig = vertexList.size();

  while (!in.eof()) {
    string first;
    in >> first;
    
    if (first == "v") {
      float trif[3];
      in >> trif[0] >> trif[1] >> trif[2];
      vertexList.push_back(Vec3f(trif[0],trif[1],trif[2]));
      //           cout << "v " << trif[0] << " " << trif[1] << " " << trif[2] << " " << endl;
    } else if (first == "f") {
      int trii[3];
      
      // hackish parsing routine for the following regex
      // [1-0]+[/]?.*[/]?.* [1-0]+[/]?.*[/]?.* [1-0]+[/]?.*[/]?.*
      int idx;
      char buf[128];
      for (int i=0;i<3;i++) {
        idx = 0;
        memset(&buf, 0, sizeof(buf));
        
        // skip preceding white spaces
        while(!in.eof() && in.peek() == ' ') {
          in.seekg(1, ios::cur);
        }
        
        // read digits until you hit / or space or a newline
        while(!in.eof() && in.peek() != '/' && in.peek() != ' ' 
              && in.peek() != '\n' && in.peek() != '\r') { // <-- windows!
          in.get(buf[idx++]);
        }
        trii[i] = atoi(buf)-1;
        
        // skip the rest until you see a space or a newline
        while(!in.eof() && in.peek() != ' ' && in.peek() != '\n' 
              && in.peek() != '\r') {
          in.seekg(1, ios::cur);
        }
      }
      
      // skip the rest until you see a newline
      while(!in.eof() && in.peek() != '\n' && in.peek() != '\r') {
        in.seekg(1, ios::cur);
      }
      
      tri = Triangle(vertexList[trii[0]+vcnt_orig], 
                     vertexList[trii[1]+vcnt_orig], 
                     vertexList[trii[2]+vcnt_orig]);
      this->triangleList.push_back(tri);
      this->boundingBox += tri.bound;
    } else {
      continue;
    }
  }
  
  return;
}

void TriangleMesh::serialize(std::ostream &out) const {
  int size = triangleList.size();
  out.write((char*)&size, sizeof(int));

  // Relying on STL vector's guarantee on data layout (continuity)
  out.write((char*)&triangleList[0], sizeof(triangleList[0])*size);

  boundingBox.serialize(out);
}

void TriangleMesh::deserialize(std::istream &in) {
  int size;
  in.read((char*)&size, sizeof(int));
  triangleList.resize(size);

  // Relying on STL vector's guarantee on data layout (continuity)
  in.read((char*)&triangleList[0], sizeof(triangleList[0])*size);

  boundingBox.deserialize(in);
}
