# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17 -pthread

# Include paths
INCLUDES = -I.

# Sources
MAIN_SRC = main.cpp
KASJER_SRC = kasjer.cpp
ZWIEDZ_SRC = zwiedzajacy.cpp
LOGGER_SRC = logger.cpp
MQ_SRC = messageQueue.cpp
LOGGER_SENDER_SRC = loggerSender.cpp

# Objects
MAIN_OBJ = $(MAIN_SRC:.cpp=.o) $(MQ_SRC:.cpp=.o) $(LOGGER_SENDER_SRC:.cpp=.o)
KASJER_OBJ = $(KASJER_SRC:.cpp=.o) $(MQ_SRC:.cpp=.o) $(LOGGER_SENDER_SRC:.cpp=.o)
ZWIEDZ_OBJ = $(ZWIEDZ_SRC:.cpp=.o) $(MQ_SRC:.cpp=.o) $(LOGGER_SENDER_SRC:.cpp=.o)
LOGGER_OBJ = $(LOGGER_SRC:.cpp=.o) $(MQ_SRC:.cpp=.o) $(LOGGER_SENDER_SRC:.cpp=.o)

# Executables
TARGETS = main kasjer zwiedzajacy logger

# Files required by ftok
FTOK_FILES = main_file logger_file

# Default target
all: prepare $(TARGETS)
	@echo "All programs compiled."

# Prepare files for ftok
prepare:
	@for f in $(FTOK_FILES); do \
		if [ ! -f $$f ]; then touch $$f; fi; \
	done

# Build rules
main: $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o main $(MAIN_OBJ)

kasjer: $(KASJER_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o kasjer $(KASJER_OBJ)

zwiedzajacy: $(ZWIEDZ_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o zwiedzajacy $(ZWIEDZ_OBJ)

logger: $(LOGGER_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o logger $(LOGGER_OBJ)

# Generic compilation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TARGETS) *.o simulation.log $(FTOK_FILES)
