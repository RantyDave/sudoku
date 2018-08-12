#include "sudoku.hpp"
#include <iostream>
#include <algorithm>

Sudoku::Sudoku(const char* init_string)
{
    for (uint8_t n=0; n<81; n++) {
        // cache geometry
        uint_fast8_t column=n%9;
        uint_fast8_t row=n/9;
        uint_fast8_t block=(row/3)*3+(column/3);
        uint_fast8_t intra_block=(row%3)*3+(column%3);
        geometry[n]={ row, column, block, intra_block };
        
        // store position values and keep track of symbols used
        uint_fast8_t symbol=0;
        if ((init_string[n]>='1') && (init_string[n]<='9')) symbol=init_string[n]-'0';
        blocks[block].positions[intra_block]=symbol;
        if (symbol) {
            rows[row].symbols_used[symbol-1]=true;
            ++rows[row].n_symbols_used;
            columns[column].symbols_used[symbol-1]=true;
            ++columns[column].n_symbols_used;
            blocks[block].symbols_used[symbol-1]=true;
            ++blocks[block].n_symbols_used;
        } else {
            // else keep track of the fact that this is an empty position
            empty_positions[n_empty_positions++].position=n;
        }
    }
    
    // for each unknown position, find out what the possibilities are (and count them)
    // and eliminate 
    for (Empty* x=&empty_positions[0]; x<&empty_positions[n_empty_positions]; x++) {
        Geometry geo { geometry[x->position] };
        x->eliminate_for(rows[geo.row].symbols_used);
        x->eliminate_for(columns[geo.column].symbols_used);
        x->eliminate_for(blocks[geo.block].symbols_used);
        x->enlistify();
    }
    
    // order the empty locations, fewest possibilities first
    std::sort(&empty_positions[0], &empty_positions[n_empty_positions], [](const Empty& a, const Empty& b) { return a.possible<b.possible; });
}

void Sudoku::Empty::eliminate_for(const nine& symbols)
{
    for (uint_fast8_t n=0; n<9; n++) {
        if (symbols[n]) possibles[n]=false;
    }
}

void Sudoku::Empty::enlistify()
{
    for (uint_fast8_t n=0; n<9; n++) {
        if (possibles[n]) {
            possibles[possible]=n;
            ++possible;
        }
    }
}

void Sudoku::dump(bool in_detail)
{
    // dump the puzzle
    for (uint_fast8_t location=0; location<81; location++) {
        Geometry geo=geometry[location];
        int symbol=blocks[geo.block].positions[geo.intra_block];
        std::cout << (symbol ? "\033[1;37m" : "\033[0;37m") << symbol << "\033[0;37m ";
        if (location%9==8) std::cout << std::endl;
    }
    
    if (in_detail) {
        // and symbol usage
        for (unsigned int n=0; n<9; n++) dumpnine("Row: ", n, rows[n].symbols_used, rows[n].n_symbols_used);
        for (unsigned int n=0; n<9; n++) dumpnine("Column: ", n, columns[n].symbols_used, columns[n].n_symbols_used);
        for (unsigned int n=0; n<9; n++) dumpnine("Block: ", n, blocks[n].symbols_used, blocks[n].n_symbols_used);
        
        // and possible values
        for (unsigned int n=0; n<n_empty_positions; n++) {
            int position=empty_positions[n].position;
            std::cout << "Position: " << position
                      << " (row=" << static_cast<int>(geometry[position].row) << " col=" << static_cast<int>(geometry[position].column)
                      << ") possible=" << static_cast<int>(empty_positions[n].possible) << " ( ";
            for (unsigned int p=0; p<empty_positions[n].possible; p++) std::cout << static_cast<int>(empty_positions[n].possibles[p]+1) << " ";
            std::cout << ")" << std::endl;
        }
    }

}

void Sudoku::dumpnine(const char* title, int index, const nine& nine, uint8_t symbols_used)
{
    std::cout << "\033[0;30m" << title << index << " = ";
    for (int n=0; n<9; n++) std::cout << (nine[n] ? "\033[1;37m" : "\033[0;37m") << n+1 << "\033[0;37m";
    std::cout << "  used=" << static_cast<int>(symbols_used) <<  "\033[1;37m" << std::endl;
}

void Sudoku::solve()
{
    // keep trying till we get to the end
    uint_fast8_t empty_position { 0 };
    Empty* empty { &empty_positions[empty_position] };
    while (empty_position!=n_empty_positions) {
        // have we tried all the symbols for this location?
        if (empty->next_possible_to_try==empty->possible) {
//            std::cout << "--------------------------------stepping back" << std::endl;
            empty->next_possible_to_try=0;
            
            // go back one step
            --empty_position;
            empty=&empty_positions[empty_position];
            uint_fast8_t location { empty_positions[empty_position].position };
            Geometry* geo { &geometry[location] };
            
            // mark that symbol as unused because we're about to try another
            uint_fast8_t old_symbol_minus_one { empty->possibles[empty->next_possible_to_try-1] };
            rows[geo->row].symbols_used[old_symbol_minus_one]=false;
            columns[geo->column].symbols_used[old_symbol_minus_one]=false;
            blocks[geo->block].symbols_used[old_symbol_minus_one]=false;
            continue;
        }
        
        // suss geometry, fetch symbol
        uint_fast8_t location { empty_positions[empty_position].position };
        Geometry* geo { &geometry[location] };
        uint_fast8_t symbol_minus_one { empty->possibles[empty->next_possible_to_try++] };
//        std::cout << "(row=" << static_cast<int>(geo.row) << " column=" << static_cast<int>(geo.column) << ") " << static_cast<int>(symbol_minus_one+1);
        
        // will we break any constraints?
        if (rows[geo->row].symbols_used[symbol_minus_one]) {
//            std::cout << " has already been used in row=" << static_cast<int>(geo.row) << std::endl;
            continue;
        }
        if (columns[geo->column].symbols_used[symbol_minus_one]) {
//            std::cout << " has already been used in column=" << static_cast<int>(geo.column) << std::endl;
            continue;
        }
        if (blocks[geo->block].symbols_used[symbol_minus_one]) {
//            std::cout << " has already been used in block=" << static_cast<int>(geo.block) << std::endl;
            continue;
        }
        
        // so fill the gap in and attempt to move forwards
//        std::cout << " doesn't break any constraints" << std::endl;
        rows[geo->row].symbols_used[symbol_minus_one]=true;
        columns[geo->column].symbols_used[symbol_minus_one]=true;
        blocks[geo->block].symbols_used[symbol_minus_one]=true;
        blocks[geo->block].positions[geo->intra_block]=symbol_minus_one+1;
        ++empty_position;
        empty=&empty_positions[empty_position];
//        dump();
    }
}

