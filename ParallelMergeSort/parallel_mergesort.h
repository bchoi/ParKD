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

#ifndef _PARALLEL_MERGESORT_H_
#define _PARALLEL_MERGESORT_H_

#include <tbb/task.h>

#include "MergeSort.h"

template<typename RandomAccessIterator, typename Compare>
void sequentialMerge(RandomAccessIterator begin1,
					RandomAccessIterator end1,
					RandomAccessIterator begin2,
					RandomAccessIterator end2,
					RandomAccessIterator begin3,
					const Compare& cmp) {
	int a = 0;
	int aFence = end1 - begin1;
	int b = 0;
	int bFence = end2 - begin2;
	int k = 0;

	while (a < aFence && b < bFence) {
		// cmp call
		if (cmp(*(begin1+a), *(begin2+b)))
			begin3[k++] = begin1[a++];
		else
			begin3[k++] = begin2[b++];
	}

	while (a < aFence) begin3[k++] = begin1[a++];
	while (b < bFence) begin3[k++] = begin2[b++];
}

template<typename RandomAccessIterator, typename Compare>
void parallel_mergesort(RandomAccessIterator begin, 
                        RandomAccessIterator end, 
                        RandomAccessIterator begin2, 
                        RandomAccessIterator end2, const Compare& comp) {
  if (end>begin) {
    
    if (end - begin < MIN_SIZE) {
      std::sort(begin, end, comp);
    }
    else {
      MergeSort<RandomAccessIterator, Compare>& sort = 
        *new(tbb::task::allocate_root()) 
        MergeSort<RandomAccessIterator, Compare>(begin, end, begin2, end2, comp);
      tbb::task::spawn_root_and_wait(sort);
    }
  }
}

// external interfaces
template<typename T>
inline void parallel_mergesort(T* begin, T* end, T* begin2, T* end2) {
  parallel_mergesort(begin, end, begin2, end2, std::less< T >());
}

template<typename RandomAccessIterator>
inline void parallel_mergesort(RandomAccessIterator begin, RandomAccessIterator end, RandomAccessIterator begin2, RandomAccessIterator end2) {
  parallel_mergesort(begin, end, begin2, end2, 
                     std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>());
}

#endif // _PARALLEL_MERGESORT_H_
