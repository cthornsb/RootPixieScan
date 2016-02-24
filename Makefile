# Makefile for RootPixieScan
# updated: May 1st, 2015
# Cory Thornsberry

#####################################################################

# Set the PixieSuite directory
PIXIE_SUITE_DIR = /home/cory/Research/pixie16/PixieSuite

# Flag for verbosity
VERBOSE = 0

#####################################################################

FC = gfortran
CC = g++
LINKER = g++

CFLAGS = -g -fPIC -Wall -O3 -std=c++0x `root-config --cflags` -Iinclude -I$(PIXIE_SUITE_INC_DIR) -DREVF
LDLIBS = -lm -lstdc++ -lgsl -lgslcblas `root-config --libs` -L$(PIXIE_SUITE_DIR)/exec/lib -lPixieScan
LDFLAGS = `root-config --glibs`
ROOT_INC = `root-config --incdir`

ifeq ($(VERBOSE), 1)
	CFLAGS += -DVERBOSE
endif

PIXIE_SUITE_INC_DIR = $(PIXIE_SUITE_DIR)/exec/include

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

POLL_INC_DIR = $(PIXIE_SUITE_DIR)/exec/include

TOOL_DIR = $(TOP_LEVEL)/tools
TOOL_SRC_DIR = $(TOOL_DIR)/src
TOOL_INC_DIR = $(TOOL_DIR)/include

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

# C++ CORE
SOURCES = Scanner.cpp Places.cpp Trace.cpp EventProcessor.cpp MapFile.cpp TraceExtractor.cpp ChanEvent.cpp \
		  ChanIdentifier.cpp Correlator.cpp pugixml.cpp StatsData.cpp SsdProcessor.cpp TreeCorrelator.cpp \
		  DetectorDriver.cpp ParseXml.cpp DetectorLibrary.cpp RandomPool.cpp DetectorSummary.cpp RawEvent.cpp \
		   TimingInformation.cpp PlaceBuilder.cpp HisFile.cpp Plots.cpp PlotsRegister.cpp

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

OBJECTS = $(addprefix $(C_OBJ_DIR)/,$(SOURCES:.cpp=.o))

# ROOT dictionary stuff
DICT_SOURCE = RootDict
STRUCT_FILE = Structures

ROOTOBJ = $(DICT_OBJ_DIR)/$(DICT_SOURCE).o 
ROOTOBJ += $(C_OBJ_DIR)/$(STRUCT_FILE).o
SFLAGS = $(addprefix -l,$(DICT_SOURCE))

# Determine what to build
TO_BUILD = $(OBJECTS)

#####################################################################

all: directory dictionary $(EXECUTABLE)
#	Create all directories, make all objects, and link executable

dictionary:
#	Create root dictionary objects
	@$(TOOL_DIR)/rcbuild.sh

tools: directory $(HEX_READ) $(HIS_2_ROOT) $(HIS_READER) $(RAW_2_ROOT) $(LDF_READER) $(RAW_VIEWER) $(PULSE_VIEWER)

.PHONY: clean tidy directory

.SECONDARY: $(DICT_DIR)/$(DICT_SOURCE).cpp $(ROOTOBJ)
#	Want to keep the source files created by rootcint after compilation
#	as well as keeping the object file made from those source files

#####################################################################

directory: $(OBJ_DIR) $(FORT_OBJ_DIR) $(C_OBJ_DIR)
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

$(C_OBJ_DIR):
#	Make c++ object file directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir $@; \
	fi

########################################################################

$(C_OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
#	Compile C++ source files
	$(CC) -c $(CFLAGS) $< -o $@

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
	@ln -s -f $(HEX_READ) $(INSTALL_DIR)/hexRead
	@ln -s -f $(HIS_2_ROOT) $(INSTALL_DIR)/his2root
	@ln -s -f $(HIS_READER) $(INSTALL_DIR)/hisReader
	@ln -s -f $(RAW_2_ROOT) $(INSTALL_DIR)/raw2root
	@ln -s -f $(LDF_READER) $(INSTALL_DIR)/ldfReader
#	@ln -s -f $(RAW_VIEWER) $(INSTALL_DIR)/rawViewer
	@ln -s -f $(PULSE_VIEWER) $(INSTALL_DIR)/pulseViewer

#####################################################################

tidy: clean_obj clean_dict clean_tools

clean: clean_obj

clean_obj:
	@echo "Cleaning up..."
	@rm -f $(C_OBJ_DIR)/*.o $(EXECUTABLE)
	
clean_dict:
	@echo "Removing ROOT dictionaries..."
	@rm -f $(DICT_DIR)/$(DICT_SOURCE).cpp $(DICT_DIR)/$(DICT_SOURCE).h $(DICT_OBJ_DIR)/*.o  $(DICT_OBJ_DIR)/*.so
	@rm -f $(STRUCT_FILE_OBJ) $(SOURCE_DIR)/Structures.cpp $(INCLUDE_DIR)/Structures.h $(DICT_DIR)/LinkDef.h
	
clean_tools:
	@echo "Removing tools..."
	@rm -f $(TOOL_DIR)/rcbuild
	@rm -f $(HEX_READ) $(HIS_2_ROOT) $(RAW_2_ROOT) $(LDF_READER) $(HIS_READER) $(RAW_VIEWER) $(PULSE_VIEWER)
