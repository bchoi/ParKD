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

#ifndef _PARALLELMERGE_H_
#define _PARALLELMERGE_H_

#include <tbb/task.h>

#include "config.h"

template<typename RandomAccessIterator, typename Compare>
class ParallelMerge : public tbb::task {

public:
	RandomAccessIterator begin1, begin2, begin3;
	RandomAccessIterator end1, end2, end3;
	const Compare& cmp;

	ParallelMerge(RandomAccessIterator begin1_, 
                  RandomAccessIterator end1_, 
                  RandomAccessIterator begin2_, 
                  RandomAccessIterator end2_, 
                  RandomAccessIterator begin3_, 
                  RandomAccessIterator end3_, 
                  const Compare& cmp_) : 
      begin1(begin1_), begin2(begin2_), begin3(begin3_), end1(end1_), 
      end2(end2_), end3(end3_), cmp(cmp_) {}
	task* execute() {

		size_t size1 = end1 - begin1;
		size_t size2 = end2 - begin2;
		size_t size3 = end3 - begin3;
		if (size1 <= MIN_SIZE || size2 <= MIN_SIZE) {
			sequentialMerge(begin1, end1, begin2, end2, begin3, cmp);
		}
		else {
			// Java logical shift -> c++ arithmetic shift
			int aHalf = size1 >> 1;
			int bSplit = findSplit(begin1+aHalf, begin2, end2);

			// recursively merge in parallel
			ParallelMerge& pm1 = *new(allocate_child()) 
              ParallelMerge(begin1, begin1+aHalf, begin2, begin2+bSplit, 
                            begin3, begin3+aHalf+bSplit, cmp);
			ParallelMerge& pm2 = *new(allocate_child()) 
              ParallelMerge(begin1+aHalf, end1, begin2+bSplit, end2, 
                            begin3+aHalf+bSplit, end3, cmp);
			set_ref_count(3);
			spawn(pm1);
			spawn_and_wait_for_all(pm2);
		}
		return NULL;
	}
	/*
	void sequentialMerge(RandomAccessIterator begin1,
					RandomAccessIterator end1,
					RandomAccessIterator begin2,
					RandomAccessIterator end2,
					RandomAccessIterator begin3,
					const Compare& cmp);
	*/
	int findSplit(RandomAccessIterator value, 
                  RandomAccessIterator begin, 
                  RandomAccessIterator end) {
		size_t low = 0;
		size_t high = end-begin;
		while (low < high) {
			// Java logical shift -> arithmetic shift (c++)
			size_t middle = low + ((high-low) >> 1);
			// cmp call
			if (cmp(*value, *(begin+middle)))
				high = middle;
			else
				low = middle + 1;
		}
		return high;
	}
};

#endif // _PARALLELMERGE_H_
