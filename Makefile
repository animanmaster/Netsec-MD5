# The one Makefile to rule them all
#
# The One Makefile was created by the Dark Lord Sauron 
# during the Second Age in order to gain dominion 
# over the free peoples of Middle-earth. In disguise 
# as Annatar, or "Lord of Gifts", he aided the Elven 
# smiths of Eregion and their leader Celebrimbor in 
# the making of the Makefiles of Power. He then forged 
# the One Makefile himself in the fires of Mount Doom.
#
# Sauron intended it to be the most powerful of all 
# Makefiles, able to rule and control those who wore the 
# others. Since the other Makefiles were themselves powerful, 
# Sauron was obliged to place much of his native power 
# into the One to achieve his purpose.
# 
# Creating the Makefile simultaneously strengthened and 
# weakened Sauron's power. On the one hand, as long 
# as Sauron had the Makefile, he could control the power 
# of all the other Makefiles, and thus he was significantly 
# more powerful after its creation than before; 
# and putting such a great portion of his own power 
# into the Makefile ensured Sauron's continued existence so 
# long as the Makefile existed. On the other hand, by binding 
# his power within the Makefile, Sauron became dependent on 
# it — without it his power was significantly diminished.
#
# Author: Remo Cocco

############################################################
# Configurable Parameters
############################################################

# The name of the program
PROGRAM_NAME=md5

# The directory that houses all of the source
SOURCE_DIR=src
# the extension on the source files
# the Makefile will use this to detect all of the source
SOURCE_EXTENSION=c
# all of the includes dir, sperated by a space
INCLUDES_DIRS=include

# The directory that where all of the build files do
BUILD_DIR=build
# the extension for all fo the build files
BUILD_FILE_EXTENSION=o

# compiler for source
SOURCE_COMPILER=gcc
# flags for source
SOURCE_FLAGS=-c -g -Wall -ansi

# compiler for the objects
OBJECT_COMPILER=gcc
# flags for the objects
OBJECT_FLAGS=-ansi -pthread

############################################################
# Nothing below here should be changed
############################################################

# all of the source files
SOURCE=$(wildcard $(SOURCE_DIR)/*.$(SOURCE_EXTENSION))

# flags for includes
INCLUDES_FLAGS=$(foreach INCLUDE_DIR,$(INCLUDES_DIRS),-I$(INCLUDE_DIR))

# all of the object files
OBJECTS=$(SOURCE:$(SOURCE_DIR)/%.$(SOURCE_EXTENSION)=$(BUILD_DIR)/%.$(BUILD_FILE_EXTENSION))

############################################################
# RULES
############################################################

# The program depends on the build directory and all of the oject files
$(PROGRAM_NAME) : $(BUILD_DIR) $(OBJECTS)
	$(OBJECT_COMPILER) $(OBJECT_FLAGS) $(OPTIMIZATION_FLAGS) $(OBJECTS) -o $(PROGRAM_NAME)

# how to create the build directory
$(BUILD_DIR) :
	mkdir $(BUILD_DIR)

# how to clean the project
clean :
	rm -Rf $(PROGRAM_NAME) $(BUILD_DIR)

# each build file depends on its corresponding source file
$(BUILD_DIR)/%.$(BUILD_FILE_EXTENSION) : $(SOURCE_DIR)/%.$(SOURCE_EXTENSION)
	$(SOURCE_COMPILER) $(SOURCE_FLAGS) $(INCLUDES_FLAGS) $< -o $@

