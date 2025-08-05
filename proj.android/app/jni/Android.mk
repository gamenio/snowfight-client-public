LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
include $(LOCAL_PATH)/macros.mk

$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos/audio/include)
$(call import-add-path,$(LOCAL_PATH)/../../../dep)

LOCAL_MODULE := snowfight

LOCAL_MODULE_FILENAME := libsnowfight

SRC_FILE_LIST := $(call find-sources,$(LOCAL_PATH)/../../../Classes)

LOCAL_SRC_FILES := snowfight/main.cpp \
					$(SRC_FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += ../../../cocos2d/external/xxtea/xxtea.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Classes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dep/asio
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../dep/gpg-cpp-sdk/android/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../cocos2d/external/xxtea

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cc_static
LOCAL_STATIC_LIBRARIES += protobuflite_static
LOCAL_STATIC_LIBRARIES += libgpg-1
LOCAL_STATIC_LIBRARIES += ext_ssl
LOCAL_STATIC_LIBRARIES += sqlitecpp_static
LOCAL_STATIC_LIBRARIES += firebase_gma
LOCAL_STATIC_LIBRARIES += firebase_analytics
LOCAL_STATIC_LIBRARIES += firebase_app # The library needs to be placed after other Firebase libraries

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module, cocos)
$(call import-module, protobuf-lite)
$(call import-module, gpg-cpp-sdk/android)
$(call import-module, sqlitecpp)
$(call import-module, firebase-cpp-sdk)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
