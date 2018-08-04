// Yet another Soduku solver. (c) David Preece 2018
#include <emmintrin.h>
#include <vector>
#include <array>
#include <set>
#include <cstring>
#include <cstdint>
#include <iostream>

typedef uint8_t nine[9];

class Block {
public:
    nine positions;
    nine symbols_used {false, false, false, false, false, false, false, false, false};
    
    //initialise with list, use 0 to mean 'blank'
    Block(uint32_t* in) {
        // copy in, record where the gaps are and which numbers we used.
        for (uint32_t n=0; n<9; n++) {
            uint8_t next=*(in+n);
            positions[n]=next;
            if (next!=0) symbols_used[next-1] = true;
        }
    }
};

// the puzzle itself
// *not* input in 'printed' order, each Block is a 3x3 block
// 0 is blank
Block blocks[9] = {
    Block((uint32_t[]){3, 7, 0, 0, 8, 5, 1, 0, 0}),
    Block((uint32_t[]){0, 0, 0, 2, 0, 0, 5, 7, 0}),
    Block((uint32_t[]){0, 0, 0, 0, 0, 7, 4, 0, 9}),
    Block((uint32_t[]){0, 0, 4, 9, 3, 0, 8, 0, 1}),
    Block((uint32_t[]){3, 5, 0, 0, 0, 0, 0, 2, 7}),
    Block((uint32_t[]){9, 0, 8, 0, 2, 5, 6, 0, 0}),
    Block((uint32_t[]){2, 0, 3, 5, 0, 0, 0, 0, 0}),
    Block((uint32_t[]){0, 9, 4, 0, 0, 2, 0, 0, 0}),
    Block((uint32_t[]){0, 0, 6, 1, 9, 0, 0, 4, 2})
};

// an 'as printed' line - nine positions covering three blocks
class Line {
public:
    // lines are implemented as pointers to positions in blocks
    uint8_t* positions[9];
    
    // with the current proposed solution, is this line 'wrong' i.e. uses a given symbol more than once?
    bool is_wrong() {
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
    void dump() {
        for (uint32_t n = 0; n < 9; n++) {
            std::cout << static_cast<uint32_t>(*positions[n]) << " ";
        }
        std::cout << std::endl;
    }
};

Line horizontal_lines[9];
Line vertical_lines[9];

bool recurse_into(uint8_t block=0x00, uint8_t index=0xff)
{
    //find the next index and/or block
    ++index;
    if (index==9) {
        index=0;
        ++block;
        if (block==9) {
            // victory!
            for (uint8_t line=0; line<9; line++) horizontal_lines[line].dump();
            return true;
        }
    }
    Block* blk=&blocks[block];

    //for us to try inserting symbols in gaps, the gap needs to be empty
    if (blk->positions[index]==0) {
        for (uint8_t symbol=0; symbol<9; symbol++) {
            //and the proposed symbol must not have been used before
            if (blk->symbols_used[symbol]==false) {
                //give it a go
                blk->positions[index]=symbol+1;
                blk->symbols_used[symbol]=true;
                
                //see if we broke anything
                uint8_t first_horizontal_line=(block/3)*3;
                uint8_t horizontal_line_offset=index/3;
                uint8_t first_vertical_line=(block%3)*3;
                uint8_t vertical_line_offset=index%3;
                bool line_is_wrong=(horizontal_lines[first_horizontal_line+horizontal_line_offset].is_wrong() or
                                    vertical_lines[first_vertical_line+vertical_line_offset].is_wrong());

                // we've got this far and all the lines have passed, win!
                if ((block==8) and (index==8) and !line_is_wrong) {
                    for (uint32_t n=0; n<9; n++) horizontal_lines[n].dump();
                    return true;
                }
                
                // recurse in...
                if (!line_is_wrong and recurse_into(block, index)) return true; // bail if we won
                
                //oh well
                blk->symbols_used[symbol]=false;
            }
        }
        blk->positions[index]=0;
        return false;
    }
    
    return recurse_into(block, index);
};

// For each 3x3 block find available permutations...
// Find which of these permutations make a valid horizontal line of three blocks
// Then do the same with the permutations of horizontal lines, finding valid verticals.
int main(int argc, const char * argv[]) {
    // initialise the line pointers
    for (uint32_t block_line=0; block_line<3; block_line++)
        for (uint32_t intra_line=0; intra_line<3; intra_line++)
            for (uint32_t across_block=0; across_block<3; across_block++)
                for (uint32_t intra_across=0; intra_across<3; intra_across++)
                    horizontal_lines[block_line*3 + intra_line].positions[across_block*3 + intra_across] =
                        &blocks[block_line*3 + across_block].positions[intra_line*3 + intra_across];

    for (uint32_t block_column=0; block_column<3; block_column++)
        for (uint32_t intra_column=0; intra_column<3; intra_column++)
            for (uint32_t down_block=0; down_block<3; down_block++)
                for (uint32_t intra_down=0; intra_down<3; intra_down++)
                    vertical_lines[block_column*3 + intra_column].positions[down_block*3 + intra_down] =
                        &blocks[block_column + down_block*3].positions[intra_column + intra_down*3];
    
    //go
    recurse_into();
    
    return 0;
}
