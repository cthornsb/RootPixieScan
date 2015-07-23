# Makefile for RootPixieScan
# updated: May 1st, 2015
# Cory Thornsberry

#####################################################################

# Set the PixieSuite directory
PIXIE_SUITE_DIR = /home/pixie16/cthorns//PixieSuitePLD

# Set the hhirf directory
#HHIRF_DIR = /usr/hhirf-intel64
HHIRF_DIR = /usr/hhirf

# Set the ACQ2 library directory
#ACQ2_LIBDIR = /usr/hhirf-intel64
#ACQ2_LIBDIR = /usr/hhirf
ACQ2_LIBDIR = /usr/acq2/lib

# Flag for new readout system. Set to 0 for the classic scan
# Switching this flag may require a full recompilation (make clean && make)
NEW_READOUT = 1

# Flag for turning damm on or off (must be set for damm)
# Switching this flag may require a full recompilation (make clean && make)
USE_HHIRF = 1

# Flag for verbosity
VERBOSE = 0

#####################################################################

FC = gfortran
CC = g++
LINKER =

ifeq ($(NEW_READOUT), 1)
	LINKER = $(CC)
else
	LINKER = $(FC)
endif

FFLAGS = -g -fsecond-underscore
CFLAGS = -g -fPIC -Wall -O3 -std=c++0x `root-config --cflags` -Iinclude -DREVF
LDLIBS = -lm -lstdc++ -lgsl -lgslcblas `root-config --libs`
LDFLAGS = `root-config --glibs`
ROOT_INC = `root-config --incdir`

ifeq ($(NEW_READOUT), 1)
	LDLIBS += -lncurses
else
	LDLIBS += -lgfortran
endif

# NEED TO REMOVE!!!
CFLAGS += -DUSE_HHIRF

ifeq ($(VERBOSE), 1)
	CFLAGS += -DVERBOSE
endif

# Directories
TOP_LEVEL = $(shell pwd)
DICT_DIR = $(TOP_LEVEL)/dict
INCLUDE_DIR = $(TOP_LEVEL)/include
SOURCE_DIR = $(TOP_LEVEL)/src

OBJ_DIR = $(TOP_LEVEL)/obj
C_OBJ_DIR = $(OBJ_DIR)/c++
DICT_OBJ_DIR = $(DICT_DIR)/obj

FORT_DIR = $(TOP_LEVEL)/scan
FORT_OBJ_DIR = $(OBJ_DIR)/fortran

POLL_INC_DIR = $(PIXIE_SUITE_DIR)/Poll/include
POLL_SRC_DIR = $(PIXIE_SUITE_DIR)/Poll/source

INTERFACE_INC_DIR = $(PIXIE_SUITE_DIR)/Interface/include
INTERFACE_SRC_DIR = $(PIXIE_SUITE_DIR)/Interface/source

TOOL_DIR = $(TOP_LEVEL)/tools
TOOL_SRC_DIR = $(TOOL_DIR)/src

INSTALL_DIR = ~/bin

# Tools
HEX_READ = $(TOOL_DIR)/hexRead
HEX_READ_SRC = $(TOOL_SRC_DIR)/HexRead.cpp
HIS_2_ROOT = $(TOOL_DIR)/his2root
HIS_2_ROOT_SRC = $(TOOL_SRC_DIR)/his2root.cpp
HIS_READER = $(TOOL_DIR)/hisReader
HIS_READER_SRC = $(TOOL_SRC_DIR)/hisReader.cpp
RAW_2_ROOT = $(TOOL_DIR)/raw2root
RAW_2_ROOT_SRC = $(TOOL_SRC_DIR)/raw2root.cpp
LDF_READER = $(TOOL_DIR)/ldfReader
LDF_READER_SRC = $(TOOL_SRC_DIR)/ldfReader.cpp
RAW_VIEWER = $(TOOL_DIR)/rawViewer
RAW_VIEWER_SRC = $(TOOL_SRC_DIR)/rawViewer.cpp
PULSE_VIEWER = $(TOOL_DIR)/pulseViewer
PULSE_VIEWER_SRC = $(TOOL_SRC_DIR)/pulseViewer.cpp

