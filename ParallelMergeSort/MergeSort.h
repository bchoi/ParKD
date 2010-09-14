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

#ifndef _MERGESORT_H_
#define _MERGESORT_H_

#include <tbb/task.h>

#include "ParallelMerge.h"
#include "config.h"

template<typename RandomAccessIterator, typename Compare>
class MergeSort : public tbb::task {

public:
	// first array (A)
	RandomAccessIterator begin1, end1;
	// second array (B) - to save the result
	RandomAccessIterator begin2, end2;
	const Compare& cmp;

	MergeSort(RandomAccessIterator begin_, 
              RandomAccessIterator end_, 
              RandomAccessIterator begin2_, 
              RandomAccessIterator end2_, const Compare& cmp_) : 
      begin1(begin_), end1(end_), begin2(begin2_), end2(end2_), cmp(cmp_) {}
	task* execute() {

		// actual size = (end-begin+1);
		int size = end1 - begin1;

		// parallel merge sort
		if (size <= MIN_SIZE) {
			std::sort(begin1, end1, cmp);
		}
		else {
			// get quarter size
			int q = size / 4;
			int* idxs = new int[4];
			idxs[0] = q;
			idxs[1] = 2*q;
			idxs[2] = 3*q;
			idxs[3] = size;

			// what if the size of the array is not a multiple of 4?

			set_ref_count(5);
			// spawn tasks to process each quarter of the array
			for (int i=0;i<4;i++) {
				MergeSort& m = *new(allocate_child()) MergeSort(begin1+(i==0 ? 0 : idxs[i-1]), begin1+idxs[i], begin2+(i==0 ? 0 : idxs[i-1]), begin2+idxs[i], cmp);
				if (i == 3)
					spawn_and_wait_for_all(m);
				else
					spawn(m);
			}

			
			// parrallel merge
			ParallelMerge<RandomAccessIterator, Compare>& pm1 = *new(allocate_child()) ParallelMerge<RandomAccessIterator, Compare>(begin1, begin1+idxs[0], begin1+idxs[0], begin1+idxs[1], begin2, begin2+idxs[1], cmp);
			ParallelMerge<RandomAccessIterator, Compare>& pm2 = *new(allocate_child()) ParallelMerge<RandomAccessIterator, Compare>(begin1+idxs[1], begin1+idxs[2], begin1+idxs[2], end1, begin2+idxs[1], end2, cmp);

			set_ref_count(3);
			spawn(pm1);
			spawn_and_wait_for_all(pm2);

			/*
			// sequential (1) merge of the two from the previous parallel merge
			sequentialMerge(begin2, begin2+idxs[1], begin2+idxs[1], end2, begin1, cmp);
			*/
			ParallelMerge<RandomAccessIterator, Compare>& pm3 = *new(allocate_child()) ParallelMerge<RandomAccessIterator, Compare>(begin2, begin2+idxs[1], begin2+idxs[1], end2, begin1, end1, cmp);
			set_ref_count(2);
			spawn_and_wait_for_all(pm3);
/*
			// continuation style
			MergeSortContinuation<RandomAccessIterator, Compare>& ct = 
              *new(allocate_continuation()) 
              MergeSortContinuation<RandomAccessIterator, Compare>(begin1, end1, 
                                                                   begin2, end2, 
                                                                   2*q, cmp);
			 ParallelMerge<RandomAccessIterator, Compare>& pm1 = 
               *new(ct.allocate_child()) 
               ParallelMerge<RandomAccessIterator, Compare>(ct.begin1, 
                                                            ct.begin1+idxs[0], 
                                                            ct.begin1+idxs[0], 
                                                            ct.begin1+idxs[1], 
                                                            ct.begin2, 
                                                            ct.begin2+idxs[1], 
                                                            ct.cmp);
             ParallelMerge<RandomAccessIterator, Compare>& pm2 = 
               *new(ct.allocate_child()) 
               ParallelMerge<RandomAccessIterator, Compare>(ct.begin1+idxs[1], 
                                                            ct.begin1+idxs[2], 
                                                            ct.begin1+idxs[2], 
                                                            ct.end1, 
                                                            ct.begin2+idxs[1], 
                                                            ct.end2, ct.cmp);

             ct.set_ref_count(2);
             ct.spawn(pm1);
             //ct.spawn(pm2);
             return &pm2;
*/
		}
		return NULL;
	}
};

#endif // _MERGESORT_H_
