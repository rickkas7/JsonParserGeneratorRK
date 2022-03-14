

all : JsonTest
	./JsonTest

JsonTest : JsonTest.cpp ../src/JsonParserGeneratorRK.cpp ../src/JsonParserGeneratorRK.h libwiringgcc
	gcc JsonTest.cpp ../src/JsonParserGeneratorRK.cpp UnitTestLib/libwiringgcc.a -std=c++11 -lc++ -IUnitTestLib -I../src -o JsonTest

check : JsonTest.cpp ../src/JsonParserGeneratorRK.cpp ../src/JsonParserGeneratorRK.h libwiringgcc
	gcc JsonTest.cpp ../src/JsonParserGeneratorRK.cpp UnitTestLib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -IUnitTestLib -I ../src -o JsonTest && valgrind --leak-check=yes ./JsonTest 

libwiringgcc :
	cd UnitTestLib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc