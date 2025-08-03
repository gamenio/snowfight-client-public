LOCAL_PATH:=$(call my-dir)

# With Firebase libraries for the selected build configuration (ABI)
FIREBASE_LIBRARY_PATH:=\
libs/android/$(TARGET_ARCH_ABI)

# The core Firebase library is required for all Firebase products.
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_app
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_app.a
LOCAL_EXPORT_C_INCLUDES:=$(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

# Include the Firebase library for Google Analytics.
# Note: Duplicate this block for each Firebase product that you want
# to use in your app, and replace variable values as appropriate.
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_analytics
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_analytics.a
LOCAL_EXPORT_C_INCLUDES:=$(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

# Include the library for Google Mobile Ads for Firebase.
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_gma
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_gma.a
LOCAL_EXPORT_C_INCLUDES:=$(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)