#!/bin/bash
# Build a root data structure source file and header. This script
# should be used in conjunction with 'make dictionary' to generate
# a root dictionary.

TOP_LEVEL=`pwd`
TOOL_DIR=$TOP_LEVEL/tools/
DICT_DIR=$TOP_LEVEL/dict/
SRC_DIR=$TOP_LEVEL/src/
INC_DIR=$TOP_LEVEL/include/

STRUCT_SOURCE=Structures.cpp
STRUCT_HEADER=Structures.h
LINKFILE=LinkDef.h

BUILDER_EXE=rcbuild

DEF_FILE=$DICT_DIR/def.struct
DEF_FILE_BKP=$DICT_DIR/.def.struct.bkp

# Ensure that the definitions file exists
if [ ! -f $DEF_FILE ]; then
	echo " ERROR: Structure definition file does not exist!"
	
	if [ ! -f $DEF_FILE_BKP ]; then
		echo "  Searching for backup definition file... failed"
		echo "  FATAL ERROR! Failed to find backup file!"
		exit 1
	fi
	
	# Restore the backup of the definitions file
	echo "  Searching for backup definition file... done"
	cp -p $DEF_FILE_BKP $DEF_FILE
else
	# Check if the definitions file has been modified
	if [ ! $DEF_FILE -nt $DEF_FILE_BKP ]; then
		# Check if any of the required files are missing
		if [ -f $SRC_DIR/$STRUCT_SOURCE ] && [ -f $INC_DIR/$STRUCT_HEADER ] && [ -f $DICT_DIR/$LINKFILE ]; then
			exit 0
		fi
	fi

	# Make a backup of the definitions file
	cp -p $DEF_FILE $DEF_FILE_BKP
fi

# Build the builder executable, if needed
if [ ! -f $TOOL_DIR/$BUILDER_EXE ]; then
	g++ -Wall -O2 -o $TOOL_DIR/$BUILDER_EXE -I$TOOL_DIR/include $TOOL_DIR/src/rcbuild.cpp
	echo " Building structures file generator... done"

	# Check that the executable built successfully
	if [ $? -ne 0 ]; then
		echo "  FATAL ERROR! Failed to build "$BUILDER_EXE"!"
		exit $?
	fi
fi

# Rebuild the structure files
$TOOL_DIR$BUILDER_EXE $DEF_FILE --src-dir $SRC_DIR --inc-dir $INC_DIR --dict-dir $DICT_DIR

# Check that the executable exited successfully
if [ $? -ne 0 ]; then
	echo "  ERROR: "$BUILDER_EXE" returned "$?
	exit $?
fi

if [[ FILE1 -nt FILE2 ]]; then
  echo FILE1 is newer than FILE2
fi

exit 0
