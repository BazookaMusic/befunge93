#include "include/befunge.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "No file provided. Exiting." << std::endl;
        exit(-1);
    } else if (argc != 2) {
        std::cerr << "Wrong number of arguments. One required," << argc << 
        " given. Exiting" << std::endl;
    }

    std::cout.setf(std::ios::unitbuf);

    char * file_path = argv[1];

    VM vm;
    //vm.load_program(file_path);
    //vm.print_program();
    vm.execute(file_path);
    
    return 0;
}