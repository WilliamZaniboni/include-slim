# Makefile for GCC
#
# Author: Fabio Jun Takada Chino
# Author: Marcos Rodrigues Vieira
# Revision: Luis Fernando Milano Oliveira
#
CC=gcc
AR=ar
CFLAGS=-m64 -fPIC
prefix?=/usr/local
exec-prefix?=/usr/local
SRCPATH=./src/arboretum
INCLUDEPATH=./src/include/
INCLUDE=-I$(INCLUDEPATH)
SRC=	$(SRCPATH)/CStorage.cpp \
	$(SRCPATH)/stCellId.cpp \
	$(SRCPATH)/stCompress.cpp \
	$(SRCPATH)/stCountingTree.cpp \
	$(SRCPATH)/stDBMNode.cpp \
	$(SRCPATH)/stDFNode.cpp \
	$(SRCPATH)/stDiskPageManager.cpp \
	$(SRCPATH)/stDummyNode.cpp \
	$(SRCPATH)/stGHNode.cpp \
	$(SRCPATH)/stGnuplot.cpp \
	$(SRCPATH)/stGnuplot3D.cpp \
	$(SRCPATH)/stLevelDiskAccess.cpp \
	$(SRCPATH)/stListPriorityQueue.cpp \
	$(SRCPATH)/stMMNode.cpp \
	$(SRCPATH)/stMNode.cpp \
	$(SRCPATH)/stMemoryPageManager.cpp \
	$(SRCPATH)/stPage.cpp \
	$(SRCPATH)/stPlainDiskPageManager.cpp \
	$(SRCPATH)/stPointSet.cpp \
	$(SRCPATH)/stResult.cpp \
	$(SRCPATH)/stSeqNode.cpp \
	$(SRCPATH)/stSlimNode.cpp \
	$(SRCPATH)/stStructUtils.cpp \
	$(SRCPATH)/stTreeInformation.cpp \
	$(SRCPATH)/stUtil.cpp \
	$(SRCPATH)/stVPNode.cpp
#	$(SRCPATH)/stMetricHistogram.cpp \
#	$(SRCPATH)/stMGrid.cpp \

OBJS=$(subst .cpp,.o,$(SRC))

STD=-std=c++17

LIBNAME=build/libarboretum.a

# Implicit Rules
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(STD) -c $< -o $@ $(INCLUDE)

# Rules
$(LIBNAME): $(OBJS)
	mkdir build
	$(AR) -r $(LIBNAME) $(OBJS)

default: $(LIBNAME)

help:
	@echo Arboretum gcc Makefile
	@echo '
	@echo Targets:
	@echo    default: Build libarboretum.a
	@echo    help:    Prints this help screen
	@echo    clean:   Remove all .o files
	@echo    install: Install library and headers
	@echo '
	@echo The install target has two options:
	@echo    prefix = include prefix
	@echo    exec-prefix = lib prefix
	@echo '
	@echo e.g.: make install prefix=... exec-prefix=...
	@echo '
	@echo Current values are:
	@echo    prefix = $(prefix) [$(prefix)/include]
	@echo    exec-prefix = $(exec-prefix) [$(exec-prefix)/lib]

clean:
	rm -f $(SRCPATH)/*.o
	rm -f $(LIBNAME)
	rm -fR build

install:
	@echo This target is not complete yet.
