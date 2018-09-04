#include "sudoku.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

Sudoku::Sudoku(const char* init_string)
{
    // cache geometry
    for (int n=0; n<81; n++) {
        Square* sqr { &state.squares[n] };
        sqr->row=n/9;
        sqr->column=n%9;
        sqr->block=(sqr->row/3)*3+(sqr->column/3);
        
        // peers
        int peer=0;
        for (int i=0; i<9; i++) if (i!=sqr->column) sqr->peers[peer++]=sqr->row*9+i;
        for (int i=0; i<9; i++) if (i!=sqr->row) sqr->peers[peer++]=sqr->column+i*9;
        for (int y=0; y<3; y++) {
            for (int x=0; x<3; x++) {
                int potential_peer=(sqr->row/3)*27+(sqr->column/3)*3+y*9+x;
                if (potential_peer/9==sqr->row) continue;
                if (potential_peer%9==sqr->column) continue;
                sqr->peers[peer++]=potential_peer;
            }
        }
    }
    
    // store known symbols and keep track which symbols are still possible
    for (int n=0; n<81; n++) {
        if ((init_string[n]>='1') && (init_string[n]<='9')) {
            // known, and therefore the only option
            uint8_t symbol=init_string[n]-'0';
//            std::cout << "square: " << n << " " << sqr->display_location() << " known value: " << init_string[n] << std::endl;
            state.eliminate_from_possibilities(n, symbol);
        }
    }
    
    // form a work list of locations that have more than one possibility and sort them, fewest possibilities first
    for (int n=0; n<81; n++) if (!state.squares[n].is_certain()) work_list.push_back(id_possible_pair(n, state.squares[n].possible));
    std::sort(work_list.begin(), work_list.end(), [](auto a, auto b){return a.second<b.second;});
}

Sudoku::State::State()
{
    // set all the possibles to "yes"
    memset(row_possibles, 1, 81);
    memset(column_possibles, 1, 81);
    memset(block_possibles, 1, 81);
}

bool Sudoku::State::eliminate_from_possibilities(uint8_t square_id, uint8_t symbol)
{
    Square* sqr=&squares[square_id];
    uint8_t symbol_minus_one=symbol-1;
    
    // even half way possible?
    if (breaks_constraint(sqr, symbol_minus_one)) return false;
    
    // make the symbol (temporarily) the only possibility for this square
    sqr->possible=1;
    for (int i=0; i<9; i++) sqr->possibles[i]=(i==symbol_minus_one);
    
    // mark the symbol as having been used in the units
    row_possibles[sqr->row][symbol_minus_one]=false;
    column_possibles[sqr->column][symbol_minus_one]=false;
    block_possibles[sqr->block][symbol_minus_one]=false;

    // remove the possibility from all our peers
    for (int p=0; p<20; p++) {
        uint8_t peer_id=sqr->peers[p];
        Square* peer_square=&squares[peer_id];
        if (peer_square->possibles[symbol_minus_one]) {
            
            // remove the possibility from this peer
            peer_square->possibles[symbol_minus_one]=false;
            --peer_square->possible;
            
            // did we just create a certainty?
            if (peer_square->is_certain()) {
                // recurse down into that certainty, too
//                std::cout << "...implies a new certain square: " << static_cast<int>(sqr->peers[p]) << " " << peer_square->display_location() << " symbol: " << peer_square->display_value() << std::endl;
                if (!eliminate_from_possibilities(peer_id, peer_square->certain_value())) return false;
            }
        }
    }
    return true;
}

bool Sudoku::State::breaks_constraint(const Square* sqr, uint8_t symbol_minus_one)
{
    if (!row_possibles[sqr->row][symbol_minus_one]) {
//        std::cout << "...breaks constraint on row: " << static_cast<int>(sqr->row) << std::endl;
        return true;
    }
    if (!column_possibles[sqr->column][symbol_minus_one]) {
//        std::cout << "...breaks constraint on column: " << static_cast<int>(sqr->column) << std::endl;
        return true;
    }
    if (!block_possibles[sqr->block][symbol_minus_one]) {
//        std::cout << "...breaks constraint on block: " << static_cast<int>(sqr->block) << std::endl;
        return true;
    }
    return false;
}

uint8_t Sudoku::Square::certain_value()
{
    for (int i=0; i<9; i++) if (possibles[i]) return i+1;
    return 0;
}

