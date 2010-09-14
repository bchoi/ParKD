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

#ifndef _BOXEDGE_H_
#define _BOXEDGE_H_

#include <iostream>

class BoxEdge {
public:
  float t;
  unsigned int triangleIndex;
  bool edgeType;
  char axis;
  
  BoxEdge() {}

  BoxEdge(float f, unsigned int t, bool edgeType, char axis) :
    t(f), triangleIndex(t), edgeType(edgeType), axis(axis) {}

  bool operator<(const BoxEdge &RHS) const {
    if (this->t == RHS.t) {
      if (this->triangleIndex != RHS.triangleIndex) {
        return this->triangleIndex < RHS.triangleIndex;
      } else { // at this point we have two edges with equal t and index
        // resolve by comparing edge type
        return (int)(this->edgeType) < (int)(RHS.edgeType);
      }
    } else {
      return this->t < RHS.t;
    }
  }

  friend std::ostream &operator<<(std::ostream &o, const BoxEdge &edge);
  friend std::ostream &operator<<(std::ostream &o, const BoxEdge *edge);
};

namespace std {
  template<>
    class less<BoxEdge> {
  public:
    bool operator()(const BoxEdge& x, const BoxEdge& y) const {
      if (x.t == y.t) {
        if (x.triangleIndex != y.triangleIndex) {
          return x.triangleIndex < y.triangleIndex;
        } else {
          return (int)x.edgeType < (int)y.edgeType;
        }
      } else {
        return x.t < y.t;
      }
    }

    bool operator()(const BoxEdge* x, const BoxEdge* y) const {
      if (x->t == y->t) {
        if (x->triangleIndex != y->triangleIndex) {
          return x->triangleIndex < y->triangleIndex;
        } else {
          return (int)x->edgeType < (int)y->edgeType;
        }
      } else {
        return x->t < y->t;
      }
    }
  };
}

#endif // _BOXEDGE_H_
