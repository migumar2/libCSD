CPP=g++
FLAGS=-O0 -g3 -Wall -DNDEBUG -I libcds-v1.0.12/includes/ 
LIB=libcds-v1.0.12/lib/libcds.a

OBJECTS_UTILS=utils/VByte.o
OBJECTS=$(OBJECTS_UTILS) StringDictionary.o StringDictionaryPFC.o Test.o

BIN=Test

%.o: %.cpp
	@echo " [C++] Compiling $<"
	@$(CPP) $(FLAGS) -c $< -o $@

all: 	$(OBJECTS) $(BIN)

Test:	
	$(CPP) $(FLAGS) -o Test $(OBJECTS) ${LIB}

clean:
	@echo " [CLN] Removing object files"
	@rm -f Test $(OBJECTS) *~

