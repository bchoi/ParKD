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

#ifndef _SAH_H_
#define _SAH_H_

#include "common.h"
#include "BoundingBox.h"

#define Ct_DEFAULT 15.0f
#define Ci_DEFAULT 20.0f
#define emptyBonus_DEFAULT 0.0f

class SAH {
public:
  SAH(float Ct = Ct_DEFAULT,
      float Ci = Ci_DEFAULT,
      float emptyBonus = emptyBonus_DEFAULT);

  float calculateSAH(const BoundingBox &nodeExtent, char axis,
                     uint nA, uint nB, float position) const;

  // convenient function
  float operator()(const BoundingBox &nodeExtent, char axis,
                   uint nA, uint nB, float position) const;

  // parameters in SAH formula
  const float m_Ci;          // triangle intersection cost
  const float m_Ct;	         // node traversal cost
  const float m_emptyBonus;  // takes value between 0 and 1
                             // closer to 1 => encourage empty space splitting
                             // closer to 0 => discourage
};

#endif // _SAH_H_
