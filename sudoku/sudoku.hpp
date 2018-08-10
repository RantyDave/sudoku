#ifndef sudoku_hpp
#define sudoku_hpp

#include <cstdint>
#include <iostream>

typedef uint8_t nine[9];

// Takes input strings of the form...
// 37........852....71..57.4.9..435.9.893.....258.1.276..2.3.94..65....219........42

class Sudoku
{
public:
    Sudoku(const char* init_string);
    void dump(bool in_detail=false);
    void solve();
    
private:
    void dumpnine(const char* title, int index, const nine& nine);
    struct Block
    {
        nine positions;
        nine symbols_used {false, false, false, false, false, false, false, false, false};
    };
    
    struct Line
    {
        nine symbols_used {false, false, false, false, false, false, false, false, false};
    };
    
    struct Geometry
    {
        uint8_t column : 4;
        uint8_t row : 4;
        uint8_t block : 4;
        uint8_t intra_block : 4;
    };
    
    Line columns[9];
    Line rows[9];
    Block blocks[9];
    Geometry geometry[81];
    uint8_t empty_positions[81];
    uint8_t n_empty_positions { 0 };
};

#endif /* sudoku_hpp */
