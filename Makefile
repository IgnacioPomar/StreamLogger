# Compiler settings
CXX = g++
# Compilation options, enable C++20, position independent code, and warnings
CXXFLAGS = -std=c++20 -fPIC -Wall -Wextra
# Directories
SRC_DIR = src
INCLUDE_DIR = include
TESTER_DIR = tester/src
BUILD_DIR = bin
INSTALL_DIR = /usr/local

# Find all source files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Define the library output file
LIBRARY_OUTPUT = $(BUILD_DIR)/libstreamlogger.so

# Path for the test binary
TESTER_OUTPUT = $(BUILD_DIR)/tester

all: $(LIBRARY_OUTPUT) $(TESTER_OUTPUT)

# Rule for the dynamic library
$(LIBRARY_OUTPUT): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -shared $^ -o $@

# Rule for the test executable
$(TESTER_OUTPUT): $(TESTER_DIR)/tester.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $^ -o $@ -L$(BUILD_DIR) -lstreamlogger

# Install the library and headers
install:
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include/StreamLogger
	cp $(LIBRARY_OUTPUT) $(INSTALL_DIR)/lib
	cp $(INCLUDE_DIR)/* $(INSTALL_DIR)/include/StreamLogger

# Clean compiled objects and binaries
clean:
	rm -rf $(BUILD_DIR)

# Launch the tester
LaunchTest: $(TESTER_OUTPUT)
	LD_LIBRARY_PATH=$(BUILD_DIR) $(TESTER_OUTPUT)

.PHONY: all install clean LaunchTest