# Main executable
EXECUTABLE = PixieLDF

# Scan libraries
LIBS = $(HHIRF_DIR)/scanorlib.a $(HHIRF_DIR)/orphlib.a\
	   $(ACQ2_LIBDIR)/acqlib.a $(ACQ2_LIBDIR)/ipclib.a

# FORTRAN
FORTRAN =

# C++ CORE
SOURCES = Places.cpp ReadBuffData.RevD.cpp Trace.cpp EventProcessor.cpp MapFile.cpp TraceExtractor.cpp ChanEvent.cpp \
		  ChanIdentifier.cpp Correlator.cpp pugixml.cpp StatsData.cpp SsdProcessor.cpp TreeCorrelator.cpp \
		  DetectorDriver.cpp ParseXml.cpp DetectorLibrary.cpp RandomPool.cpp DetectorSummary.cpp RawEvent.cpp \
		   TimingInformation.cpp PlaceBuilder.cpp HisFile.cpp Plots.cpp PlotsRegister.cpp

ifeq ($(NEW_READOUT), 1)
	SOURCES += NewPixieStd.cpp
else
	FORTRAN += messlog.f mildatim.f scanor.f
	SOURCES += PixieStd.cpp Initialize.cpp TracePlotter.cpp TraceFilterer.cpp
	EXECUTABLE = OldPixieLDF	
endif

# ANALYZERS
SOURCES += CfdAnalyzer.cpp
#SOURCES += DoubleTraceAnalyzer.cpp
SOURCES += FittingAnalyzer.cpp
#SOURCES += TauAnalyzer.cpp
SOURCES += TraceAnalyzer.cpp
SOURCES += WaveformAnalyzer.cpp

# PROCESSORS
#SOURCES += DssdProcessor.cpp
#SOURCES += GeProcessor.cpp
#SOURCES += ImplantSsdProcessor.cpp
SOURCES += IonChamberProcessor.cpp
SOURCES += LiquidProcessor.cpp
SOURCES += LogicProcessor.cpp
#SOURCES += McpProcessor.cpp
#SOURCES += MtcProcessor.cpp
#SOURCES += NeutronProcessor.cpp
#SOURCES += PositionProcessor.cpp
#SOURCES += PulserProcessor.cpp
#SOURCES += ScintProcessor.cpp
#SOURCES += SsdProcessor.cpp
SOURCES += TriggerProcessor.cpp
#SOURCES += ValidProcessor.cpp
SOURCES += VandleProcessor.cpp

FORTOBJ = $(addprefix $(FORT_OBJ_DIR)/,$(FORTRAN:.f=.o))
OBJECTS = $(addprefix $(C_OBJ_DIR)/,$(SOURCES:.cpp=.o))

# This is a special object file included from PixieSuite
HRIBF_SOURCE = $(POLL_SRC_DIR)/hribf_buffers.cpp
HRIBF_SOURCE_OBJ = $(C_OBJ_DIR)/hribf_buffers.o

# This is a special object file included from PixieSuite
SOCKET_SOURCE = $(POLL_SRC_DIR)/poll2_socket.cpp
SOCKET_SOURCE_OBJ = $(C_OBJ_DIR)/poll2_socket.o

# This is a special object file included from PixieSuite
CTERMINAL_SOURCE = $(POLL_SRC_DIR)/CTerminal.cpp
CTERMINAL_SOURCE_OBJ = $(C_OBJ_DIR)/CTerminal.o

# This file is used to handle packing/unpacking of .his files
HIS_FILE_SRC = $(SOURCE_DIR)/HisFile.cpp
HIS_FILE_OBJ = $(C_OBJ_DIR)/HisFile.o

# If UPAK is not used, we need a new main file
SCAN_MAIN = $(SOURCE_DIR)/ScanMain.cpp
SCAN_MAIN_OBJ = $(C_OBJ_DIR)/ScanMain.o

# ROOT dictionary stuff
DICT_SOURCE = RootDict
STRUCT_FILE = Structures

ROOTOBJ = $(DICT_OBJ_DIR)/$(DICT_SOURCE).o 
ROOTOBJ += $(C_OBJ_DIR)/$(STRUCT_FILE).o
SFLAGS = $(addprefix -l,$(DICT_SOURCE))

