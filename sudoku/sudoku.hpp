#ifndef sudoku_hpp
#define sudoku_hpp

#include <cstdint>
#include <iostream>

typedef uint_fast8_t nine[9];

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
        nine symbols_used { false, false, false, false, false, false, false, false, false };
    };
    
    struct Line
    {
        nine symbols_used { false, false, false, false, false, false, false, false, false };
    };
    
    struct Empty
    {
        uint_fast8_t position;
        uint_fast8_t possible { 9 };  // number of possibles (initially), needed for sorting
        uint_fast8_t next_possible_to_try { 0 };
        nine possibles { true, true, true, true, true, true, true, true, true };
        void eliminate_for(const nine& symbols);
    };
    
    struct Geometry
    {
        uint_fast8_t row;
        uint_fast8_t column;
        uint_fast8_t block;
        uint_fast8_t intra_block;
        uint_fast8_t adjacent_location_horiz;  // 0 means "doesn't have one"
        uint_fast8_t adjacent_location_vert;
    };
    
    Line rows[9];
    Line columns[9];
    Block blocks[9];
    Geometry geometry[81];
    Empty empty_positions[81];
    uint_fast8_t n_empty_positions { 0 };
};

#endif /* sudoku_hpp */
