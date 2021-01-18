CXX := g++
CXXFLAGS := -Wall -O1 -std=c++17 -I include/ -fsanitize=undefined -fvisibility=hidden
DEBUGFLAGS := -Wall -O0 -g -std=c++17 -I include/ -fsanitize=undefined -fvisibility=hidden
# CXX := clang 
# CXXFLAGS := -std=c++17 -I /Library/Developer/CommandLineTools/usr/include/c++/v1/ -I include/
.PHONY: clean, main, simd

main: main.o  Bitmask.o TNFA.o Parser.o Matcher.o
	rm Bitmask.o
	make Bitmask.o
	$(CXX) main.o Bitmask.o TNFA.o Parser.o Matcher.o $(CXXFLAGS) $(LDFLAGS)
	mv a.out run

simd: main.o  Bitmask.o TNFA.o Parser.o Matcher.o
	rm Bitmask.o
	make Bitmask.o
	$(CXX) main.o Bitmask.o TNFA.o Parser.o Matcher.o $(CXXFLAGS) -mavx2 -DALLOW_SIMD_INSTRUCTION $(LDFLAGS)
	mv a.out run_simd

debug: main.o Bitvector.o Bitmask.o TNFA.o Parser.o Matcher.o
	$(CXX) main.o Bitvector.o Bitmask.o TNFA.o Parser.o Matcher.o $(DEBUGFLAGS) $(LDFLAGS)

main.o: src/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 

TNFA.o: src/TNFA.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 

Parser.o: src/Parser.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 

Bitvector.o: src/Bitvector.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 

Bitmask.o: src/Bitmask.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 

Matcher.o: src/Matcher.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< 



clean:
	$(RM) -r *.o


