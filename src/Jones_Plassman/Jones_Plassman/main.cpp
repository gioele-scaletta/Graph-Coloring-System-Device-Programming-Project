

#include "graph.h"
#include <iostream>


int main() {
	
	clock_t start, end;
	double time_not_parallel, time_parallel;
	graph myGraph = graph();

	myGraph.readFileDIMACS("../../../../benchmark/manual/v100.gra");

	start = clock();
	myGraph.JonesPlassmanColoringParallel();
	end = clock();

	if (myGraph.checkColoring())
		time_parallel = double(end - start) / double(CLOCKS_PER_SEC);
	else
		cout << "Parallel coloring is wrong!" << endl;

	myGraph.cancelColors();

	start = clock();
	myGraph.JonesPlassmanColoring();
	end = clock();

	if (myGraph.checkColoring())
		time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
	else
		cout << "Normal coloring is wrong!" << endl;

	cout << "Non-parallel coloring took " << time_not_parallel << " sec" << endl;
	cout << "Parallel coloring took " << time_parallel << " sec" << endl;
	cout << "Difference: " << (time_not_parallel - time_parallel) << " sec" << endl;


	system("pause");
	return 0;
}






