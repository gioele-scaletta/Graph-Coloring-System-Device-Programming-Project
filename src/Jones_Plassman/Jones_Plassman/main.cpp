

#include "graph.h"
#include <iostream>


int main() {
	
	clock_t start, end;
	double time_not_parallel;
	
	int color_parallel;
	double time, score;
	string graph_sample, graph_sample_path;
	double finalscoreJonesPlassmanColoringParallelQueueCounter=0, finalscoreJonesPlassmanColoringParallelQueueVcetor=0, finalscoreJonesPlassmanColoringParallelBarriers=0, finalscoreJonesPlassmanColoringParallelVector=0, finalscoreJonesPlassmanColoringParallelOnenodeThread=0, finalscoreLargestDegreeFirst = 0;
	double finaltimeJonesPlassmanColoringParallelQueueCounter = 0, finaltimeJonesPlassmanColoringParallelQueueVcetor = 0, finaltimeJonesPlassmanColoringParallelBarriers = 0, finaltimeJonesPlassmanColoringParallelVector = 0, finaltimeJonesPlassmanColoringParallelOnenodeThread = 0, finaltimeLargestDegreeFirst=0;

	map<string, string> GRAPHS = {{"citeseer_sub_10720.gra", "../../../../benchmark/small_dense_real/citeseer_sub_10720.gra"}, {"ba10krd", "../../../../benchmark/scaleFree/ba10k5d.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../benchmark/manual/v100.gra"},
	/*{"citeseer.scc.gra", "../../../../../../benchmark/large/citeseer.scc.gra"}*/ //NOT WORKING
	};

	map<string, string>::iterator it;
	for (it = GRAPHS.begin(); it != GRAPHS.end(); it++) {

		graph myGraph = graph();

		graph_sample = it->first;
		graph_sample_path = it->second;

		cout << "-------------------------------------------------" << endl;
		cout << "Loading " << graph_sample << endl;

		myGraph.readFileDIMACS(graph_sample_path);

		cout << "Results for graph:" << graph_sample << endl;

		/*Greedy Sequential*/
		start = clock();
		myGraph.GreedySequential();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nGreedy Sequential Coloring:" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Largest Degree First Standard*/
		start = clock();
		myGraph.LargestDegreeFirstStandard();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nLargest Degree First standard:" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Largest Degree First with overlaps*/

		start = clock();
		myGraph.LargestDegreeFirst();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nLargest Degree First with overlaps:" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Smallest Degree Last*/
		start = clock();
		myGraph.SmallestDegreeLastStandard();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nSmallest Degree Last:" << endl;
		if (color_parallel != -1) {
			time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << double(end - start) / double(CLOCKS_PER_SEC) << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman standard*/
		start = clock();
		myGraph.JonesPlassmanColoringParallelStandard();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nJones-Plassman standard:" << endl;
		if (color_parallel != -1) {
			time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << double(end - start) / double(CLOCKS_PER_SEC) << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman with overlaps*/
		start = clock();
		myGraph.JonesPlassmanColoringParallelQueueCounter();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\nJones-Plassman standard:" << endl;
		if (color_parallel != -1) {
			time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << double(end - start) / double(CLOCKS_PER_SEC) << " sec and used " << color_parallel << " colors" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

	}

	system("pause");
	return 0;
}






