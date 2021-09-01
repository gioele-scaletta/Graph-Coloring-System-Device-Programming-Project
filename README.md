***********************************************
System and Device Programming

September 3rd, 2021



# PARALLEL GRAPH COLORING



Andrea Cavallo

Gioele Scaletta



### STRUCTURE OF THE PROJECT

The project is composed of several parts:

- 	the main program, contained in the folder `src`, is a C++ program created with
	Visual Studio 2017 on Windows. To open the entire solution, it is enough to click 
	on the .sln file. The program can be compiled, run and debugged using the Visual 
	Studio environment. 
	
-	the results of the simulations are stored in .csv files ("results.csv", 
	"results_best.csv", "memory.csv") contained in the folder `results`
	
- 	the `scripts` folder contains 3 Python scripts that are used to analyze the data 
	of the simulations. In particular:
	- "memory.py" plots data about memory usage
	- "plots.py" plots results of performances vs number of threads and other parameters
	- "tables.py" creates two tables in LaTeX containing the times and the colors for
	each algorithm on many graphs and it creates some plots to summarize the performances
	of the different algorithms
	
- 	plots are stored in the `images` folder

- 	some references are reported in the `references` folder

### STRUCTURE OF THE MAIN PROGRAM

The main C++ program contains a `graph` class with all the variables and methods
needed to read graphs from textual files, run different coloring algorithms and 
analyze the performances. There is also a `main.cpp` file which is divided 
in two sections:
-	first, the performances are analyzed for different number of threads and 
	different number of nodes per threads
-	then, all the algorithms are tested on many graphs using the best values of 
	the two parameters identified before
	The results for both sections are stored in .csv files which are then analyzed
	using Python scripts.

### MORE INFORMATION

Further information about the implemented algorithms, the data structure used
and the parallelization methods is reported in the report.
	
	
	
	
	
	