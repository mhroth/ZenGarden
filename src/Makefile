my-dir = $(dir $(lastword $(MAKEFILE_LIST)))

include Makefile.sourcefiles

OBJS := $(foreach FILE,$(LOCAL_SRC_FILES),$(FILE:.cpp=.o))

SNDFILE_INCLUDE = `pkg-config --cflags sndfile`
SNDFILE_LIB = `pkg-config --libs sndfile`

#CXXFLAGS = -g -Os -I. -Wall -Wno-sign-compare -Wno-unknown-pragmas -Wno-format $(LOCAL_CFLAGS)
CXXFLAGS = -g -Os -fPIC -Wall -I$(my-dir) $(SNDFILE_INCLUDE)

### User should manually specify the platform

SUPPORTED_PLATFORMS=Linux OSX

ifeq (,$(findstring $(OS),$(SUPPORTED_PLATFORMS)))

### Help message
all %:
	@echo The OS environment variable is set to [$(OS)].
	@echo Please set the OS environment variable to one of the following:
	@echo $(SUPPORTED_PLATFORMS)
	@echo for example:
	@echo '`OS=Linux make`'

else

### platform specific makefile for definitions
include makefile.$(OS)

all: libzengarden libjnizengarden examplegarden

clean:
	rm -rf $(LOCAL_MODULE).so *.o src/me/rjdj/zengarden/*.class ../ZenGarden.jar ../libs/$(PLATFORM_DIR)/*

libjnizengarden: ../libs/$(PLATFORM_DIR)/libjnizengarden.$(JNI_EXTENSION)

../libs/$(PLATFORM_DIR)/libjnizengarden.$(JNI_EXTENSION): ./me/rjdj/zengarden/jnizengarden.cpp $(OBJS)
	$(call MAKE_SO, $@, $<, $(OBJS))

libzengarden: ../libs/$(PLATFORM_DIR)/libzengarden.$(SO_EXTENSION)

../libs/$(PLATFORM_DIR)/libzengarden.$(SO_EXTENSION): ./me/rjdj/zengarden/jnizengarden.cpp $(OBJS)
	$(call MAKE_SO, $@, $<, $(OBJS))

examplegarden: me/rjdj/zengarden/*.java
	cd me/rjdj/zengarden/ && javac *.java
	jar cfm ../ZenGarden.jar me/rjdj/zengarden/manifest me/rjdj/zengarden/*.class

endif