# Determine what to build
TO_BUILD = $(OBJECTS)
ifeq ($(NEW_READOUT), 1)
	# New scan code
	TO_BUILD += $(SCAN_MAIN_OBJ) $(HRIBF_SOURCE_OBJ) $(SOCKET_SOURCE_OBJ) $(CTERMINAL_SOURCE_OBJ)
else
	# Old scan code
	TO_BUILD += $(FORTOBJ) $(LIBS)
endif

#####################################################################

all: directory $(DICT_OBJ_DIR)/$(DICT_SOURCE).so $(EXECUTABLE)
#	Create all directories, make all objects, and link executable

dictionary: $(DICT_OBJ_DIR) $(DICT_OBJ_DIR)/$(DICT_SOURCE).so
#	Create root dictionary objects

tools: $(HEX_READ) $(HIS_2_ROOT) $(HIS_READER) $(RAW_2_ROOT) $(LDF_READER) $(RAW_VIEWER) $(PULSE_VIEWER)

.PHONY: clean tidy directory

.SECONDARY: $(DICT_DIR)/$(DICT_SOURCE).cpp $(ROOTOBJ)
#	Want to keep the source files created by rootcint after compilation
#	as well as keeping the object file made from those source files

#####################################################################

directory: $(OBJ_DIR) $(FORT_OBJ_DIR) $(C_OBJ_DIR) $(DICT_OBJ_DIR)
# Setup the configuration directory
	@if [ ! -d $(TOP_LEVEL)/config/default ]; then \
		tar -xf $(TOP_LEVEL)/config.tar; \
		echo "Building configuration directory"; \
	fi
# Create a symbolic link to the default config directory
	@if [ ! -e $(TOP_LEVEL)/setup ]; then \
		ln -s $(TOP_LEVEL)/config/default $(TOP_LEVEL)/setup; \
		echo "Creating symbolic link to default configuration directory"; \
	fi

$(OBJ_DIR):
#	Make the object file directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir $@; \
	fi

$(FORT_OBJ_DIR):
#	Make fortran object file directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir $@; \
	fi

$(C_OBJ_DIR):
#	Make c++ object file directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir $@; \
	fi

$(DICT_OBJ_DIR):
#	Make root dictionary object file directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir $@; \
	fi

########################################################################

$(FORT_OBJ_DIR)/%.o: $(FORT_DIR)/%.f
#	Compile fortran source files
	$(FC) -c $(FFLAGS) $< -o $@

