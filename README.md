# sudoku
A C++ sudoku solver inspired by Peter Norvig's [constraint propagation](http://norvig.com/sudoku.html) work. Also includes a more direct port by [Pau Fernández](https://github.com/pauek/norvig-sudoku).

A huge plus for this particular solver is that uncommenting the `std::cout` lines (and the `dump` at soduku.cpp:160) will produce a log of what the solver is 'thinking' - hence making its operation clear. Clearer.

## solving sudoku

Assuming the reader knows what sudoku is all about...

Solving a sudoku puzzle is a question of narrowing down possibilities until there are none left. These possibilities exist in two broad axes:

 * Each square contains one symbol in the range 1-9. Initially, all squares can contain any number.
 * Each row, column, and 3x3 block has to contain each symbol in the range 1-9 only once.
 
In the soduku solver contained herein, the per-square state is encapsulated in the `Square` class, and the row/column/block states (also known as unit states) are held in three nine byte booleans (typedef'd to `nine`). The entire state is wrapped into a single `State` object.

It's important to note that each square has twenty peers - squares with which the probability state is shared. For instance, if a square in the middle of the top row is definitely '7', that means that the possibility to hold a '7' is eliminated from all the other squares in that row (8 squares), all the other squares in that column (8 squares), and all the other squares in that block (4 squares).

## seeding

The clues given as part of a soduku puzzle give us certainties as to the symbols on some of the squares. For each of these certainties we can set the square itself to only possibly hold that symbol; propogate the impossibility to hold that symbol to each of the peer squares; and in the process eliminate the possibility for each of the three units to contain that symbol.

After seeding we will have a number of squares that are 'certain', many squares that have a number of possible outcomes, and a statement of which symbols have yet to appear in each row/column/block. We scan across the entire puzzle and create a work list, consisting of the square id and the number of possibilities for each square that is not certain. This work list is then sorted so that squares with the fewest possibilities are considered first.

## solving

The puzzle is solved when we have worked through this list of possibilities and reached the (or possibly a) solution that satisfies all the constraints. This solver uses a simple backtracking algorithm:

 * For the current square, propose each possibility - one at a time. Work through the implications of this possibility (`eliminate_from_possibilities`) until it breaks (or doesn't) any constraints (`breaks_constraint`).
 * When a possibility breaks a constraint, we roll back to the state prior to trying the possibility and try the next one.
 * If we've tried all the possibilities for that square and each breaks a constraint, we roll back to the state prior to even attempting this square and try the next possibility from there instead.
 
 When we have applied this for the entire work list, the puzzle must be solved.
 
 ## the big bikkies
 
The key to solving sudoku quickly is the realisation that, in the process of eliminating possibilities for individual squares, some of these squares will be left with only one possibility and can therefore be regarded as 'certain'. It is therefore valid to recurse the `eliminate_from_possibilities` call using this new certainty (soduku.cpp:80), and so on and so on. Often one giant leap solves 2/3rds the puzzle in one go.

It is also possible to present a self-solving soduku that will have had all its possibilities eliminated as part of the seeding stage. The puzzle `37........852....71..57.4.9..435.9.893.....258.1.276..2.3.94..65....219........42` exhibits this property.
