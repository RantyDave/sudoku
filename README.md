# sudoku
Yet another sudoku solver

Probably not as simple as it could be, certainly not as fast as possible. I think. I guess I should benchmark it.

## cool bits

Considers the puzzle as objects: the 3x3 blocks of numbers are of class "Block". They are initialised with their numbers, using 0 to mean "gap". It then caches each valid combination of numbers into the block so we can apply the combination with a single memcpy. Extra fun, we can reference each combination with a single uint, and they are contiguously numbered - which makes a whole bunch of things easier.

Then we have a "Row", which is three blocks across and has the permutations for the blocks applied directly; and three Rows make the entire puzzle.

A "Line" is a collection of nine pointers that can be tested to ensure the numbers underneath them are unique; and we can use these for both horizontal and vertical lines across the entire puzzle.

All that happens, then, is that for each row we apply the possible (pre-validated) combinations to the blocks and check to ensure all three horizontal lines across the blocks are valid. If they are, the triplet representing this small-scale successful row is stored. For a second stage these combinations of successful triplets are tried across all three rows until all the vertical lines across the puzzle are valid.

So: each individual 3x3 block is given only valid combinations; the horizontal lines are checked when we make the permutations of 3x3 blocks in a row; and the vertical rows are checked when we apply combinations of triplets - ergo it is solved!
