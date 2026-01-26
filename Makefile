CXX = g++
CXXFLAGS = -Wall -std=c++11 -fopenmp
TARGET = network_metrics
SRCS = main.cpp networkMetrics.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	
run:
	./$(TARGET)

run-with-id:
	@if [ -z "$(ID)" ]; then \
		echo "Usage: make run-with-id ID=<execution_id>"; \
		echo "Example: make run-with-id ID=run_001"; \
		exit 1; \
	fi
	./$(TARGET) $(ID) 