BOT_SRCS := connection.cpp game_logic.cpp main.cpp protocol.cpp game_objs.cpp player.cpp
TEST_SRCS := game_objs.cpp tests.cpp
CXX := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -Ijsoncons/src -g
# jsoncons full of these
CXXFLAGS += -Wno-unused-parameter

LDFLAGS := -lpthread -lboost_system


DEPSFLAGS := -MMD -MP

all: plusbot

clean:
	rm -f plusbot tests *.o *.d

.PHONY: all clean

plusbot: $(BOT_SRCS:.cpp=.o)
	$(CXX) $(LDFLAGS) $^ -o $@

tests: $(TEST_SRCS:.cpp=.o)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DEPSFLAGS) $< -o $@

-include *.d
