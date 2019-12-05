befunge93: befunge93.cpp include/befunge.hpp
	g++ -O3 befunge93.cpp -o befunge93 -Wall -Wextra -Werror

test:
	make clean && make && time ./befunge93 ./tests/test.bf

clean:
	rm befunge93
