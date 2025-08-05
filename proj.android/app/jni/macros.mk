# -----------------------------------------------------------------------------
# Function : Recursively find the source files
# Arguments: 1: Source file root directory
# Returns  : Source file path
# Usage    : $(call find-sources,<source root>)
# -----------------------------------------------------------------------------
SRC_FILE_SUFFIXS = 	$1/*.cpp \
					$1/*.cc
find-sources = $(wildcard $(SRC_FILE_SUFFIXS)) $(foreach d,$(wildcard $1/*),$(call find-sources,$d))