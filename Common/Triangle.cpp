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

#include "TriangleMesh.h"
#include "Triangle.h"
#include "Vec3f.h"

Triangle::Triangle() { 
  Vec3f v;
  vertex[0] = v;
  vertex[1] = v;
  vertex[2] = v;
  compute_bound();
}

Triangle::Triangle(Vec3f &v0, Vec3f &v1, Vec3f &v2) {
  vertex[0] = v0;
  vertex[1] = v1;
  vertex[2] = v2;
  compute_bound();
}

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MAX3(a, b, c) MAX( MAX(a ,b) ,c)
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MIN3(a, b, c) MIN( MIN(a, b) ,c)

void Triangle::compute_bound() {
  bound.max[0] = MAX3(vertex[0][0], vertex[1][0], vertex[2][0]);
  bound.min[0] = MIN3(vertex[0][0], vertex[1][0], vertex[2][0]);

  bound.max[1] = MAX3(vertex[0][1], vertex[1][1], vertex[2][1]);
  bound.min[1] = MIN3(vertex[0][1], vertex[1][1], vertex[2][1]);

  bound.max[2] = MAX3(vertex[0][2], vertex[1][2], vertex[2][2]);
  bound.min[2] = MIN3(vertex[0][2], vertex[1][2], vertex[2][2]);
}

void Triangle::serialize(std::ostream &out) const {
  bound.serialize(out);
}

void Triangle::deserialize(std::istream &in) {
  bound.deserialize(in);
}
