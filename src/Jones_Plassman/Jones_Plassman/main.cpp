

#include "graph.h"
#include <iostream>


int main() {
	
	clock_t start, end;
	double time_not_parallel, time_parallel;
	graph myGraph = graph();
	int color_parallel, color_normal; 

	myGraph.readFileDIMACS("../../../../benchmark/scaleFree/ba10k5d.gra");
	//myGraph.readFileDIMACS("../../../../benchmark/manual/v100.gra");

	start = clock();
	myGraph.JonesPlassmanColoringParallel();
	end = clock();

	color_parallel = myGraph.checkColoring();
	if (color_parallel != -1)
		time_parallel = double(end - start) / double(CLOCKS_PER_SEC);
	else
		cout << "Parallel coloring is wrong!" << endl;
	
	myGraph.cancelColors();

	start = clock();
	myGraph.JonesPlassmanColoring();
	end = clock();

	color_normal = myGraph.checkColoring();
	if (color_normal != -1)
		time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
	else
		cout << "Normal coloring is wrong!" << endl;
		
	cout << "Non-parallel coloring took " << time_not_parallel << " sec and used " << color_normal <<" colors" <<endl;
	cout << "Parallel coloring took " << time_parallel << " sec and used "<< color_parallel << " colors" << endl;
	cout << "Difference: " << (time_not_parallel - time_parallel) << " sec" << endl;


	system("pause");
	return 0;
}






