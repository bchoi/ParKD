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

#include <limits>

#include "BoundingBox.h"

using namespace std;

BoundingBox::BoundingBox() {
  reset();
}

BoundingBox::BoundingBox(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
  min[0] = xMin;
  max[0] = xMax;
  min[1] = yMin;
  max[1] = yMax;
  min[2] = zMin;
  max[2] = zMax;
}
    
BoundingBox &BoundingBox::operator+=(const BoundingBox & RHS) {
  for(int i = 0; i < 3; i++) {
    if (RHS.min[i] < this->min[i]) this->min[i] = RHS.min[i];
    if (RHS.max[i] > this->max[i]) this->max[i] = RHS.max[i];
  }
  return *this;
}

const BoundingBox &BoundingBox::operator+(const BoundingBox & RHS) {
  return BoundingBox(*this) += RHS;
}

void BoundingBox::reset() {
  min[0] = min[1] = min[2] = numeric_limits<float>::max();
  max[0] = max[1] = max[2] = -numeric_limits<float>::max();
}

void BoundingBox::print(std::ostream &out) const {
  out <<   "[" << min[0] << "," << max[0]
      << "]x[" << min[1] << "," << max[1]
      << "]x[" << min[2] << "," << max[2] << "]";
}

void BoundingBox::serialize(std::ostream &out) const {
  // out << min[0] << min[1] << min[2];
  // out << max[0] << max[1] << max[2];
  out.write((char*)min, sizeof(float)*3);
  out.write((char*)max, sizeof(float)*3);
}

void BoundingBox::deserialize(std::istream &in) {
  // in >> min[0] >> min[1] >> min[2];
  // in >> max[0] >> max[1] >> max[2];
  in.read((char*)min, sizeof(float)*3);
  in.read((char*)max, sizeof(float)*3);
}
