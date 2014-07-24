# Optional analyzers
PULSEFIT = 1
#DCFD = 1

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
CFLAGS = -g -fPIC -Wall -O3 -DLINK_GFORTRAN `root-config --cflags` -Iinclude -DREVF -Dpulsefit -Dnewreadout #-Ddcfd
LDLIBS = -lm -lstdc++ -lgsl -lgslcblas -lgfortran `root-config --libs`
LDFLAGS = `root-config --glibs`

DICT_DIR = ./dict
INCLUDE_DIR = ./include
ROOT_INC = `root-config --incdir`
SOURCE_DIR = ./src
FORT_DIR = ./scan
COBJ_DIR = ./obj/c++
FOBJ_DIR = ./obj/fortran

FORTRAN = messlog.f mildatim.f scanor.f set2cc.f
SOURCES = Structures.cpp BetaProcessor.cpp DssdProcessor.cpp LogicProcessor.cpp Places.cpp ReadBuffData.RevD.cpp \
	   Trace.cpp CfdAnalyzer.cpp EventProcessor.cpp MapFile.cpp Plots.cpp TraceExtractor.cpp ChanEvent.cpp \
	   FittingAnalyzer.cpp McpProcessor.cpp PlotsRegister.cpp TraceFilterer.cpp ChanIdentifier.cpp GeProcessor.cpp \
	   MtcProcessor.cpp PositionProcessor.cpp TracePlotter.cpp Correlator.cpp ImplantSsdProcessor.cpp NeutronProcessor.cpp \
	   pugixml.cpp SsdProcessor.cpp TreeCorrelator.cpp DetectorDriver.cpp Initialize.cpp ParseXml.cpp PulserProcessor.cpp \
	   StatsData.cpp ValidProcessor.cpp DetectorLibrary.cpp IonChamberProcessor.cpp PathHolder.cpp RandomPool.cpp TauAnalyzer.cpp \
	   VandleProcessor.cpp DetectorSummary.cpp LiquidProcessor.cpp PixieStd.cpp RawEvent.cpp TimingInformation.cpp WaveformAnalyzer.cpp \
	   DoubleTraceAnalyzer.cpp PlaceBuilder.cpp TraceAnalyzer.cpp 
FORTOBJ = $(addprefix $(FOBJ_DIR)/,$(FORTRAN:.f=.o))
OBJECTS = $(addprefix $(COBJ_DIR)/,$(SOURCES:.cpp=.o))
SHARED = libPixie.so

all: $(FORTOBJ) $(SHARED) $(OBJECTS) PixieLDF

$(FOBJ_DIR)/%.o: $(FORT_DIR)/%.f
	$(FC) -c $(FFLAGS) $< -o $@

$(COBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CC) -c $(CFLAGS) $< -o $@

RootDict.cpp:
	@cd $(DICT_DIR); rootcint -f $@ -c ../$(INCLUDE_DIR)/Structures.h LinkDef.h

RootDict.o: RootDict.cpp
	$(CC) -I$(ROOT_INC) $(CFLAGS) $(LDFLAGS) -c $(DICT_DIR)/RootDict.cpp -o $(COBJ_DIR)/$@

libPixie.so: RootDict.o $(COBJ_DIR)/Structures.o
	$(CC) -g -shared -Wl,-soname,$@ -o $(DICT_DIR)/$@ $(COBJ_DIR)/Structures.o $(COBJ_DIR)/RootDict.o -lc	

PixieLDF: $(FORTOBJ) $(SHARED) $(OBJECTS)
	$(FC) $(LDFLAGS) $(FORTOBJ) $(OBJECTS) $(LIBS) -L$(DICT_DIR) -lPixie -o $@ $(LDLIBS)

clean:
	@echo "Cleaning up..."
	@rm -f $(COBJ_DIR)/*.o $(FOBJ_DIR)/*.o $(DICT_DIR)/RootDict* $(DICT_DIR)/*.so ./PixieLDF
