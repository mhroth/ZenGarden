my-dir = $(dir $(lastword $(MAKEFILE_LIST)))

include Makefile.sourcefiles

OBJS := $(foreach FILE,$(LOCAL_SRC_FILES),$(FILE:.cpp=.o))

SNDFILE_INCLUDE = `pkg-config --cflags sndfile`
SNDFILE_LIB = `pkg-config --libs sndfile`

#CXX = g++
#CXXFLAGS = -g -Os -I. -Wall -Wno-sign-compare -Wno-unknown-pragmas -Wno-format $(LOCAL_CFLAGS)
CXXFLAGS = -g -O3 -fPIC -Wall -I$(my-dir) -Wno-sign-compare -Wno-unknown-pragmas -Wno-format $(SNDFILE_INCLUDE)

#$(LOCAL_MODULE).so: $(OBJS) libsndfile-flags
#	${CXX} $(OBJS) ${CXXFLAGS} -shared -fPIC -L. $(LOCAL_CFLAGS) -Wl,-soname,$@ -o $@

all: libzengarden libjnizengarden examplegarden

clean:
	rm -rf $(LOCAL_MODULE).so *.o src/me/rjdj/zengarden/*.class ../ZenGarden.jar ../libs/osx/*

libsndfile-flags:
	pkg-config --libs --cflags sndfile >> libsndfile-flags

libjnizengarden: ../libs/osx/libjnizengarden.jnilib
	
../libs/osx/libjnizengarden.jnilib: ./me/rjdj/zengarden/jnizengarden.cpp $(OBJS)
	gcc -o $@ $(CXXFLAGS) -dynamiclib \
	-I/Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Headers \
	$< $(OBJS) $(SNDFILE_LIB) -lstdc++ 

libzengarden: ../libs/osx/libzengarden.dylib

../libs/osx/libzengarden.dylib: ./me/rjdj/zengarden/jnizengarden.cpp $(OBJS)
	gcc -o $@ $(CXXFLAGS) -dynamiclib \
	-I/Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Headers \
	$< $(OBJS) $(SNDFILE_LIB) -lstdc++

examplegarden: me/rjdj/zengarden/*.java
	cd me/rjdj/zengarden/ && javac *.java
	jar cfm ../ZenGarden.jar me/rjdj/zengarden/manifest me/rjdj/zengarden/*.class
