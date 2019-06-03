all:
	
thread:
	g++ -c -o threads.o threads.cpp -m32
	g++ -o synch_0 tests/synch_0.cpp threads.o -m32

test:
	g++ -o $(name)  tests/$(name).c threads.o -m32

both: thread test

clean:
	rm threads.o
	rm $(name)

clean_test:
	rm $(name)
