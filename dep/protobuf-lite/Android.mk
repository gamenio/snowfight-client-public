LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := protobuflite_static

LOCAL_MODULE_FILENAME := libprotobuf-lited

LOCAL_ARM_MODE := arm

SRC_FILE_LIST := \
google/protobuf/arena.cc \
google/protobuf/arenastring.cc \
google/protobuf/extension_set.cc \
google/protobuf/generated_message_table_driven_lite.cc \
google/protobuf/generated_message_util.cc \
google/protobuf/io/coded_stream.cc \
google/protobuf/io/zero_copy_stream.cc \
google/protobuf/io/zero_copy_stream_impl_lite.cc \
google/protobuf/message_lite.cc \
google/protobuf/repeated_field.cc \
google/protobuf/stubs/bytestream.cc \
google/protobuf/stubs/common.cc \
google/protobuf/stubs/int128.cc \
google/protobuf/stubs/io_win32.cc \
google/protobuf/stubs/once.cc \
google/protobuf/stubs/status.cc \
google/protobuf/stubs/statusor.cc \
google/protobuf/stubs/stringpiece.cc \
google/protobuf/stubs/stringprintf.cc \
google/protobuf/stubs/structurally_valid.cc \
google/protobuf/stubs/strutil.cc \
google/protobuf/stubs/time.cc \
google/protobuf/stubs/atomicops_internals_x86_gcc.cc \
google/protobuf/wire_format_lite.cc

LOCAL_SRC_FILES := $(SRC_FILE_LIST)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)
                                 
include $(BUILD_STATIC_LIBRARY)

