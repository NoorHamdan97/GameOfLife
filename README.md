# GameOfLife
develop a simple version of a thread pool and utilize it to complete a workload in parallel, utilizing synchronization techniques

Life is played on a grid of square cells-like a chess board but extending infinitely in every direction. A cell can be alive or dead. A live cell is shown by putting 
a marker on its square. A dead cell is shown by leaving the square empty. Each cell in the grid has a neighborhood consisting of the eight cells in every direction
including diagonals, each cell belongs to a certain species, and each species is color-coded.

The cycle of life in this game is divided into 2 phases, the first phase is where new cells are created and some cells die :( 
the second phase the cells get to know their lovely neighborhood and change its properties according to the neighborhood it is in.
