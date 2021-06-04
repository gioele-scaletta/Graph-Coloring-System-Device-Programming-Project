

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

	map<string, string> GRAPHS = { /*{"citeseer_sub_10720.gra", "../../../../../../benchmark/small_dense_real/citeseer_sub_10720.gra"}/*, {"ba10krd", "../../../../../../benchmark/scaleFree/ba10k5d.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../../../benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../../../benchmark/manual/v100.gra"},*/
	{"citeseer.scc.gra", "../../../../../../benchmark/large/citeseer.scc.gra"} //NOT WORKING
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



		start = clock();
		myGraph.SmallDegreeFirstStandard();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring Smallest Degree Firsat standard:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
			//	finalscoreLargestDegreeFirst += score;
			//	finaltimeLargestDegreeFirst += time;

		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();



		start = clock();
		myGraph.LargestDegreeFirstStandard();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring Largest Degree Firsat standard:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
			//	finalscoreLargestDegreeFirst += score;
			//	finaltimeLargestDegreeFirst += time;

		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();





	



		//3.0
		start = clock();
		myGraph.LargestDegreeFirst();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring LargeDreeFirst:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
			finalscoreLargestDegreeFirst += score;
			finaltimeLargestDegreeFirst += time;

		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		//1
		start = clock();
		myGraph.JonesPlassmanColoring();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoring:\n" << endl;
		if (color_parallel != -1) {
			time_not_parallel = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << double(end - start) / double(CLOCKS_PER_SEC) << " sec and used " << color_parallel << " colors" << endl;
			cout << "\nTime perc wrt notpar: " << (double(end - start) / double(CLOCKS_PER_SEC) / time_not_parallel) * 100 << "%" << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;


		myGraph.cancelColors();




	
		/*

		//3.0
		start = clock();
		myGraph.JonesPlassmanColoringParallel();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassman Prima MIS poi Coloro:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
		//	finalscoreLargestDegreeFirst += score;
		//	finaltimeLargestDegreeFirst += time;

		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		*/

		/*
		//3
		start = clock();
		myGraph.JonesPlassmanColoringParallelQueueVector();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoringParallelQueueVector:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
			finalscoreJonesPlassmanColoringParallelQueueVcetor += score;
			finaltimeJonesPlassmanColoringParallelQueueVcetor += time;

		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();
		*/

		//2
		start = clock();
		myGraph.JonesPlassmanColoringParallelQueueCounter();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoringParallelQueueCounter:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Coloring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << score << "%" << endl;
			finalscoreJonesPlassmanColoringParallelQueueCounter += score;
			finaltimeJonesPlassmanColoringParallelQueueCounter += time;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		


		
		//4
		start = clock();
		myGraph.JonesPlassmanColoringParallelBarriers();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoringParallelBarriers:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "\nColoring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " << (time / time_not_parallel) * 100 << "%" << endl;
			finalscoreJonesPlassmanColoringParallelBarriers += score;
			finaltimeJonesPlassmanColoringParallelBarriers += time;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		//5
		start = clock();
		myGraph.JonesPlassmanColoringParallelVector();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoringParallelVector:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "\nColoring took " << time << " sec and used " << color_parallel << " colors" << endl;
			score = (time / time_not_parallel) * 100;
			cout << "\nTime perc wrt notpar: " <<score << "%" << endl;
			finalscoreJonesPlassmanColoringParallelVector += score;
			finaltimeJonesPlassmanColoringParallelVector += time;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		//5
		start = clock();
		myGraph.JonesPlassmanColoringParallelOneNodeThread();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << "\n\nColoring JonesPlassmanColoringParallelOnenodeThread:\n" << endl;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "\nColoring took " << time << " sec and used " << color_parallel << " colors" << endl;
			cout << "\nTime perc wrt notpar: " << score << "%" << endl;
			finalscoreJonesPlassmanColoringParallelOnenodeThread += score;
			finalscoreJonesPlassmanColoringParallelOnenodeThread += time;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();






	}
	cout << "\nFinal score JonesPlassmanColoringParallelQueueCounter: " << finalscoreJonesPlassmanColoringParallelQueueCounter << endl;
	cout << "\nFinal time JonesPlassmanColoringParallelQueueCounter: " << finaltimeJonesPlassmanColoringParallelQueueCounter << endl;
	cout << "\n\nFinal score JonesPlassmanColoringParallelBarriers: " << finalscoreJonesPlassmanColoringParallelBarriers << endl;
	cout << "\nFinal time JonesPlassmanColoringParallelBarriers: " << finaltimeJonesPlassmanColoringParallelBarriers << endl;
	cout << "\n\nFinal score JonesPlassmanColoringParallelVector: " << finalscoreJonesPlassmanColoringParallelVector << endl;
	cout << "\nFinal time JonesPlassmanColoringParallelVector: " << finaltimeJonesPlassmanColoringParallelVector << endl;
	cout << "\n\nFinal score JonesPlassmanColoringParallelOneNodeOneThread: " << finalscoreJonesPlassmanColoringParallelOnenodeThread << endl;
	cout << "\nFinal time JonesPlassmanColoringParallelOneNodeOneThread: " << finaltimeJonesPlassmanColoringParallelOnenodeThread << endl;
	cout << "\n\nFinal score JonesPlassmanColoringParallelQueueVector: " <<finalscoreJonesPlassmanColoringParallelQueueVcetor << endl;
	cout << "\nFinal time JonesPlassmanColoringParallelQueueVector: " << finaltimeJonesPlassmanColoringParallelQueueVcetor << endl;
	cout << "\n\nFinal score LargestDegreeFirst: " << finalscoreLargestDegreeFirst << endl;
	cout << "\nFinal time LargestDegreeFirst: " << finaltimeLargestDegreeFirst << endl;



	system("pause");
	return 0;
}






