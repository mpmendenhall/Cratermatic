#####################################################################
# Makefile for Cratermatic project
#####################################################################


# assure correct shell is used
SHELL = /bin/sh
# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o
	 
# compiler command to use
CC = cc
CXX = g++

CXXFLAGS = -std=c++03 -O3 -fPIC -I. -IBase -IClassify -IImage -IInteractive
LDFLAGS =  -L/sw/lib/ -lfftw3

#
# things to build
#

VPATH = ./:Base/:Classify/:Image/:Interactive/

objects = Basics.o Classify_ImageData.o Classify_IO.o \
	Classify_Morphology.o Watershed.o Classify.o \
	ComplexImage.o CraterFinder.o Histogram.o Image_Drawing.o \
	Image_Geometry.o Image_IO.o Image_Math.o Image_Transforms.o \
	Image.o Interactive_Stack.o Interactive_Top.o \
	Interactive_TopInteractor.o Interactive.o Merger.o \
	RasterRegion.o RectRegion.o RGBImage.o Utils.o

all: Cratermatic

Cratermatic: craters.cpp libCratermatic.a
	$(CXX) $(CXXFLAGS) craters.cpp $(LDFLAGS) -o Cratermatic

libCratermatic.a: $(objects)
	ar rs libCratermatic.a $(objects)

# generic rule for everything else .cpp linked against libCratermatic
% : %.cpp libCratermatic.a
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

#
# cleanup
#
.PHONY: clean
clean:
	-rm -f libCratermatic.a Cratermatic
	-rm -f *.o
