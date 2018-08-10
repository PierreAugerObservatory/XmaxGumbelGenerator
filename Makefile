MAIN_SRC= XmaxGumbelGenerator.cc
##MAIN_OBJ = $(MAIN_SRC:.cc=.o)
EXE= XmaxGumbelGenerator

### CXXFLAGS ###
##CXXFLAGS    = $(shell root-config --cflags)
CXXFLAGS = -std=c++11 -O2 -Wall -Wunused -Wuninitialized -Woverloaded-virtual -fPIC -pthread -m64 

### CPPFLAGS ###
CPPFLAGS_ROOT= -I$(ROOTSYS)/include
CPPFLAGS = $(CPPFLAGS_ROOT)

### LDFLAGS ####
LDFLAGS_ROOT = $(shell root-config --libs) -L$(ROOTSYS)/lib -lSpectrum -lMathMore -lMinuit
LDFLAGS_ROOT+= $(shell root-config --ldflags)
LDFLAGS = $(LDFLAGS_ROOT)

##### TARGETS #######
all: $(EXE)

$(EXE): $(MAIN_SRC)
	@echo "Building $(EXE) from $(MAIN_SRC) ..."
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f *~ *.o  *.so *.ps core  $(EXE)

