SOURCES := $(shell find src -type f -name *.cpp)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.cpp=.o))

citrus: $(OBJECTS)
	g++ $(OBJECTS) -o $@

obj/%.o: src/%.cpp $(HEADERS)
	g++ -c -o $@ $< -Wall -g -Iinclude

obj/FastNoise.o: src/FastNoise.cpp
	g++ -c -o $@ $< -Wall -g -O3 -Iinclude

debug: citrus
	gdb citrus

run: citrus
	./citrus

clean:
	rm obj/*.o
	rm citrus

.PHONY: clean
