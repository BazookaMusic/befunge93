#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <stack>


static const signed long long pointer_mask = 3UL << 63;
static const signed long long not_pointer_mask = ~(3UL << 63);

struct pair {
    signed long long first,second;
};

struct Cell {
    signed long long head,tail;
    bool marked;
    bool free;
    Cell(): marked(false){}
    Cell(signed long long int head, signed long long int tail): head(head), tail(tail), marked(false) {}
};

static Cell * pointer_to_addr(signed long long p) {
    return (Cell *)(p & not_pointer_mask);
}

struct FreeListNode {
    Cell* cell;
    FreeListNode* next;

    FreeListNode(Cell* cell, FreeListNode* next): cell(cell), next(next){}
};

class FreeList {
    private:
        FreeListNode* head;
    
    public:

        FreeList(): head(NULL){}

        ~FreeList() {
            while (head) {
                FreeListNode* temp = head;
                head = head->next;
                delete temp;
            }
        }

        bool empty() {
            return head == NULL;
        }

        void insertFront(Cell* cell) {
            FreeListNode* new_cell = new FreeListNode(cell,head);
            head = new_cell;
        }

        Cell* removeFront() {
            if (empty()) {
                return NULL;
            }

            FreeListNode* temp = head;

            Cell* cell = head->cell;

            head = head->next;

            delete temp;

            return cell;
        }
};

class Heap {
    Cell* cells;
    int curr_index_allocation;
    FreeList free_list;
    static const int capacity = 1 << 24;

    int curr_size;

    public:
        static int max_capacity() {
            return capacity;
        }

        Heap(): cells(new Cell[capacity]),  curr_index_allocation(-1), free_list(FreeList()), curr_size(0) {}

        ~Heap() {
            delete[] cells;
        }

        int size() {
            return curr_size;
        }

        bool hasSpace() {
            return !free_list.empty() || curr_index_allocation < (capacity - 1);
        }

        signed long long int allocate(signed long long int head, signed long long int tail) {
            // full heap
            // start using freelist
            if (curr_index_allocation == (capacity - 1) ) {
                if (!free_list.empty()) {
                    Cell* free_cell = free_list.removeFront();
                    free_cell->head = head;
                    free_cell->tail = tail;
                    free_cell->free = false;
                    free_cell->marked = false;
                    ++curr_size;
                    return (signed long long int)(free_cell) | pointer_mask;
                } else{
                    // OOM
                    std::cerr << "Out of memory" << std::endl;
                    print_contents();
                    exit(-1);

                    return (signed long long)NULL;
                }
            } else { // non empty just insert
                ++curr_index_allocation;
                cells[curr_index_allocation].head = head;
                cells[curr_index_allocation].tail = tail;
                cells[curr_index_allocation].free = false;
                ++curr_size;
                return (signed long long int)(&cells[curr_index_allocation])| pointer_mask;
            }
        }

        // free cell
        void free_cell(Cell* cell){
            --curr_size;
            cell->head = 0xDEADBABE;    // tracker for wrong frees
            cell->tail = 0;
            cell->marked = false;
            cell->free = true;
            free_list.insertFront(cell);
        }

        bool isPointer(signed long long candidate) {
            return (candidate & pointer_mask) != 0;
        }

        void free_unmarked() {
            for (int i = 0; i <= curr_index_allocation; i++) {
                if (!cells[i].free && !cells[i].marked) {
                    free_cell(&cells[i]);
                } else {
                    cells[i].marked = false;
                }
            }
        }

        void print_contents() {
            for (int i = 0; i < curr_index_allocation; i++) {
                std::cout << cells[i].head << std::endl;
            }
        }

};


class GC;

// BEFUNGE STACK
class Stack {
    private:
        friend class GC;
        
        int curr_index;
        signed long long int* contents;
        static const int capacity = 1 << 20;
    public:
   
        Stack(): curr_index(-1), contents(new signed long long int[capacity]) {}
        ~Stack() {
            delete [] contents;
        }

        static int max_capacity() {
            return capacity;
        }

        int size() {
            return curr_index + 1;
        }

        void push(signed long long int item) {

            if (curr_index == capacity) {
                std::cerr << "Stack overflow" << std::endl;
                exit(-1);
            }

            
            ++curr_index;
            contents[curr_index] = item;
        }

