# Notes on algorithms for graph coloring

## HEAD (Hybrid Evolutionary Algorithm in Duet)

#### Algorithm

* Two threads in parallel find a k-coloring solution using the **TabuCol** algorithm: starting from a coloring (possibly inherited from previous generation), move to a neighboring one (by changing the color of one vertex) with the constraint of not doing the opposite of the past move 
* Solutions of the two threads are merged using **Greedy Partition Crossover (GPX)**: large color classes (many vertices with the same color) should be transmitted to the children
* First two steps are repeated until a valid and good solution is found or the two individuals are the same
* An improvement is to replace at each step one of the two solutions with a solution found previously, to increase diversity

#### Results

* Seems good and reasonably fast
* **Possible problem**: parallelism is not exploited to work synchronously on portions of the same graph but just to have two threads proposing two solutions concurrently



## Parallel Genetic Algorithm (Paper LNCS 3036)

#### Algorithm

* Different threads represent different *islands*, each containing several partial solutions to the coloring problem. At each step, some solutions from different islands are merged according to different crossover algorithms:
  * **Sum-Product Partition Crossover (SPPX)**: pseudocode in the paper
  * **Conflict Elimination Crossover (CEX)**: the block of conflict-free nodes of the first parent is copied, whereas the rest of the graph is copied from the other parent
  * **Union Independent Set Crossover (UISX)**: the intersection of the biggest independent set of the two parents is inherited from the child, the rest is taken from the first parent
  * **Greedy Partition Crossover (GPX)**: see above
* **Mutations** are also used when generating children from parents:
  * **Transposition**: the colors of two randomly selected vertices is inverted
  * **First Fit**: one block is selected and we try to make a conflict-free assignment of its vertices to other blocks. Vertices with no conflict-free assignment remain in the original block
* The quality of a solution (which is used to send the best individuals to other islands) is measured according to a cost function reported in the paper

#### Comments

* Similar approach as HEAD, but this paper is older so it probably performs worse

## Parallel coloring algorithms reported in Allwright paper

#### Luby's algorithm

* Find **Maximal Independent Set (MIS)** at each step and color its vertices in the same way.
* MIS search can be done in parallel: assign a weight to each vertex (with different random permutations at each step), an independent set can be constructed in parallel by choosing all vertices whose weights are local maxima

#### Jones - Plassman

* Similar to Luby, but with some differences:
  * assign a weight to each vertex (will be the same for the whole algorithm)
  * at each step, select any independent set (not necessarily MIS)
  * color each vertex in the IS with the smallest color which is not assigned to any neighboring vertex

#### Largest-Degree-First

* Similar to Jones - Plassman, but:
  * weights are degree of vertices (random number in case of conflicts)
  * colors are assigned in order of decreasing degree

#### Smallest-Degree-Last

* Two phases at each step:
  * *Weighting phase*: find all vertices with degree equal to the smallest degree *d* present in the graph. These are assigned the current weight and removed from the graph. The algorithm repeatedly removes vertices of degree *d* (and, then *d+1* and so on), assigning successively larger weights until all vertices have a weight
  * *Coloring phase*: each vertex looks around at its uncolored neighbors and when it discovers it has the highest weights, it colors itself using the lowest available color in its neighborhood
* Both phases can be parallelized
* Finds better solutions with respect to the other algorithms (smallest number of colors), but it takes longer