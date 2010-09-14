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

LEVEL= .
include $(LEVEL)/Makefile.common

SRCS= $(HEADER_FILES) $(ACCEL_SRC)

COMMON_DEP = $(wildcard Common/*.cpp Common/*.h ParKD/*.cpp ParKD/*.h)

.PHONY: Accel-%/clean dev dev-debug benchmark benchmark-% parkd-%

.PRECIOUS: $(RESULTS)/% %.obj %.blob

###########################################################################
# Build targets
###########################################################################

# Build all normal targets
all: $(subst Accel,parkd,$(ALL_ACCEL)) $(PACKER_EXEC)
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Built: "
	@$(ECHO) "# "$^
	@$(ECHO) "################################################################\
###########"
	$(MAKE) pack

# Build all debug targets
all-debug: $(foreach parkd,$(subst Accel,parkd,$(ALL_ACCEL)),$(parkd)-debug)  \
	$(PACKER_EXEC)-debug
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Built: "
	@$(ECHO) "# "$^
	@$(ECHO) "################################################################\
###########"
	$(MAKE) pack

# The following rule (parkd-%-debug) need to come before parkd-% for correct
# pattern matching order
parkd-%-debug:
	ACCEL_DIR=Accel-$* $(MAKE) -C Accel-$* debug
	ln -sf Accel-$*/bin/parkd-debug $@

parkd-%:
	ACCEL_DIR=Accel-$* $(MAKE) -C Accel-$*
	ln -sf Accel-$*/bin/parkd $@

###########################################################################
# Development targets - Use these to build dev-* implementations
###########################################################################

dev: $(subst dev-Accel,dev-parkd,$(ACCEL_DIR))
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Built: "
	@$(ECHO) "# "$^
	@$(ECHO) "################################################################\
###########"

dev-debug: $(foreach parkd,$(subst dev-Accel,dev-parkd,$(ACCEL_DIR)),         \
           $(parkd)-debug)
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Built: "
	@$(ECHO) "# "$^
	@$(ECHO) "################################################################\
###########"

dev-parkd-%-debug:
	ACCEL_DIR=dev-Accel-$* $(MAKE) -C dev-Accel-$* debug
	ln -sf dev-Accel-$*/bin/parkd-debug $@

dev-parkd-%:
	ACCEL_DIR=dev-Accel-$* $(MAKE) -C dev-Accel-$*
	ln -sf dev-Accel-$*/bin/parkd $@

###########################################################################
# Pack target - prepare input meshes
###########################################################################

pack: $(PACKER_EXEC) $(MODELS_PACKED)
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Packed:" 
	@for i in $(MODELS_PACKED); do $(ECHO) "# "$$i; done
	# @$(ECHO) "# "$(MODELS_PACKED)
	@$(ECHO) "################################################################\
###########"

packer:
	$(MAKE) -C Packer
	ln -sf Packer/bin/$@ $@

packer-debug:
	$(MAKE) -C Packer debug
	ln -sf Packer/bin/$@ $@

###########################################################################
# Benchmark targets - need to set THREAD_LIMIT in Makefile.common
###########################################################################

# Benchmark for a specific impl (CSV output)
benchmark-csv-%: idle_check parkd-% $(MODELS_PACKED) $(RESULTS)/%
	$(MAKE) -C $(RESULTS)/$* benchmark-csv

# Benchmark all impl/s (CSV output)
benchmark-csv: $(subst Accel,benchmark-csv,$(ALL_ACCEL))
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Benchmarked:" 
	@$(ECHO) "# "$(foreach parkd,$^,$(subst benchmark-csv,parkd,$(parkd)))
	@$(ECHO) "################################################################\
###########"

benchmark: $(subst Accel,benchmark,$(ALL_ACCEL))
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Benchmarked:" 
	@$(ECHO) "# "$(foreach parkd,$^,$(subst benchmark,parkd,$(parkd)))
	@$(ECHO) "################################################################\
###########"

# Benchmark for a specific impl
benchmark-%: parkd-% $(MODELS_PACKED) $(RESULTS)/%
	$(MAKE) -C $(RESULTS)/$* benchmark

idle_check:
	@$(ECHO) "Please make sure that the system is idle."
	@$(UPTIME)
	@$(ECHO)
	@$(SLEEP) 3

###########################################################################
# Regression test (check) targets
###########################################################################

# Run regression tests for all impl/s
check: $(subst Accel,check,$(ALL_ACCEL))
	@$(ECHO)
	@$(ECHO) "################################################################\
###########"
	@$(ECHO) "# Checked:"
	@python Utils/report.py
	@$(ECHO) "################################################################\
###########"

# Run regression tests for a specific impl
check-%: $(TEST_MODELS_PACKED) parkd-% $(RESULTS)/%
	$(MAKE) -C $(RESULTS)/$* check-one

$(RESULTS)/%: Accel-%
	@mkdir -p $@
	@$(ECHO) "IMPL = "$* >> $(RESULTS)/$*/Makefile.conf
	@$(ECHO) "LEVEL = ../.." >> $(RESULTS)/$*/Makefile.conf
	@cd $@; ln -sf ../Makefile.subdir Makefile

###########################################################################
# Clean targets
###########################################################################

TO_CLEAN = *.out packer packer-debug $(subst Accel,parkd,$(ALL_ACCEL))        \
      $(foreach parkd,$(subst Accel,parkd,$(ALL_ACCEL)),$(parkd)-debug)       \
      $(subst dev-Accel,dev-parkd,$(ALL_DEV_ACCEL))                           \
      $(foreach parkd,$(subst dev-Accel,dev-parkd,$(ALL_DEV_ACCEL)),$(parkd)-debug)\

# Clean everything
clean-all: clean clean-results clean-models

# Clean object files and executables
clean: $(foreach dir,$(ALL_ACCEL),$(dir)/clean)
	@$(ECHO) "    CLEAN  "$(PACKER)
	$(MAKE) -C $(PACKER) clean

    # executables
	for i in $(TO_CLEAN); do                                                  \
      if [ -h $$i ]; then                                                     \
        $(ECHO) "    CLEAN  "$$i;                                             \
        rm -f $$i;                                                            \
      fi;                                                                     \
    done

# Clean Results/ directory
clean-results:
	@$(ECHO) "    CLEAN  "$(RESULTS)
	find $(RESULTS)/* -type d | xargs rm -rf

# Clean uncompressed/packed models
clean-models:
	@$(ECHO) "    CLEAN  "$(MODELS_DIR)
	rm -f $(MODELS_DIR)/*.obj $(MODELS_DIR)/*.blob

Accel-%/clean:
	ACCEL_DIR=Accel-$* $(MAKE) -C Accel-$* clean

# Don't want to do this every time packer gets remade - it'll never change!
# http://www.gnu.org/software/make/manual/make.html#Prerequisite-Types
%.blob : %.obj | $(PACKER_EXEC)
	@$(ECHO) "     PACK  "$@
	$(PACKER_EXEC) < $< > $@

%.obj: %.obj.bz2
	@$(ECHO) "  BUNZIP2  "$@
	$(BUNZIP2) -c < $< > $@
