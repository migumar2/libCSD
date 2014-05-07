CPP=g++
FLAGS=-O9 -Wall -DNDEBUG -I libcds/includes/ 
LIB=libcds/lib/libcds.a

OBJECTS_CODER=utils/Coder/StatCoder.o utils/Coder/DecodingTableBuilder.o utils/Coder/DecodingTable.o utils/Coder/DecodingTree.o utils/Coder/BinaryNode.o
OBJECTS_UTILS=utils/VByte.o utils/LogSequence.o utils/DAC_VLS.o utils/DAC_BVLS.o $(OBJECTS_CODER) 
 
OBJECTS_HUTUCKER=HuTucker/HuTucker.o
OBJECTS_REPAIR=RePair/Coder/arrayg.o RePair/Coder/basics.o RePair/Coder/hash.o RePair/Coder/heap.o RePair/Coder/records.o RePair/Coder/dictionary.o RePair/Coder/IRePair.o RePair/Coder/CRePair.o RePair/RePair.o
OBJECTS_HASH=Hash/Hash.o Hash/HashDAC.o Hash/Hashdh.o Hash/HashBdh.o Hash/HashBBdh.o
OBJECTS_HUFFMAN=Huffman/huff.o Huffman/Huffman.o
OBJECTS_FMINDEX=FMIndex/SuffixArray.o FMIndex/SSA.o
OBJECTS_XBW=XBW/TrieNode.o XBW/XBW.o  
OBJECTS=$(OBJECTS_UTILS) $(OBJECTS_HUTUCKER) $(OBJECTS_HUFFMAN) $(OBJECTS_REPAIR) $(OBJECTS_HASH) $(OBJECTS_XBW) $(OBJECTS_FMINDEX) StringDictionary.o StringDictionaryHASHHF.o StringDictionaryHASHRPF.o StringDictionaryHASHUFFDAC.o StringDictionaryHASHRPDAC.o StringDictionaryPFC.o StringDictionaryRPFC.o StringDictionaryHTFC.o StringDictionaryHHTFC.o StringDictionaryRPHTFC.o StringDictionaryRPDAC.o StringDictionaryXBW.o StringDictionaryFMINDEX.o
EXES=Build.o Test.o

BIN=Build Test

%.o: %.cpp
	@echo " [C++] Compiling $<"
	@$(CPP) $(FLAGS) -c $< -o $@

all: clean $(OBJECTS) $(EXES) $(BIN)
	@echo " [MSG] Done compiling tests"
	@echo " [FLG] $(FLAGS)"
	
Build:	
	$(CPP) $(FLAGS) -o Build Build.o $(OBJECTS) ${LIB}
	
Test:	
	$(CPP) $(FLAGS) -o Test Test.o $(OBJECTS) ${LIB}
 

clean:
	@echo " [CLN] Removing object files"
	@rm -f  $(BIN) $(OBJECTS) $(EXES) *~ iterators/*~ FMIndex/*~ Hash/*~ Huffman/*~ RePair/*~ utils/*~ XBW/*~ 