        signed long long int pop() {
            // push 0, when empty
            if (curr_index < 0) {
                return 0;
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

        // helper
        void print_stack() {
            std::cout << "STACK BOTTOM" << std::endl;
            for (int i = 0; i <= curr_index; i++) {
                std::cout << contents[i] << std::endl;
            }
            std::cout << "STACK TOP" << std::endl;
        }

        pair top_two() {
            pair tt;
            if (curr_index >= 1) {
                tt.first = contents[curr_index];
                tt.second = contents[curr_index-1];
                
            } else {
                if (curr_index == -1) {
                    tt.first = tt.second = -1;
                } else {
                    tt.first = contents[curr_index];
                    tt.second = -1;
                }
            }

            return tt;
        }

};



// Mark n' Sweep Garbage Collector
class GC {
    Stack& stack;
    Heap& heap;
    Stack pointers; // tracks pointers only

    private:

        void mark(Cell* cell) {
            if (cell->marked) {
                return;
            }

            cell->marked = true;

            if (heap.isPointer(cell->tail)) {
                mark(pointer_to_addr(cell->tail));
            }
        }
        // mark all cells
        void mark_garbage() {
            signed long long* stack_contents = pointers.contents;

            for (int i = 0; i < pointers.size(); i++) {
                if (heap.isPointer(stack_contents[i])) {
                    mark(pointer_to_addr(stack_contents[i]));
                }
            }
        }

        void sweep() {
            heap.free_unmarked();
        }

        void collect_garbage() {
            mark_garbage();
            sweep();
        }
    public:
 
        GC(Stack& stack, Heap& heap): stack(stack), heap(heap) {}

        signed long long pop() {
            signed long long val = stack.pop();

            if (heap.isPointer(val)) {
                pointers.pop();
            }

            return val;
        }

        void push(signed long long val) {
            if (heap.isPointer(val)) {
                pointers.push(val);
            }

            stack.push(val);
        }

        signed long long allocate(signed long long head, signed long long tail) {
            if (!heap.hasSpace()) {
                
                // don't forget to mark pointers we're inserting
                if (heap.isPointer(tail)) {
                    mark(pointer_to_addr(tail));
                }

                if (heap.isPointer(head)) {
                    mark(pointer_to_addr(tail));
                }


                collect_garbage();
            }
            return heap.allocate(head,tail);
        }

        signed long long get_head(signed long long addr) {
            if (!heap.isPointer(addr)) {
                std::cerr << "Invalid pointer access" << std::endl;
                exit(-1);
            }
            return pointer_to_addr(addr)->head;
        }

        signed long long get_tail(signed long long addr) {
            if (!heap.isPointer(addr)) {
                std::cerr << "Invalid pointer access" << std::endl;
                exit(-1);
            }

            return pointer_to_addr(addr)->tail;
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
        default:
            break;
        }
    }


    PC(): x(0), y(0) {}
};




// all valid commands
static const char * charset = "0123456789+-*/%!`><^v?_|\":\\$.,#gp&~@cht ";

class VM {
    private:
        unsigned int program[25][80];
        PC pc;
        DIRECTION curr_dir;
        Stack stack;
        Heap heap;

        GC gc;

        



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
        // if char is not a valid command, add 255 to separate
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
            case 'c':
                return 36;
            case 'h':
                return 37;
            case 't':
                return 38;
            case ' ':
                return 39;
            default:
                return 1000 + a;
            }
        }



    public:
        VM(): pc(PC()), curr_dir(RIGHT), gc(GC(stack,heap)) {
            srand(time(NULL));
        }

        void print_program() {
            for (int i = 0; i <= pc.limity; i++) {
                for (int j = 0; j <= pc.limitx; j++) {
                   std::cout << bytecode_to_char(program[i][j]);
                }
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
                        &&CONS_LAB,
                        &&HEAD_LAB,
                        &&TAIL_LAB,
                        &&NULL_LAB,
                        &&INVALID_LAB
            };

            static const int n_commands = 40;


            load_program(input_file_path);
            

            signed long long value1,value2;
            int jump_location;
            char char_buf;

            NEXT_INS;

            ADD_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
                stack.push(value1 + value2);
                NEXT_INS;
            SUB_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
                stack.push(value1 - value2);
                NEXT_INS;
            MUL_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
                stack.push(value1 * value2);
                NEXT_INS;
            DIV_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
                if (value2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    exit(-1);
                }
                stack.push(value1 / value2);
                NEXT_INS;
            MOD_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
                if (value2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    exit(-1);
                }
                stack.push(value1 % value2);
                NEXT_INS;
            NOT_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();
                stack.push(value1 != 0? 0: 1);
                NEXT_INS;
            GT_LAB:
                pc.move(curr_dir);
                value2 = gc.pop();
                value1 = gc.pop();
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
                value1 = gc.pop();
                curr_dir = value1 == 0 ? RIGHT: LEFT;
                pc.move(curr_dir);
                NEXT_INS;
            VERTIF_LAB:
                value1 = gc.pop();
                curr_dir = value1 == 0 ? DOWN: UP;
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
                gc.pop();
                NEXT_INS;
            
            OUTI_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();
                std::cout << value1;
                NEXT_INS;
            
            OUTC_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();
                std::cout << (char)value1;
                NEXT_INS;
            
            BRIDGE_LAB:
                pc.move(curr_dir);
                pc.move(curr_dir);
                NEXT_INS;
            
            GET_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();
                value2 = gc.pop();

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
                value1 = gc.pop();
                value2 = gc.pop();

                if (value1 <= pc.limitx && value2 <= pc.limity && 
                    value1 >= 0 && value2 >= 0) {
                        signed long long new_value = gc.pop();

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
                stack.push((signed long long int)char_buf);
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
            CONS_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();
                value2 = gc.pop();
                signed long long val =  gc.allocate(value2,value1);
                stack.push(val);
                NEXT_INS;
            HEAD_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();

                if (heap.isPointer(value1)) {
                    long long val = gc.get_head(value1);
                    stack.push(val);
                } else {
                    std::cerr << "Invalid dereference " << value1 << std::endl;
                    exit(-1);
                }
                NEXT_INS;

            TAIL_LAB:
                pc.move(curr_dir);
                value1 = gc.pop();

                if (heap.isPointer(value1)) {
                    stack.push(gc.get_tail(value1));
                } else {
                    std::cerr << "Invalid dereference " << value1 << std::endl;
                    exit(-1);
                }

                NEXT_INS;

            INVALID_LAB:
                std::cout << "Invalid command detected << " << bytecode_to_char(program[pc.y][pc.x]) 
                            <<" >> at " << pc.y << "," << pc.x << ". Exiting."<< std::endl;
                exit(-1);
        }
};
