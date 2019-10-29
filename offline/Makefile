ROOTCFLAGS    = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs)
ROOTGLIBS     = $(shell root-config --glibs) -lTMVA

CXX           = g++
CXXFLAGS      = -g -fPIC -Wno-deprecated -O -ansi -D_GNU_SOURCE -g -O2
LD            = g++
LDFLAGS       = -g
SOFLAGS       = -shared


ARCH         := $(shell root-config --arch)
PLATFORM     := $(shell root-config --platform)


CXXFLAGS      += $(ROOTCFLAGS)
#CXX           += -I./
LIBS           = $(ROOTLIBS) 

NGLIBS         = $(ROOTGLIBS) 
NGLIBS        += -lMinuit 
GLIBS          = $(filter-out -lNew, $(NGLIBS))


INCLUDEDIR           = ./
OUTLIB               = ./lib/

.SUFFIXES: .cc,.C,.hh,.h
.PREFIXES: ./lib/

PlainTObjectsDumper: $(INCLUDEDIR)/PlainTObjectsDumper.cc \
	$(OUTLIB)DataTreeBaseClass.o 
	$(CXX) $(CXXFLAGS) -I$(INCLUDEDIR) -ldl -o PlainTObjectsDumper $(OUTLIB)/*.o $(GLIBS) $(LDFLAGS) $ $<

$(OUTLIB)DataTreeBaseClass.o: $(INCLUDEDIR)/DataTreeBaseClass.C
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) -o $(OUTLIB)DataTreeBaseClass.o $<

clean:
	rm -f lib/*o
	rm -f PlainTObjectsDumper
