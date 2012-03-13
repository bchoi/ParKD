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

LEVEL=..
include $(LEVEL)/Makefile.common

SRC_DIR = $(wildcard $(COMMON)/*.cpp) $(wildcard *.cpp)
SRC = $(notdir $(SRC_DIR))

HEADERS = $(wildcard *.h)

all: $(BUILD_DIR) $(BUILD_DIR)/packer

debug: $(BUILD_DIR) $(BUILD_DIR)/packer-debug

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o : %.cpp $(HEADERS)
	@$(ECHO) "       CC  "$@
	$(CC-NORMAL) -c -o $@ $<

$(BUILD_DIR)/%.debug.o : %.cpp $(HEADERS)
	@$(ECHO) "       CC  "$@
	$(CC-DEBUG) -c -o $@ $<

$(BUILD_DIR)/packer: $(SRC:%.cpp=$(BUILD_DIR)/%.o)
	@$(ECHO) "     LINK  "$@
	$(CC) -O3 $(FLAG) $(LFLAG) -o $@ $^

$(BUILD_DIR)/packer-debug: $(SRC:%.cpp=$(BUILD_DIR)/%.debug.o)
	@$(ECHO) "     LINK  "$@
	$(CC) $(FLAG_DBG) -o $@ $^
