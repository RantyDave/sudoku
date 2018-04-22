#import <vector>
#import <array>
#import <set>
#import <cstring>
#import <stdio.h>

typedef unsigned int uint;
typedef std::array<uint, 3> triplet;
typedef unsigned short nine[9];

//precompute some factorials
struct Factorials {
    nine factorial;
    Factorials() {
        factorial[0] = 1;
        uint running = 1;
        for (uint n = 1; n < 9; n++) {
            factorial[n] = running;
            running *= (n+1);
        }
    }
};
Factorials f;

struct Block {
    //initialise with list, use 0 to mean 'blank'
    nine positions;
    uint gaps;
    nine gap_indexes;
    nine symbols;
    uint combinations;
    nine* result_block;
    
    Block(uint* in) {
        // copy in, record where the gaps are and which numbers we used.
        gaps=0;
        bool used[9] = {false, false, false, false, false, false, false, false, false};  // first is actually symbol '1'
        for (uint n = 0; n<9; n++) {
            uint next = *(in+n);
            
            // check
            if ((next != 0) && (used[next-1])) {
                printf("Tried to use a number twice\n");
                exit(1);
            }
            
            // go
            positions[n] = next;
            if (next == 0) gap_indexes[gaps++] = n;
            else used[next-1] = true;
        }
        combinations = f.factorial[gaps];
        
        // by looking at which symbols got used, work out which ones must remain
        uint symbol=0;
        for (uint n=1; n < 10; n++) {
            if (not used[n-1]) symbols[symbol++] = n;
        }
        
        // pre-render combinations
        result_block = new nine[combinations];
        for (uint n = 0; n < combinations; n++) {
            render_combination(n);
            memcpy(result_block[n], &positions, sizeof(nine));
        }
    }
    
    ~Block() {
        delete[] result_block;
    }

    void render_combination(uint combination) {
        bool used[9] = {false, false, false, false, false, false, false, false, false};
        
        // go
        for (uint gap = 0; gap < gaps; gap++) {
            // find the major divisor
            uint factorial = f.factorial[(gaps - 1) - gap];
            uint n = combination / factorial;
            combination -= n * factorial;
            
            // skip forward 'n' unused symbols
            uint last_available_symbol = 9999;
            uint symbol_idx = 0;
            uint skip=0;
            do {
                uint this_symbol = symbols[symbol_idx];
                if (not used[this_symbol - 1]) {
                    last_available_symbol = this_symbol;
                    ++skip;
                }
                ++symbol_idx;
                if (symbol_idx == gaps) symbol_idx = 0;  // loop around back to the start
            } while (skip <= n);
            used[last_available_symbol - 1] = true;
            
            // write this symbol in to the gap
            positions[gap_indexes[gap]] = last_available_symbol;
        }
    }
    
    void apply_combination(uint combination) {
        memcpy(positions, result_block[combination], sizeof(nine));
    }
};

Block blocks[9] = {
    Block((uint[]){3, 7, 0, 0, 8, 5, 1, 0, 0}),
    Block((uint[]){0, 0, 0, 2, 0, 0, 5, 7, 0}),
    Block((uint[]){0, 0, 0, 0, 0, 7, 4, 0, 9}),
    Block((uint[]){0, 0, 4, 9, 3, 0, 8, 0, 1}),
    Block((uint[]){3, 5, 0, 0, 0, 0, 0, 2, 7}),
    Block((uint[]){9, 0, 8, 0, 2, 5, 6, 0, 0}),
    Block((uint[]){2, 0, 3, 5, 0, 0, 0, 0, 0}),
    Block((uint[]){0, 9, 4, 0, 0, 2, 0, 0, 0}),
    Block((uint[]){0, 0, 6, 1, 9, 0, 0, 4, 2})
};

struct Row {
    Block *p_b0, *p_b1, *p_b2;
    std::vector<triplet> valid_horizontal_combinations;
    Row(Block* _0, Block* _1, Block* _2) : p_b0(_0), p_b1(_1), p_b2(_2) {};
    
    // apply one of the previously found viable combinations of three that go to make a row
    void apply_row_combination(triplet& comb) {
        p_b0->apply_combination(comb[0]);
        p_b1->apply_combination(comb[1]);
        p_b2->apply_combination(comb[2]);
    }
};

Row rows[3] = {
    Row(&blocks[0], &blocks[1], &blocks[2]),
    Row(&blocks[3], &blocks[4], &blocks[5]),
    Row(&blocks[6], &blocks[7], &blocks[8])
};

