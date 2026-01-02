# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17 -pthread

# Executables
TARGETS = main kasjer zwiedzajacy logger

# Sources
MAIN_SRC = main.cpp
KASJER_SRC = kasjer.cpp
ZWIEDZ_SRC = zwiedzajacy.cpp
LOGGER_SRC = logger.cpp

# Include paths
INCLUDES = -I.

# Default target
all: $(TARGETS)
	@echo "All programs compiled."

# Build main
main: $(MAIN_SRC) shared_defs.h loggerSender.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o main $(MAIN_SRC)

# Build kasjer
kasjer: $(KASJER_SRC) shared_defs.h loggerSender.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o kasjer $(KASJER_SRC)

# Build zwiedzajacy
zwiedzajacy: $(ZWIEDZ_SRC) shared_defs.h loggerSender.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o zwiedzajacy $(ZWIEDZ_SRC)

# Build logger process
logger: $(LOGGER_SRC) shared_defs.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o logger $(LOGGER_SRC)

# Clean build
clean:
	rm -f $(TARGETS) *.o simulation.log

# Optional: create file for ftok
prepare:
	touch logger_file
