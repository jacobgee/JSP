CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CLIENT_FILES := $(filter-out obj/Server.o, $(OBJ_FILES))
SERVER_FILES := $(filter-out obj/Client.o, $(OBJ_FILES))
LD_FLAGS := -Wall
CC_FLAGS := -Wall -std=c++0x

all: jserver jclient

jserver: $(SERVER_FILES)
	g++ $(LD_FLAGS) -lpthread -o $(addprefix build/,$@) $^

jclient: $(CLIENT_FILES)
	g++ $(LD_FLAGS) -o $(addprefix build/,$@) $^

obj/%.o: src/%.cpp src
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm build/* obj/*
