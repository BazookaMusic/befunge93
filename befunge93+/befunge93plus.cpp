#include "include/befungeplus.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout.setf(std::ios::unitbuf);
    if (argc < 2) {
        std::cerr << "No file provided. Exiting." << std::endl;
        exit(-1);
    } else if (argc != 2) {
        std::cerr << "Wrong number of arguments. One required," << argc << 
        " given. Exiting" << std::endl;
    }

    char * file_path = argv[1];

    VM vm;
    vm.execute(file_path);


    

    return 0;
}