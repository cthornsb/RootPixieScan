# Set the hhirf directory
#HHIRF_DIR = /usr/hhirf-intel64
HHIRF_DIR = /usr/hhirf

# Set the ACQ2 library directory
#ACQ2_LIBDIR = /usr/hhirf-intel64
ACQ2_LIBDIR = /usr/hhirf

# Scan libraries
LIBS = $(HHIRF_DIR)/scanorlib.a $(HHIRF_DIR)/orphlib.a\
       $(ACQ2_LIBDIR)/acqlib.a $(ACQ2_LIBDIR)/ipclib.a

CC = g++
FC = gfortran

FFLAGS = -g -fsecond-underscore
CFLAGS = -g -fPIC -Wall -O3 -DLINK_GFORTRAN `root-config --cflags` -Iinclude -DREVF -Dnewreadout
LDLIBS = -lm -lstdc++ -lgsl -lgslcblas -lgfortran `root-config --libs`
LDFLAGS = `root-config --glibs`
ROOT_INC = `root-config --incdir`

TOP_LEVEL = $(shell pwd)
DICT_DIR = $(TOP_LEVEL)/dict
INCLUDE_DIR = $(TOP_LEVEL)/include
SOURCE_DIR = $(TOP_LEVEL)/src
FORT_DIR = $(TOP_LEVEL)/scan

OBJ_DIR = $(TOP_LEVEL)/obj
C_OBJ_DIR = $(OBJ_DIR)/c++
FORT_OBJ_DIR = $(OBJ_DIR)/fortran
DICT_OBJ_DIR = $(DICT_DIR)/obj

EXECUTABLE = PixieLDF

# Source code stuff
FORTRAN = messlog.f mildatim.f scanor.f set2cc.f
SOURCES = BetaProcessor.cpp DssdProcessor.cpp LogicProcessor.cpp Places.cpp ReadBuffData.RevD.cpp \
	  Trace.cpp CfdAnalyzer.cpp EventProcessor.cpp MapFile.cpp Plots.cpp TraceExtractor.cpp ChanEvent.cpp \
	  FittingAnalyzer.cpp McpProcessor.cpp PlotsRegister.cpp TraceFilterer.cpp ChanIdentifier.cpp GeProcessor.cpp \
	  MtcProcessor.cpp PositionProcessor.cpp TracePlotter.cpp Correlator.cpp ImplantSsdProcessor.cpp NeutronProcessor.cpp \
	  pugixml.cpp SsdProcessor.cpp TreeCorrelator.cpp DetectorDriver.cpp Initialize.cpp ParseXml.cpp PulserProcessor.cpp \
	  StatsData.cpp ValidProcessor.cpp DetectorLibrary.cpp IonChamberProcessor.cpp PathHolder.cpp RandomPool.cpp TauAnalyzer.cpp \
	  VandleProcessor.cpp DetectorSummary.cpp LiquidProcessor.cpp PixieStd.cpp RawEvent.cpp TimingInformation.cpp WaveformAnalyzer.cpp \
	  DoubleTraceAnalyzer.cpp PlaceBuilder.cpp TraceAnalyzer.cpp 
FORTOBJ = $(addprefix $(FORT_OBJ_DIR)/,$(FORTRAN:.f=.o))
OBJECTS = $(addprefix $(C_OBJ_DIR)/,$(SOURCES:.cpp=.o))

# ROOT dictionary stuff
DICT_SOURCE = RootDict
STRUCT_FILE = Structures

ROOTOBJ = $(DICT_OBJ_DIR)/$(DICT_SOURCE).o 
ROOTOBJ += $(C_OBJ_DIR)/$(STRUCT_FILE).o
SFLAGS = $(addprefix -l,$(DICT_SOURCE))

#####################################################################

all: directory $(FORTOBJ) $(OBJECTS) $(DICT_OBJ_DIR)/$(DICT_SOURCE).so $(EXECUTABLE)
#	Create all directories, make all objects, and link executable

.PHONY: clean tidy directory

.SECONDARY: $(DICT_DIR)/$(DICT_SOURCE).cpp $(ROOTOBJ)
#	Want to keep the source files created by rootcint after compilation
#	as well as keeping the object file made from those source files

#####################################################################

directory: $(OBJ_DIR) $(FORT_OBJ_DIR) $(C_OBJ_DIR) $(DICT_OBJ_DIR)

$(OBJ_DIR):
#	Make the object file directory
	mkdir $(OBJ_DIR)

$(FORT_OBJ_DIR):
#	Make fortran object file directory
	mkdir $(FORT_OBJ_DIR)
	
$(C_OBJ_DIR):
#	Make c++ object file directory
	mkdir $(C_OBJ_DIR)

$(DICT_OBJ_DIR):
#	Make root dictionary object file directory
	mkdir $(DICT_OBJ_DIR)

########################################################################

$(FORT_OBJ_DIR)/%.o: $(FORT_DIR)/%.f
#	Compile fortran source files
	$(FC) -c $(FFLAGS) $< -o $@

$(C_OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
#	Compile C++ source files
	$(CC) -c $(CFLAGS) $< -o $@

#####################################################################

$(DICT_OBJ_DIR)/%.o: $(DICT_DIR)/%.cpp
#	Compile rootcint source files
	$(CC) -c $(CFLAGS) $< -o $@

$(DICT_OBJ_DIR)/%.so: $(C_OBJ_DIR)/Structures.o $(DICT_OBJ_DIR)/$(DICT_SOURCE).o
#	Generate the root shared library (.so) for the dictionary
	$(CC) -g -shared -Wl,-soname,lib$(DICT_SOURCE).so -o $(DICT_OBJ_DIR)/lib$(DICT_SOURCE).so $(C_OBJ_DIR)/Structures.o $(DICT_OBJ_DIR)/$(DICT_SOURCE).o -lc

$(DICT_DIR)/%.cpp: $(INCLUDE_DIR)/$(STRUCT_FILE).h $(DICT_DIR)/LinkDef.h
#	Generate the dictionary source files using rootcint
	@cd $(DICT_DIR); rootcint -f $@ -c $(INCLUDE_DIR)/$(STRUCT_FILE).h $(DICT_DIR)/LinkDef.h

#####################################################################

$(EXECUTABLE): $(FORTOBJ) $(OBJECTS)
#	Link the executable
	$(FC) $(LDFLAGS) $(FORTOBJ) $(OBJECTS) $(ROOTOBJ) $(LIBS) -L$(DICT_OBJ_DIR) $(SFLAGS) -o $@ $(LDLIBS)

#####################################################################

clean:
	@echo "Cleaning up..."
	@rm -f $(C_OBJ_DIR)/*.o $(FORT_OBJ_DIR)/*.o ./PixieLDF
	
tidy:
	@echo "Removing ROOT dictionaries..."
	@rm -f $(DICT_DIR)/$(DICT_SOURCE).cpp $(DICT_DIR)/$(DICT_SOURCE).h $(DICT_OBJ_DIR)/*.o  $(DICT_OBJ_DIR)/*.so
