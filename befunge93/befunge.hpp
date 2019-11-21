#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <string>

class Stack {
    private:
        static const int size = 2 << 24;
        int curr_index;
        signed long int* contents;
    public:
        Stack(): curr_index(-1), contents(new signed long int[size]) {}
        ~Stack() {
            delete [] contents;
        }

        void push(signed long int item) {

            if (curr_index > size) {
                std::cerr << "Stack overflow" << std::endl;
                exit(-1);
            }
            contents[++curr_index] = item;
        }

        signed long int pop() {
            // push 0, when empty
            if (curr_index < 0) {
                push(0);
            }
            return contents[curr_index--];

        }

        bool empty() {
            return curr_index == -1;
        }

        void dup() {
            // if has more than self explanatory,
            // else add a zero to the top
            curr_index++;
            if (curr_index > 0) {
                contents[curr_index] = contents[curr_index - 1];
            } else {
                contents[curr_index] = 0;
            }
            
        }

        void exchange_two_first() {
            if (curr_index > 0) {
                // has two elements, swap them
                contents[curr_index] ^= contents[curr_index - 1];
                contents[curr_index - 1] ^= contents[curr_index];
                contents[curr_index] ^= contents[curr_index - 1];
            } else if (curr_index == 0) {
                // has one element, add a zero in front
                curr_index++;
                contents[curr_index] = 0;
            } else {
                // has no elements, add two zeros
                curr_index += 2;
                contents[0] = 0;
                contents[1] = 0;

            }
        }
};

enum DIRECTION {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT
};

struct PC {
    
    int x,y;

    static const int maxlimitx = 79;
    static const int maxlimity = 24;

    int limitx = maxlimitx;
    int limity = maxlimity;


    void move(DIRECTION d) {
        switch (d)
        {
        case UP:
            y = y - 1  >= 0 ? y - 1: limity;
            break;
        case DOWN:
            y = y + 1  <= limity ? y + 1: 0;
            break;
        case RIGHT:
            x = x + 1  <= limitx ? x + 1: 0;
            break;
        case LEFT:
            x = x - 1 >= 0 ? x - 1: limitx;
            break;
        default:
            break;
        }
    }


    PC(): x(0), y(0) {}
};



struct pair {
    int first,second;
};

// all valid commands
static const char * charset = "0123456789+-*/%!`><^v?_|\":\\$.,#gp&~@ ";

class VM {
    private:
        unsigned int program[25][80];
        PC pc;
        DIRECTION curr_dir;
        Stack stack;

        



        // transform bytecode to character
        // everything not in valid commands has
        // an offset of 255
        static char bytecode_to_char(unsigned int a) {
            if (a < 1000) {
                 return charset[a];
            } else {
                return (char)(a - 1000);
            }
        }

    
        // convert a char to bytecode to match with labels
        // if char is not a valid command, add 1000 to separate
        // to completely separate it from command bytecode
        // and allow 1-1 conversion
        static unsigned int char_to_bytecode(const char a) {
            switch (a)
            {
            case '0':
                return 0;
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4':
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            case '+':
                return 10;
            case '-':
                return 11;
            case '*':
                return 12;
            case '/':
                return 13;
            case '%':
                return 14;
            case '!':
                return 15;
            case '`':
                return 16;
            case '>':
                return 17;
            case '<':
                return 18;
            case '^':
                return 19;
            case 'v':
                return 20;
            case '?':
                return 21;
            case '_':
                return 22;
            case '|':
                return 23;
            case '"':
                return 24;
            case ':':
                return 25;
            case '\\':
                return 26;
            case '$':
                return 27;
            case '.':
                return 28;
            case ',':
                return 29;
            case '#':
                return 30;
            case 'g':
                return 31;
            case 'p':
                return 32;
            case '&':
                return 33;
            case '~':
                return 34;
            case '@':
                return 35;
            case ' ':
                return 36;
            default:
                return 1000 + a;
            }
        }



    public:
        VM(): pc(PC()), curr_dir(RIGHT) {
            srand(time(NULL));
        }

        void print_program() {
            for (int i = 0; i <= pc.limity; i++) {
                for (int j = 0; j <= pc.limitx; j++) {
                   std::cout << bytecode_to_char(program[i][j]);
                }

                std::cout << std::endl;
            }
        }

