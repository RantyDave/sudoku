#include "sudoku.hpp"
#include <iostream>
#include <algorithm>

Sudoku::Sudoku(const char* init_string)
{
    for (uint8_t n=0; n<81; n++) {
        // cache geometry
        uint8_t column=n%9;
        uint8_t row=n/9;
        uint8_t block=(row/3)*3+(column/3);
        uint8_t intra_block=(row%3)*3+(column%3);
        geometry[n]={ row, column, block, intra_block };
//        std::cout << "location=" << static_cast<int>(n) <<
//        " column=" << static_cast<int>(column) << " row=" << static_cast<int>(row) <<
//        " block=" << static_cast<int>(block) << " intra_block=" << static_cast<int>(intra_block);
        
        // store position values and keep track of symbols used
        uint8_t symbol=0;
        if ((init_string[n]>='1') && (init_string[n]<='9')) symbol=init_string[n]-'0';
//        std::cout << " symbol=" << static_cast<int>(symbol) << std::endl;
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
    
    // for each unknown position, find out how many known locations are in its lines and block
    for (uint8_t n=0; n<n_empty_positions; n++) {
        Geometry geo { geometry[empty_positions[n].position] };
        empty_positions[n].known=rows[geo.row].n_symbols_used+columns[geo.column].n_symbols_used+blocks[geo.block].n_symbols_used;
    }
    
    // order the empty locations, fewest possibilities first
    std::sort(&empty_positions[0], &empty_positions[n_empty_positions], [](const Empty& a, const Empty& b) { return a.known>b.known; });
}

void Sudoku::dump(bool in_detail)
{
    // dump the puzzle
    for (uint8_t location=0; location<81; location++) {
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
        
        // and unknowns
        for (unsigned int n=0; n<n_empty_positions; n++) {
            int position=empty_positions[n].position;
            std::cout << "Position: " << position
                      << " (row=" << static_cast<int>(geometry[position].row) << " col=" << static_cast<int>(geometry[position].column)
                      << ") known=" << static_cast<int>(empty_positions[n].known) << std::endl;
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
    uint8_t empty_position=0;
    uint8_t last_symbol_tried[81] { 0 };  // the last symbol attempted in this location
    while (empty_position<n_empty_positions) {
        uint8_t location { empty_positions[empty_position].position };
        Geometry geo { geometry[location] };
        
        // try the next symbol in this location
        uint8_t this_symbol { ++last_symbol_tried[location] };
//        std::cout << "location=" << static_cast<int>(location)
//                  << " (row=" << static_cast<int>(geo.row) << " col=" << static_cast<int>(geo.column) << ") ";
        
        // tried all the symbols?
        if (this_symbol==10) {
//            std::cout << "---setting usage to false and stepping back" << std::endl;
            last_symbol_tried[location]=0;
            
            // go back one step
            --empty_position;
            location=empty_positions[empty_position].position;
            geo=geometry[location];
            
            // mark that symbol as unused because we're about to try another
            uint8_t old_symbol=last_symbol_tried[location];
            rows[geo.row].symbols_used[old_symbol-1]=false;
            columns[geo.column].symbols_used[old_symbol-1]=false;
            blocks[geo.block].symbols_used[old_symbol-1]=false;
            blocks[geo.block].positions[geo.intra_block]=0;  // take this off and it goes slower, I kid you not
//            dump(true);
            continue;
        }
        
        // will we break any constraints?
//        std::cout << " this_symbol=" << static_cast<int>(this_symbol);
        if (rows[geo.row].symbols_used[this_symbol-1]) {
//            std::cout << " has already been used in row=" << static_cast<int>(geo.row) << std::endl;
            continue;
        }
        if (columns[geo.column].symbols_used[this_symbol-1]) {
//            std::cout << " has already been used in column=" << static_cast<int>(geo.column) << std::endl;
            continue;
        }
        if (blocks[geo.block].symbols_used[this_symbol-1]) {
//            std::cout << " has already been used in block=" << static_cast<int>(geo.block) << std::endl;
            continue;
        }
        
        // so fill the gap in and attempt to move forwards
//        std::cout << " doesn't break any constraints" << std::endl;
        rows[geo.row].symbols_used[this_symbol-1]=true;
        columns[geo.column].symbols_used[this_symbol-1]=true;
        blocks[geo.block].symbols_used[this_symbol-1]=true;
        blocks[geo.block].positions[geo.intra_block]=this_symbol;
        ++empty_position;
//        dump();
    }
}
