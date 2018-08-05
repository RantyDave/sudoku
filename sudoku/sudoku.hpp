#ifndef sudoku_hpp
#define sudoku_hpp

#include <cstdint>
#include <iostream>

typedef uint8_t nine[9];

struct Block
{
    nine positions;
    nine symbols_used {false, false, false, false, false, false, false, false, false};
};

// an 'as printed' line - nine positions covering three blocks
struct Line
{
    // lines are implemented as pointers to positions in blocks
    uint8_t* positions[9];
    
    // with the current proposed solution, is this line 'wrong' i.e. uses a given symbol more than once?
    bool is_wrong();
    
    // dump to stdout
    void dump();
};

class Sudoku
{
public:
    Sudoku(const char* init_string);
    void dump();
    bool recurse_into(uint8_t block=0x00, uint8_t index=0xff);
    
private:
    Block blocks[9];
    Line horizontal_lines[9];
    Line vertical_lines[9];
};

#endif /* sudoku_hpp */