struct Line {
    // lines are implemented as pointers to positions in blocks
    unsigned short* positions[9];
    bool is_wrong() {
        // 10 because then we don't have to subtract 1 every time
        bool used[10] = {false, false, false, false, false, false, false, false, false, false};
        for (uint n=0; n<9; n++) {
            uint num=*positions[n];
            if (used[num]) return true;
            used[num] = true;
        }
        return false;
    }
    void dump() {
        for (uint n = 0; n < 9; n++) {
            printf("%u ", *positions[n]);
        }
        printf("\n");
    }
};

Line horizontal_lines[9];
Line vertical_lines[9];

int main(int argc, const char * argv[]) {
    // initialise the lines
    for (uint block_line=0; block_line<3; block_line++)
        for (uint intra_line=0; intra_line<3; intra_line++)
            for (uint across_block=0; across_block<3; across_block++)
                for (uint intra_across=0; intra_across<3; intra_across++)
                    horizontal_lines[block_line*3 + intra_line].positions[across_block*3 + intra_across] =
                        &blocks[block_line*3 + across_block].positions[intra_line*3 + intra_across];

    for (uint block_column=0; block_column<3; block_column++)
        for (uint intra_column=0; intra_column<3; intra_column++)
            for (uint down_block=0; down_block<3; down_block++)
                for (uint intra_down=0; intra_down<3; intra_down++)
                    vertical_lines[block_column*3 + intra_column].positions[down_block*3 + intra_down] =
                        &blocks[block_column + down_block*3].positions[intra_column + intra_down*3];
    
    // list combinations for blocks
    for (uint block=0; block < 9; block++) {
        printf("Block %u has %u combinations\n", block, blocks[block].combinations);
    }
    
    // try combinations of three blocks horizontally to see which produce three valid lines (a successful row)
    uint total_combinations = 1;  // gets multiplied out hence the total starting at 1 (effectively unity)
    for (uint row = 0; row < 3; row++) {
        // reference the current row of three blocks
        Block& block0(blocks[0 + row*3]);
        Block& block1(blocks[1 + row*3]);
        Block& block2(blocks[2 + row*3]);
        Line& line0(horizontal_lines[0 + row*3]);
        Line& line1(horizontal_lines[1 + row*3]);
        Line& line2(horizontal_lines[2 + row*3]);
        
        // triple nested loop tries all the combinations for all the blocks
        // to get a list of three block combintions that make a valid line
        for (uint block0_combination = 0; block0_combination < block0.combinations; block0_combination++) {
            block0.apply_combination(block0_combination);
            for (uint block1_combination = 0; block1_combination < block1.combinations; block1_combination++) {
                block1.apply_combination(block1_combination);
                for (uint block2_combination = 0; block2_combination < block2.combinations; block2_combination++) {
                    block2.apply_combination(block2_combination);
                    
                    // is this a valid combination?
                    if (line0.is_wrong() || line1.is_wrong() || line2.is_wrong()) continue;
                    
                    // win!
                    triplet combination={block0_combination, block1_combination, block2_combination};
                    rows[row].valid_horizontal_combinations.push_back(combination);
                }
            }
        }
        uint row_combinations = (uint)rows[row].valid_horizontal_combinations.size();
        printf("Row %u has %u valid combinations\n", row, row_combinations);
        total_combinations *= row_combinations;
    }
    printf("Searching %u total combinations...\n", total_combinations);
    
    // same again except this time we nest combinations for the rows themselves
    for (auto row0_combination = rows[0].valid_horizontal_combinations.begin();
            row0_combination != rows[0].valid_horizontal_combinations.end(); row0_combination++) {
        rows[0].apply_row_combination(*row0_combination);
        for (auto row1_combination = rows[1].valid_horizontal_combinations.begin();
             row1_combination != rows[1].valid_horizontal_combinations.end(); row1_combination++) {
            rows[1].apply_row_combination(*row1_combination);
            for (auto row2_combination = rows[2].valid_horizontal_combinations.begin();
                 row2_combination != rows[2].valid_horizontal_combinations.end(); row2_combination++) {
                rows[2].apply_row_combination(*row2_combination);
                
                // see if all nine vertical lines think this is OK
                bool good = true;
                for (uint n = 0; (n < 9) && good; n++) {
                    if (vertical_lines[n].is_wrong()) good = false;
                }
                if (good) {
                    for (uint n = 0; n < 9; n++) {
                        horizontal_lines[n].dump();
                    }
                    exit(0);
                }
            }
        }
    }

    return 0;
}
