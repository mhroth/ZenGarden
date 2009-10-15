LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjnizengarden
LOCAL_SRC_FILES := jnizengarden.cpp

LOCAL_STATIC_LIBRARIES := sndfile zengarden
#LOCAL_SHARED_LIBRARIES := sndfile

LOCAL_CFLAGS := -I$(LOCAL_PATH)/../libsndfile-1.0.20/src -L$(LOCAL_PATH)/../libsndfile-1.0.20/src/libs/

include $(BUILD_STATIC_LIBRARY)
