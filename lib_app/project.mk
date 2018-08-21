LIB_APP_SRC:=

LIB_APP_SRC+=lib_app/utils.cpp\
	     lib_app/convert.cpp\
	     lib_app/BufPool.cpp\
	     lib_app/BufferMetaFactory.c\
		 lib_app/AllocatorTracker.cpp\
		 lib_app/FileIOUtils.cpp\


ifeq ($(findstring mingw,$(TARGET)),mingw)
else
	LIB_APP_SRC+=lib_app/console_linux.cpp
endif

UNITTEST+=$(shell find lib_app/unittests -name "*.cpp")
UNITTEST+=$(LIB_APP_SRC)
