

all : JsonTest
	./JsonTest

JsonTest : JsonTest.cpp ../src/JsonParserGeneratorRK.cpp ../src/JsonParserGeneratorRK.h libwiringgcc
	gcc JsonTest.cpp ../src/JsonParserGeneratorRK.cpp gcclib/libwiringgcc.a -std=c++11 -lc++ -Igcclib -I../src -o JsonTest

check : JsonTest.cpp ../src/JsonParserGeneratorRK.cpp ../src/JsonParserGeneratorRK.h libwiringgcc
	gcc JsonTest.cpp ../src/JsonParserGeneratorRK.cpp gcclib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -Igcclib -I ../src -o JsonTest && valgrind --leak-check=yes ./JsonTest 

libwiringgcc :
	cd gcclib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc