LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sqlitecpp_static

LOCAL_MODULE_FILENAME := libsqlitecpp

LOCAL_ARM_MODE := arm

SRC_FILE_LIST := \
src/Backup.cpp \
src/Column.cpp \
src/Database.cpp \
src/Exception.cpp \
src/Statement.cpp \
src/Transaction.cpp \
sqlite3/sqlite3.c

LOCAL_SRC_FILES := $(SRC_FILE_LIST)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/sqlite3

LOCAL_CPPFLAGS += -fexceptions 
LOCAL_CFLAGS += -DSQLITE_ENABLE_COLUMN_METADATA -DSQLITE_HAS_CODEC

LOCAL_STATIC_LIBRARIES := ext_crypto

include $(BUILD_STATIC_LIBRARY)

