#ifndef sudoku_hpp
#define sudoku_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

typedef uint8_t nine[9];
typedef std::pair<uint8_t, uint8_t> id_possible_pair;

// Takes input strings of the form...
// 37........852....71..57.4.9..435.9.893.....258.1.276..2.3.94..65....219........42

class Sudoku
{
public:
    Sudoku(const char* init_string);
    void dump(bool in_detail=false);
    void solve();
    
private:
    struct Square
    {
        uint8_t row;
        uint8_t column;
        uint8_t block;
        uint8_t possible { 9 };  // number of possibles (initially), needed for sorting
        nine possibles { true, true, true, true, true, true, true, true, true };
        uint8_t next_possible_to_try { 0 };
        uint8_t peers[20];
        
        bool is_certain() { return possible==1; };
        uint8_t certain_value();
        std::string display_location();
        char display_value();
    };
    
    struct State
    {
        State();
        bool breaks_constraint(const Square* sqr, uint8_t symbol_minus_one);
        bool eliminate_from_possibilities(uint8_t square_id, uint8_t symbol);  // true if succeeded
        
        // work list item id is stored so we can skip back over 'now certain' squares
        int item { 0 };

        // the squares themselves
        Square squares[81];
        
        // unit possibilities
        nine row_possibles[9];
        nine column_possibles[9];
        nine block_possibles[9];
    };
    State state;
    
    // work ordering and undo
    std::vector<id_possible_pair> work_list;
    std::stack<State> state_replacement_stack;
    
    // misc
    void dumpnine(const char* title, int index, const nine& nine);
};

#endif /* sudoku_hpp */
