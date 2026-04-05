CXX = g++
CXXFLAGS = -Wall -std=c++11 -fopenmp
TARGET = network_metrics
SRCS = main.cpp networkMetrics.cpp
OBJS = $(SRCS:.cpp=.o)

# Test configuration
TEST_TARGET = tests/test_network
TEST_SRC = tests/test_network.cpp main.cpp networkMetrics.cpp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(TEST_TARGET)
	rm -f tests/*.o

# ── Run the main program ────────────────────────────────────────────────
run:
	./$(TARGET)

run-with-id:
	@if [ -z "$(ID)" ]; then \
		echo "Usage: make run-with-id ID=<execution_id>"; \
		echo "Example: make run-with-id ID=run_001"; \
		exit 1; \
	fi
	./$(TARGET) $(ID)

# ── Tests ────────────────────────────────────────────────────────────────
test: test-cpp test-python

test-cpp: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) networkMetrics.h
	$(CXX) $(CXXFLAGS) -O2 -DTESTING -I. -o $@ tests/test_network.cpp main.cpp networkMetrics.cpp

PYTHON ?= $(shell command -v python3 2>/dev/null || echo python)

test-python:
	$(PYTHON) -m pytest tests/test_network_generator.py -v

# ── Help ─────────────────────────────────────────────────────────────────
help:
	@echo "Available targets:"
	@echo "  make            Build the main program"
	@echo "  make run        Run the main program"
	@echo "  make test       Run all tests (C++ and Python)"
	@echo "  make test-cpp   Run C++ unit tests only"
	@echo "  make test-python Run Python unit tests only"
	@echo "  make clean      Remove build artifacts"
	@echo "  make help       Show this message"

.PHONY: all clean run run-with-id test test-cpp test-python help
