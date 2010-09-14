# Copyright (c) 2010 University of Illinois
# All rights reserved.
#
# Developed by:           DeNovo group, Graphis@Illinois
#                         University of Illinois
#                         http://denovo.cs.illinois.edu
#                         http://graphics.cs.illinois.edu
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal with the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimers.
#
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following disclaimers
#    in the documentation and/or other materials provided with the
#    distribution.
#
#  * Neither the names of DeNovo group, Graphics@Illinois, 
#    University of Illinois, nor the names of its contributors may be used to 
#    endorse or promote products derived from this Software without specific 
#    prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.

#!/usr/bin/python

import os, sys
from stat import *

threads = [1, 4]
# models = ['teapot', 'bunny']
models = ['teapot']
accels = [ i[6:] for i in os.listdir('.') if i.startswith('Accel-') ]

for accel in accels:
    success = True
    for model in models:
        for thread in threads:
            # Results/{accel}/Regression_tests/{model}.obj.n{thread}.diff
            try:
                status = os.stat('Results/%s/Regression_tests/%s.n%d.diff' %
                                 (accel, model, thread))
                if not S_ISREG(status[ST_MODE]):
                    success = False
                elif status[ST_SIZE]:
                    success = False
            except OSError:
                success = False
    if success:
        print '# %14s : Success' % accel
    else:
        print '# %14s : Failed' % accel
