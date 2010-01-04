ROOTLIBS:=$(shell root-config --libs)
ROOTINC:=-I$(shell root-config --incdir)
RATINC:=-I$(RATROOT)/include -I$(RATROOT)/src/stlplus
RATLIB:=-L$(RATROOT)/lib
BRINC:=-I./include

#XYZTDUMP DEFS
XYZTNAME:=xyzt
STHROWNAME:=spherethrow
MOMNAME:=moi

#RATLIB DEFS
RATEVLIB:=RATEvent_Linux-g++

#CXX DEFS
CXX:=g++
CXXFLAGS=-O2 -Wall -g

#TARGET SPECS

##XYZT
XYZTSRCDIR:=src
XYZTINCDIR:=include
XYZTTARGS:=xyzt.cxx

##STHROW
STHROWSRCDIR:=src
STHROWINCDIR:=include
STHROWTARGS:=spherethrow.cxx
	
##MOM
MOMSRCDIR:=src
MOMINCDIR:=include
MOMTARGS:=moments.cxx

all: xyzt sthrow moments

xyzt:
	$(CXX) $(CXXFLAGS) -o $(XYZTNAME) $(RATINC) $(RATLIB) -l$(RATEVLIB) $(RATINC) $(ROOTINC) -I$(XYZTINCDIR) $(XYZTSRCDIR)/$(XYZTTARGS)
	
sthrow:
	$(CXX) $(CXXFLAGS) -o $(STHROWNAME) -I$(STHROWINCDIR) $(STHROWSRCDIR)/$(STHROWTARGS)
	
moments:
	$(CXX) $(CXXFLAGS) -o $(MOMNAME) -I$(MOMINCDIR) $(MOMSRCDIR)/$(MOMTARGS)
	
distclean:
	rm xyzt spherethrow > /dev/null 2>&1 