        // read program from file, convert to bytecode
        // and return program limits to avoid
        void load_program(const char* input_file_path) {
            std::ifstream program_file(input_file_path);

            int limitx = pc.maxlimitx;
            int limity = pc.maxlimity;

            if (program_file.is_open())
            {
                int i,j;
                i = j = 0;

                char c;

                // initialize program with null
                // instructions
                for (int i = 0; i <= pc.maxlimity; i++) {
                    for (int j = 0; j <= pc.maxlimitx; j++) {
                        program[i][j] = char_to_bytecode(' ');
                    }
                }


                // read program and convert to bytecode
                while (program_file.get(c) && i >= 0 && j >= 0 && j <= limitx && i <= limity)
                {
                    if (c != '\n') {
                        program[i][j] = char_to_bytecode(c);
                    
                        if (bytecode_to_char(char_to_bytecode(c)) != c) {
                            std::cerr << "WRONG CONVERSION:" << c << std::endl;
                        }
                        ++j;
                    } else { 
                        ++i;
                        j = 0;
                    }
                }
                program_file.close();

                if (!(i >= 0 && j >= 0 && j <= pc.maxlimitx && i <= pc.maxlimity)) {
                    std::cerr << "i,j= " << i << "," << j << std::endl;
                    std::cerr << "Not a valid befunge93 file" << std::endl;
                    exit(-1);
                }

            } else {
                std::cerr<< "Unable to open file" << std::endl; 
                exit(-1);
            } 

        }

        void execute(const char* input_file_path) {
            #define NEXT_INS {\
                jump_location = program[pc.y][pc.x];\
                goto *(command_table[jump_location < n_commands? jump_location: n_commands]);}
            
            // indirect threading
            static const void* command_table[] = {
                        &&NUM0_LAB,
                        &&NUM1_LAB,
                        &&NUM2_LAB,
                        &&NUM3_LAB,
                        &&NUM4_LAB,
                        &&NUM5_LAB,
                        &&NUM6_LAB,
                        &&NUM7_LAB,
                        &&NUM8_LAB,
                        &&NUM9_LAB,
                        &&ADD_LAB,
                        &&SUB_LAB,
                        &&MUL_LAB,
                        &&DIV_LAB,
                        &&MOD_LAB,
                        &&NOT_LAB,
                        &&GT_LAB,
                        &&RIGHT_LAB,
                        &&LEFT_LAB,
                        &&UP_LAB,
                        &&DOWN_LAB,
                        &&RAND_LAB,
                        &&HORIF_LAB,
                        &&VERTIF_LAB,
                        &&STRING_LAB,
                        &&DUP_LAB,
                        &&SWAP_LAB,
                        &&POP_LAB,
                        &&OUTI_LAB,
                        &&OUTC_LAB,
                        &&BRIDGE_LAB,
                        &&GET_LAB,
                        &&PUT_LAB,
                        &&INPUTI_LAB,
                        &&INPUTC_LAB,
                        &&END_LAB,
                        &&NULL_LAB,
                        &&INVALID_LAB
            };

            static const int n_commands = 37;


            load_program(input_file_path);

            signed long value1,value2;
            int jump_location;
            char char_buf;

            NEXT_INS;

            ADD_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                stack.push(value1 + value2);
                NEXT_INS;
            SUB_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                stack.push(value1 - value2);
                NEXT_INS;
            MUL_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                stack.push(value1 * value2);
                NEXT_INS;
            DIV_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                if (value2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    exit(-1);
                }
                stack.push(value1 / value2);
                NEXT_INS;
            MOD_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                if (value2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    exit(-1);
                }
                stack.push(value1 % value2);
                NEXT_INS;
            NOT_LAB:
                pc.move(curr_dir);
                value1 = stack.pop();
                stack.push(value1 != 0? 0: 1);
                NEXT_INS;
            GT_LAB:
                pc.move(curr_dir);
                value2 = stack.pop();
                value1 = stack.pop();
                stack.push(value1 > value2? 1 : 0 );
                NEXT_INS;
            RIGHT_LAB:
                curr_dir = RIGHT;
                pc.move(curr_dir);
                NEXT_INS;
            LEFT_LAB:
                curr_dir = LEFT;
                pc.move(curr_dir);
                NEXT_INS;
            UP_LAB:
                curr_dir = UP;
                pc.move(curr_dir);
                NEXT_INS;
            DOWN_LAB:
                curr_dir = DOWN;
                pc.move(curr_dir);
                NEXT_INS;
            RAND_LAB:
                int choice = rand() % 4;
                curr_dir = (DIRECTION)choice;
                pc.move(curr_dir);
                NEXT_INS;
            HORIF_LAB:
                value1 = stack.pop();
                curr_dir = value1 != 0 ? LEFT: RIGHT;
                pc.move(curr_dir);
                NEXT_INS;
            VERTIF_LAB:
                value1 = stack.pop();
                curr_dir = value1 != 0 ? UP: DOWN;
                pc.move(curr_dir);
                NEXT_INS;
            STRING_LAB:
                // skip first "
                pc.move(curr_dir);

