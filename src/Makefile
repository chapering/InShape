# Makefile for building OpenGL program in C++ 
#!/usr/bin/make -f  
_VERSION_=(64 bit) 1.5.0.0 

INC=-I./ \
	-I/usr/include \
	-I/usr/local/include/gdcm-2.1
LIB=-L./ \
	-L/usr/local/lib/gdcm-2.1
# c++0x supports array initialization in constructor
FLAGS=-DLINUX -Wall -O3 
DEBUG=0

LIBS = /lib64/libdl.so.2 $(LIB) \
	   -lgdcmMSFF -lgdcmDSED -lgdcmDICT \
	   -lgdcmopenjpeg -lgdcmcharls -lgdcmzlib \
	   -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmjpeg8 \
	   -lgdcmIOD -lgdcmCommon \
	   -lgdcmexpat \
	   -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm \
	   -lpthread -lrt 

CC=g++

LIBSRCS= mitubeRenderView.cpp mitubeRenderWindow.cpp situbeRender.cpp \
		GLoader.cpp cppmoth.cpp GLcppmoth.cpp \
		iselbox.cpp  GLiboxmoth.cpp  GLgadget.cpp DCMviewer.cpp

LIBOBJ=$(LIBSRCS:.cpp=.o)

ifneq ($(DEBUG), 0)
	FLAGS=-Wno-deprecated -DDEBUG -g 
	_VERSION_ += (debug version)
endif

%.o : %.cpp
	$(CC) -c $< -o $@ $(INC) $(FLAGS) -DVERSION=\""$(_VERSION_)\""

###############################################################################

EXE  = $(t)
SRCS = $(EXE).cpp $(LIBSRCS)
OBJS = $(SRCS:.cpp=.o) 

$(EXE) : $(OBJS)
	$(CC) -o $@ $< $(LIBOBJ) $(LIBS)
	-@rm -rf $(OBJS)

.PHONY : single 
single : $(EXE)
	@echo "Single target found got built."; 

.PHONY : clean
clean : 
	-@rm -rf $(t) $(OBJS)

.PHONY : rebuild
rebuild : clean single

.PHONY : release
release : rebuild

###############################################################################

EXES = singleitr multiviewitr multiwindowitr
ALLSRCS = $(foreach x,$(EXES), ${x}.cpp) $(LIBSRCS)
ALLOBJS = $(ALLSRCS:.cpp=.o) 

$(EXES) : $(foreach x,$(EXES),${x}.o) $(LIBOBJ)
	$(CC) -o $@ $@.o $(LIBOBJ) $(LIBS)
	-@rm -rf $@.o

.PHONY : all
all : $(EXES)
	@echo "All found got built."; 

.PHONY : cleanall 
cleanall : 
	-@rm -rf $(EXES) $(ALLOBJS)

.PHONY : rebuildall
rebuild : cleanall all

.PHONY : releaseall
release : rebuildall

###############################################################################

help : usage

.PHONY : usage
usage : 
	@echo " [ Usage ] "
	@echo " build single :       make t=<target> | clean | release | rebuild"
	@echo " build all    :       make all | cleanall | releaseall | rebuildall" 
	@echo

