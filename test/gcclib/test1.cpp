#include "Particle.h"

// gcc test1.cpp helpers.cpp spark_wiring_string.cpp spark_wiring_print.cpp -std=c++11 -lc++

int main(int argc, char *argv[]) {
	String foo = "test";
	printf("%s\n", foo.c_str());
	return 0;
}