void Sudoku::solve()
{
    // for each item on the work list
    while (state.item<work_list.size()) {
        uint8_t sqr_id { work_list[state.item].first };
        Square* sqr { &state.squares[sqr_id] };
        
        // if this is now a certainty we can just skip to the next one
//        std::cout << std::endl << "Work list item: " << state.item << " square: " << static_cast<int>(sqr_id) << " " << sqr->display_location();
        if (sqr->is_certain()) {
//            std::cout << " is now a certainty, skipping";
            ++state.item;
            continue;
        }
        
        // find the next possible (if there is one)
        while (sqr->next_possible_to_try!=9 and !sqr->possibles[sqr->next_possible_to_try]) sqr->next_possible_to_try++;
        
        // was this the last one? then we have to step back (restore pre-modified state)
        if (sqr->next_possible_to_try==9) {
            state=std::move(state_replacement_stack.top());
            state_replacement_stack.pop();

            //and loop round
//            std::cout << " no more symbols to try" << std::endl << "...stepping back to item:" << state.item << std::endl;
            sqr->next_possible_to_try=0;
            continue;
        }
        
        // so try the next symbol
        uint8_t symbol=1+sqr->next_possible_to_try++;
//        std::cout << " trying: " << static_cast<int>(symbol);

        // put a snapshot in the stack and apply the change
//        std::cout << std::endl;
        state_replacement_stack.push(state);
        bool success=state.eliminate_from_possibilities(sqr_id, symbol);
        
        // did it stick? If not, roll back and try the next symbol.
        if (!success) {
//            std::cout << "...rolling back" << std::endl;
            state=std::move(state_replacement_stack.top());
            state_replacement_stack.pop();
            continue;
        }
        
        // and go on to the next work item
//        dump();
        ++state.item;
    }
}

void Sudoku::dump(bool in_detail)
{
    // dump the puzzle out with row/column guides
    std::cout << std::endl << "   0 1 2 3 4 5 6 7 8" << std::endl << std::endl;
    for (int n=0; n<81; n++) {
        if (n%9==0) std::cout << n/9 << "  ";
        std::cout << state.squares[n].display_value() << " ";
        if (n%9==8) std::cout << std::endl;
    }
    
    if (in_detail) {
        // details for all the squares
        std::cout << std::endl;
        for (int n=0; n<81; n++) {
            Square* sqr { &state.squares[n] };
            if (sqr->is_certain()) continue;
            std::cout << "square: " << n << " " << sqr->display_location() << std::endl << "   possibles: (" << static_cast<int>(sqr->possible) << ") ";
            for (unsigned int i=0; i<9; i++) if (sqr->possibles[i]) std::cout << i+1 << " ";
            std::cout << std::endl << "   peers: ";
            for (unsigned int i=0; i<8; i++) std::cout << state.squares[sqr->peers[i]].display_location() << " ";
            std::cout << std::endl << "          ";
            for (unsigned int i=8; i<16; i++) std::cout << state.squares[sqr->peers[i]].display_location() << " ";
            std::cout << std::endl << "          ";
            for (unsigned int i=16; i<20; i++) std::cout << state.squares[sqr->peers[i]].display_location() << " ";
            std::cout << std::endl;
        }
        
        // the state of the units
        for (unsigned int n=0; n<9; n++) dumpnine("Row possibles: ", n, state.row_possibles[n]);
        for (unsigned int n=0; n<9; n++) dumpnine("Column possibles: ", n, state.column_possibles[n]);
        for (unsigned int n=0; n<9; n++) dumpnine("Block possibles: ", n, state.block_possibles[n]);
        
        // misc
        std::cout << "work list: ";
        for (auto n: work_list) std::cout << static_cast<int>(n.first) << " ";
        std::cout << std::endl << "state replacements: " << state_replacement_stack.size() << std::endl;
    }
}

void Sudoku::dumpnine(const char* title, int index, const nine& nine)
{
    std::cout << title << index << " = ";
    for (int n=0; n<9; n++)
        if (nine[n]) std::cout << n+1 << " ";
    std::cout << std::endl;
}

std::string Sudoku::Square::display_location()
{
    std::stringstream strm;
    strm << "(r" << static_cast<int>(row) << " c" << static_cast<int>(column) << " b" << static_cast<int>(block) << ")";
    return strm.str();
}

char Sudoku::Square::display_value()
{
    if (possible!=1) return '.';
    for (char n=0; n<9; n++) if (possibles[n]) return '1'+n;
    return 'X';
}
