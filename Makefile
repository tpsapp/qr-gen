CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2
SRCS = main.cpp qr_renderer.cpp qrcodegen.cpp tinypngout.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = qr-gen

TEST_SRCS = tests/test_domain.cpp qr_renderer.cpp qrcodegen.cpp tinypngout.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = test_domain

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -I. -Itests -o $@ $(TEST_OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I. -Itests -c $< -o $@

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	bash tests/test_regression.sh

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)
