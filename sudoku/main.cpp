// Yet another Soduku solver. (c) David Preece 2018
#include "sudoku.hpp"
#include <iostream>

// original test puzzle
// 37........852....71..57.4.9..435.9.893.....258.1.276..2.3.94..65....219........42

int main(int argc, const char * argv[]) {
    if (argc!=2) {
        std::cerr << "Pass the puzzle on the command line" << std::endl;
        return 1;
    }
    Sudoku puzzle(argv[1]);
    puzzle.recurse_into();
    puzzle.dump();
    return 0;
}
