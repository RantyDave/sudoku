// Yet another Soduku solver. (c) David Preece 2018
#include "sudoku.hpp"
#include <iostream>
#include <chrono>
#include <memory>

using namespace std;

// original test puzzle
// 37........852....71..57.4.9..435.9.893.....258.1.276..2.3.94..65....219........42
// see also https://raw.githubusercontent.com/horenmar/sudoku-example/master/inputs/benchmark/top95.txt

int main(int argc, const char * argv[]) {
    if (argc!=2) {
        cerr << "Pass the puzzle on the command line" << endl;
        return 1;
    }
    
    auto timer { chrono::high_resolution_clock() };
    auto start { timer.now() };
    Sudoku puzzle { argv[1] };
    puzzle.solve();
    auto end { timer.now() };

    puzzle.dump();
    cout << endl << "Took " << chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "µs" << endl;
    return 0;
}
