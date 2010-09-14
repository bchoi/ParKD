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

#include "SAH.h"

SAH::SAH(float Ct, float Ci, float emptyBonus) 
  : m_Ct(Ct), m_Ci(Ci), m_emptyBonus(emptyBonus) {}

float SAH::calculateSAH(const BoundingBox &nodeExtent, char axis,
                        uint nA, uint nB, float position) const {
  if (position < nodeExtent.min[axis] ||
      position > nodeExtent.max[axis]) {
    // outside the bounding box
    return std::numeric_limits<float>::max();
  } else {
    float delta[3] = { nodeExtent.max[0] - nodeExtent.min[0],
                       nodeExtent.max[1] - nodeExtent.min[1],
                       nodeExtent.max[2] - nodeExtent.min[2] };
    float oneOverTotalSurfaceArea = 0.5f /
      (delta[0]*delta[1] + delta[1]*delta[2] + delta[2]*delta[0]);
    int otherAxis[2] = { (axis+1)%3 , (axis+2)%3 };
    float crossSectionArea = delta[otherAxis[0]]*delta[otherAxis[1]];
    
    if (nA == 0 && position > nodeExtent.min[axis]) {
      float pB = 2*(crossSectionArea + (nodeExtent.max[axis] -
                                        position) *
                    (delta[otherAxis[0]] +
                     delta[otherAxis[1]]))
        * oneOverTotalSurfaceArea;
      return this->m_Ct + this->m_Ci*pB*nB*(1.0f-this->m_emptyBonus);
    } else if (nB == 0 && position < nodeExtent.max[axis]) {
      float pA = 2*(crossSectionArea +
                    (position - nodeExtent.min[axis])*
                    (delta[otherAxis[0]] + delta[otherAxis[1]]))*
        oneOverTotalSurfaceArea;
      return this->m_Ct + this->m_Ci*pA*(nA+nB)*(1.0f-this->m_emptyBonus);
    } else {
      float pA = 2*(crossSectionArea +
                    (position - nodeExtent.min[axis])*
                    (delta[otherAxis[0]] + delta[otherAxis[1]]))*
        oneOverTotalSurfaceArea;
      float pB = 2*(crossSectionArea +
                    (nodeExtent.max[axis] - position)*
                    (delta[otherAxis[0]] + delta[otherAxis[1]]))*
        oneOverTotalSurfaceArea;
      return this->m_Ct + this->m_Ci*(pA*nA + pB*nB);
    }
  }
}

float SAH::operator()(const BoundingBox &nodeExtent, char axis,
                      uint nA, uint nB, float position) const {
  return calculateSAH(nodeExtent, axis, nA, nB, position);
}