$(C_OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
#	Compile C++ source files
	$(CC) -c $(CFLAGS) $< -o $@

$(HRIBF_SOURCE_OBJ): $(HRIBF_SOURCE)
#	Compile hribf_buffers from PixieSuite
	$(CC) -c $(CFLAGS) -I$(POLL_INC_DIR) $< -o $@

$(SOCKET_SOURCE_OBJ): $(SOCKET_SOURCE)
#	Compile poll2_socket from PixieSuite
	$(CC) -c $(CFLAGS) -I$(POLL_INC_DIR) $< -o $@

$(CTERMINAL_SOURCE_OBJ): $(CTERMINAL_SOURCE)
#	Compile poll2_socket from PixieSuite
	$(CC) -c $(CFLAGS) -DUSE_NCURSES -I$(POLL_INC_DIR) -I$(INTERFACE_INC_DIR) $< -o $@

$(SCAN_MAIN_OBJ): $(SCAN_MAIN)
#	Main scan function
	$(CC) -c $(CFLAGS) -DUSE_NCURSES -I$(POLL_INC_DIR) $< -o $@

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

$(EXECUTABLE): $(TO_BUILD) $(ROOTOBJ)
#	Link the executable
	$(LINKER) $(LDFLAGS) $(TO_BUILD) $(ROOTOBJ) -L$(DICT_OBJ_DIR) $(SFLAGS) -o $@ $(LDLIBS)

#####################################################################

$(HEX_READ): $(HEX_READ_SRC)
#	Make the hexReader tool
	$(CC) -O3 -Wall $(HEX_READ_SRC) -o $(HEX_READ)

$(HIS_2_ROOT): $(HIS_2_ROOT_SRC) $(HIS_FILE_OBJ)
#	Make the his2root tool
	$(CC) -O3 -Wall $(HIS_2_ROOT_SRC) $(HIS_FILE_OBJ) -I$(INCLUDE_DIR) `root-config --cflags --glibs` -o $(HIS_2_ROOT)

$(HIS_READER): $(HIS_READER_SRC) $(HIS_FILE_OBJ)
#	Make the hisReader tool
	$(CC) -O3 -Wall $(HIS_READER_SRC) $(HIS_FILE_OBJ) -I$(INCLUDE_DIR) `root-config --cflags --glibs` -o $(HIS_READER)

$(RAW_2_ROOT): $(RAW_2_ROOT_SRC)
#	Make the raw2root tool
	$(CC) -O3 -Wall $(RAW_2_ROOT_SRC) `root-config --cflags --glibs` -o $(RAW_2_ROOT)

$(LDF_READER): $(LDF_READER_SRC) $(HRIBF_SOURCE_OBJ)
#	Make the ldfReader tool
	$(CC) -O3 -Wall $(LDF_READER_SRC) -I$(POLL_INC_DIR) $(HRIBF_SOURCE_OBJ) -o $(LDF_READER)

$(RAW_VIEWER): $(RAW_VIEWER_SRC)
#	Make the rawViewer tool
	$(CC) -O3 -Wall $(RAW_VIEWER_SRC) `root-config --cflags --glibs` -o $(RAW_VIEWER)

$(PULSE_VIEWER): $(PULSE_VIEWER_SRC)
#	Make the rawViewer tool
	$(CC) -O3 -Wall $(PULSE_VIEWER_SRC) `root-config --cflags --glibs` -o $(PULSE_VIEWER)

#####################################################################

install: tools
#	Install tools into the install directory
	@echo "Installing tools to "$(INSTALL_DIR)
	@if [ ! -e $(INSTALL_DIR)/hexRead ]; then ln -s $(HEX_READ) $(INSTALL_DIR)/hexRead; fi
	@if [ ! -e $(INSTALL_DIR)/his2root ]; then ln -s $(HIS_2_ROOT) $(INSTALL_DIR)/his2root; fi
	@if [ ! -e $(INSTALL_DIR)/hisReader ]; then ln -s $(HIS_READER) $(INSTALL_DIR)/hisReader; fi
	@if [ ! -e $(INSTALL_DIR)/raw2root ]; then ln -s $(RAW_2_ROOT) $(INSTALL_DIR)/raw2root; fi
	@if [ ! -e $(INSTALL_DIR)/ldfReader ]; then ln -s $(LDF_READER) $(INSTALL_DIR)/ldfReader; fi
#	@if [ ! -e $(INSTALL_DIR)/rawViewer ]; then ln -s $(RAW_VIEWER) $(INSTALL_DIR)/rawViewer; fi
	@if [ ! -e $(INSTALL_DIR)/pulseViewer ]; then ln -s $(PULSE_VIEWER) $(INSTALL_DIR)/pulseViewer; fi

#####################################################################

tidy: clean_obj

clean: clean_obj clean_dict clean_tools

clean_obj:
	@echo "Cleaning up..."
ifeq ($(NEW_READOUT), 1)
	@rm -f $(FORT_OBJ_DIR)/*.o $(C_OBJ_DIR)/*.o $(EXECUTABLE)
else
	@rm -f $(C_OBJ_DIR)/*.o ./$(EXECUTABLE)
endif
	
clean_dict:
	@echo "Removing ROOT dictionaries..."
	@rm -f $(DICT_DIR)/$(DICT_SOURCE).cpp $(DICT_DIR)/$(DICT_SOURCE).h $(DICT_OBJ_DIR)/*.o  $(DICT_OBJ_DIR)/*.so
	
clean_tools:
	@echo "Removing tools..."
	@rm -f $(HEX_READ) $(HIS_2_ROOT) $(RAW_2_ROOT) $(LDF_READER) $(RAW_VIEWER) $(PULSE_VIEWER)
