#include "sudoku.hpp"


Sudoku::Sudoku(const char* init_string)
{
    // initialise the line pointers
    for (uint8_t block_line=0; block_line<3; block_line++)
        for (uint8_t intra_line=0; intra_line<3; intra_line++)
            for (uint8_t across_block=0; across_block<3; across_block++)
                for (uint8_t intra_across=0; intra_across<3; intra_across++)
                    horizontal_lines[block_line*3 + intra_line].positions[across_block*3 + intra_across] =
                    &blocks[block_line*3 + across_block].positions[intra_line*3 + intra_across];
    
    for (uint8_t block_column=0; block_column<3; block_column++)
        for (uint8_t intra_column=0; intra_column<3; intra_column++)
            for (uint8_t down_block=0; down_block<3; down_block++)
                for (uint8_t intra_down=0; intra_down<3; intra_down++)
                    vertical_lines[block_column*3 + intra_column].positions[down_block*3 + intra_down] =
                    &blocks[block_column + down_block*3].positions[intra_column + intra_down*3];
    
    // use the horizontal line pointers to initialise from the string
    for (uint8_t line=0; line<9; line++) {
        Line* p_line=&horizontal_lines[line];
        for (uint8_t col=0; col<9; col++) {
            uint8_t value=0;
            if (*init_string>'0') value=*init_string-'0';
            *p_line->positions[col]=value;
            ++init_string;
        }
    }
}

void Sudoku::dump()
{
    for (uint32_t n=0; n<9; n++) horizontal_lines[n].dump();
}

bool Sudoku::recurse_into(uint8_t block, uint8_t index)
{
    //find the next index and/or block
    ++index;
    if (index==9) {
        index=0;
        ++block;
        if (block==9) {
            // we got to the end without breaking any rules, victory!
            return true;
        }
    }
    
    //memory geometry
    Block* blk=&blocks[block];
    uint8_t first_horizontal_line=(block/3)*3;
    uint8_t horizontal_line_offset=index/3;
    uint8_t this_horizontal_line=first_horizontal_line+horizontal_line_offset;
    uint8_t first_vertical_line=(block%3)*3;
    uint8_t vertical_line_offset=index%3;
    uint8_t this_vertical_line=first_vertical_line+vertical_line_offset;
    
    //for us to try inserting symbols in gaps, the gap needs to be empty
    uint8_t* position_under_test=&blk->positions[index];
    if (*position_under_test==0) {
        uint8_t* symbol_is_used=blk->symbols_used;
        for (uint8_t symbol=1; symbol<10; symbol++) {
            //and the proposed symbol must not have been used before
            if (*symbol_is_used==false) {
                //give it a go
                *position_under_test=symbol;
                *symbol_is_used=true;
                
                //see if we broke anything
                bool line_is_wrong=(horizontal_lines[this_horizontal_line].is_wrong() or vertical_lines[this_vertical_line].is_wrong());
                
                // recurse in...
                if (!line_is_wrong and recurse_into(block, index)) return true; // bail if we won
                
                //oh well
                //(we don't actually need to put the blank symbol back in the 'position' because it'll only be overwritten again)
                *symbol_is_used=false;
            }
            ++symbol_is_used;
        }
        *position_under_test=0;
        return false;
    }
    
    return recurse_into(block, index);
};

bool Line::is_wrong()
{
    // 10 because then we don't have to subtract 1 every time
    bool used[10] = {false, false, false, false, false, false, false, false, false, false};
    for (uint8_t n=0; n<9; n++) {
        uint8_t num=*positions[n];
        if (num){
            if (used[num]) return true;
            used[num] = true;
        }
    }
    return false;
}
    
// dump to stdout
void Line::dump() {
    for (uint32_t n = 0; n < 9; n++) {
        std::cout << static_cast<uint32_t>(*positions[n]) << " ";
    }
    std::cout << std::endl;
}
