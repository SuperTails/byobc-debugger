CPP_FLAGS = -std=c++1z -g

test: test.o physicalw65c02.o
	clang++ -o test test.o physicalw65c02.o

nestest: nestest.o physicalw65c02.o
	clang++ -o nestest nestest.o physicalw65c02.o

test.o: ../json.h test.cpp physicalw65c02.h
	clang++ -o test.o -c test.cpp $(CPP_FLAGS)

nestest.o: run_nestest.cpp physicalw65c02.h
	clang++ -o nestest.o -c run_nestest.cpp $(CPP_FLAGS)

physicalw65c02.o: physicalw65c02.h physicalw65c02.cpp
	clang++ -o physicalw65c02.o -c physicalw65c02.cpp $(CPP_FLAGS)

.PHONY: clean
clean:
	rm -f test test.o physicalw65c02.o