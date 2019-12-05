befunge93plus: befunge93plus.cpp include/befungeplus.hpp
	g++ -O3 befunge93plus.cpp -o befunge93plus -Wall -Wextra -Werror

test:
	make clean && make && time ./befunge93plus ./tests/pp.b

clean:
	rm befunge93plus
