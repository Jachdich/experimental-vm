SOURCES := $(shell find src -type f -name *.cpp)
HEADERS := $(shell find include -type f -name *.h)
OBJECTS := $(patsubst src/%,obj/%,$(SOURCES:.cpp=.o))

citrus: $(OBJECTS)
	g++ $(OBJECTS) -o $@

obj/%.o: src/%.cpp $(HEADERS)
	g++ -c -o $@ $< -Wall -g -Iinclude

debug: citrus
	gdb citrus

run: citrus
	./citrus test.vm

clean:
	rm obj/*.o
	rm citrus

.PHONY: clean
