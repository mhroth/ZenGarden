my-dir = $(dir $(lastword $(MAKEFILE_LIST)))

include Android.mk
# LOCAL_SRC_FILES
OBJS := $(foreach FILE,$(LOCAL_SRC_FILES),$(FILE:.cpp=.o))

CXX = g++
CXXFLAGS = -g -Os -I. -Wall -Wno-sign-compare -Wno-unknown-pragmas -Wno-format $(LOCAL_CFLAGS)

$(LOCAL_MODULE).so: $(OBJS) $ ../libsndfile-1.0.20/src/.libs/libsndfile.so
	${CXX} $(OBJS) ${CXXFLAGS} -shared -fPIC -L. $(LOCAL_CFLAGS) -Wl,-soname,$@ -o $@
clean:
	rm -f $(LOCAL_MODULE).so *.o

../libsndfile-1.0.20/src/.libs/libsndfile.so: ../libsndfile-1.0.20 
	cd ../libsndfile-1.0.20 && ./configure && make

