# -----------------------------------------------------------------------------
# Function : 递归查找源文件
# Arguments: 1: 源文件根目录
# Returns  : 源文件路径
# Usage    : $(call find-sources,<source root>)
# -----------------------------------------------------------------------------
SRC_FILE_SUFFIXS = 	$1/*.cpp \
					$1/*.cc
find-sources = $(wildcard $(SRC_FILE_SUFFIXS)) $(foreach d,$(wildcard $1/*),$(call find-sources,$d))