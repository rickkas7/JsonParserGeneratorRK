

CFLAGS=-std=c++11

libwiringgcc.a : helpers.o spark_wiring_print.o spark_wiring_string.o 
	ar rcs $@ $^
	
	
test1 : libwiringgcc.a
	gcc test1.cpp $(CFLAGS) libwiringgcc.a -lc++ -o test1
	 
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
        