                // keep adding to stack until 
                // " is met again
                while(program[pc.y][pc.x] != 24) {
                    // convert back to char
                    stack.push(bytecode_to_char(program[pc.y][pc.x]));
                    pc.move(curr_dir);
                }
                // skip second "                
                pc.move(curr_dir);
                NEXT_INS;
            DUP_LAB:
                pc.move(curr_dir);
                stack.dup();
                NEXT_INS;

            SWAP_LAB:
                pc.move(curr_dir);
                stack.exchange_two_first();
                NEXT_INS;
            
            POP_LAB:
                pc.move(curr_dir);
                stack.pop();
                NEXT_INS;
            
            OUTI_LAB:
                pc.move(curr_dir);
                value1 = stack.pop();
                std::cout << value1;
                NEXT_INS;
            
            OUTC_LAB:
                pc.move(curr_dir);
                value1 = stack.pop();
                std::cout << (char)value1;
                NEXT_INS;
            
            BRIDGE_LAB:
                pc.move(curr_dir);
                pc.move(curr_dir);
                NEXT_INS;
            
            GET_LAB:
                pc.move(curr_dir);
                value1 = stack.pop();
                value2 = stack.pop();

                if (value1 <= pc.limitx && value2 <= pc.limity && 
                    value1 >= 0 && value2 >= 0) {
                        stack.push(bytecode_to_char(program[value1][value2]));
                } else {
                    std::cerr << "GET: Invalid program location access: x=" << value1 << " y=" << value2 << std::endl;
                    exit(-1);
                }


                NEXT_INS;
            PUT_LAB:
               pc.move(curr_dir);
                value1 = stack.pop();
                value2 = stack.pop();

                if (value1 <= pc.limitx && value2 <= pc.limity && 
                    value1 >= 0 && value2 >= 0) {
                        signed long long new_value = stack.pop();

                        if (new_value > 255) {
                            std::cerr << "All program values have to be ascii chars, instead " 
                            << new_value << "was given." << std::endl;

                            exit(-1);
                        }
                        program[value1][value2] = char_to_bytecode(new_value);
                } else {
                    std::cerr << "PUT: Invalid program location access: x=" << value1 << " y=" << value2 << std::endl;
                    exit(-1);
                }
                NEXT_INS;

            INPUTI_LAB:
                pc.move(curr_dir);
                std::cin >> value1;
                stack.push(value1);
                NEXT_INS;
            INPUTC_LAB:
                pc.move(curr_dir);
                std::cin.get(char_buf);
                stack.push((signed long int)char_buf);
                NEXT_INS;
            NUM0_LAB:
                pc.move(curr_dir);
                stack.push(0);
                NEXT_INS;
            NUM1_LAB:
                pc.move(curr_dir);
                stack.push(1);
                NEXT_INS;
            NUM2_LAB:
                pc.move(curr_dir);
                stack.push(2);
                NEXT_INS;
            NUM3_LAB:
                pc.move(curr_dir);
                stack.push(3);
                NEXT_INS;
            NUM4_LAB:
                pc.move(curr_dir);
                stack.push(4);
                NEXT_INS;
            NUM5_LAB:
                pc.move(curr_dir);
                stack.push(5);
                NEXT_INS;
            NUM6_LAB:
                pc.move(curr_dir);
                stack.push(6);
                NEXT_INS;
            NUM7_LAB:
                pc.move(curr_dir);
                stack.push(7);
                NEXT_INS;
            NUM8_LAB:
                pc.move(curr_dir);
                stack.push(8);
                NEXT_INS;
            NUM9_LAB:
                pc.move(curr_dir);
                stack.push(9);
                NEXT_INS;
            NULL_LAB:
                pc.move(curr_dir);
                NEXT_INS;
            END_LAB:
                return;
            INVALID_LAB:
                std::cout << "Invalid command detected << " << bytecode_to_char(program[pc.y][pc.x]) 
                            <<" >> at " << pc.y << "," << pc.x << ". Exiting."<< std::endl;
                exit(-1);
        }
};
