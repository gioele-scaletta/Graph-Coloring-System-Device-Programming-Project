

#include "graph.h"
#include <iostream>
#include <iomanip>


int main() {
	
	clock_t start, end;
	double time_not_parallel;
	
	int color_parallel;
	double time, score;
	string graph_sample, graph_sample_path;
	double finalscoreJonesPlassmanColoringParallelQueueCounter=0, finalscoreJonesPlassmanColoringParallelQueueVcetor=0, finalscoreJonesPlassmanColoringParallelBarriers=0, finalscoreJonesPlassmanColoringParallelVector=0, finalscoreJonesPlassmanColoringParallelOnenodeThread=0, finalscoreLargestDegreeFirst = 0;
	double finaltimeJonesPlassmanColoringParallelQueueCounter = 0, finaltimeJonesPlassmanColoringParallelQueueVcetor = 0, finaltimeJonesPlassmanColoringParallelBarriers = 0, finaltimeJonesPlassmanColoringParallelVector = 0, finaltimeJonesPlassmanColoringParallelOnenodeThread = 0, finaltimeLargestDegreeFirst=0;

	int output_width = 50;

	map<string, string> GRAPHS = {{"citeseer_sub_10720.gra", "../../../../benchmark/small_dense_real/citeseer_sub_10720.gra"}, {"ba10k5d", "../../../../benchmark/scaleFree/ba10k5d.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../benchmark/manual/v100.gra"}, {"ba10k2d", "../../../../benchmark/scaleFree/ba10k2d.gra"},  {"agrocyc_dag_uniq.gra" , "../../../../benchmark/sigmod08/agrocyc_dag_uniq.gra"},
	{"anthra_dag_uniq.gra" , "../../../../benchmark/sigmod08/anthra_dag_uniq.gra"}, {"ecoo_dag_uniq.gra" , "../../../../benchmark/sigmod08/ecoo_dag_uniq.gra"}
	,{"citeseer.scc.gra", "../../../../benchmark/large/citeseer.scc.gra"}
	};
	
	/*map<string, string> GRAPHS = { {"uniprotenc_22m", "../../../../benchmark/large/uniprotenc_22m.scc.gra"},
	{"uniprotenc_100m", "../../../../benchmark/large/uniprotenc_100m.scc.gra"}};*/
	//map<string, string> GRAPHS = { {"uniprotenc_22m", "../../../../benchmark/large/uniprotenc_22m.scc.gra"} };

	// This does not work :( -> or maybe it works but it takes very very long
	// map<string, string> GRAPHS = { {"citeseerx", "../../../../benchmark/large/citeseerx.gra"} };

	//map<string, string> GRAPHS = { {"go_uniprot", "../../../../benchmark/large/go_uniprot.gra"} };

	map<string, string>::iterator it;
	for (it = GRAPHS.begin(); it != GRAPHS.end(); it++) {

		graph myGraph = graph();

		graph_sample = it->first;
		graph_sample_path = it->second;

		cout << "-------------------------------------------------" << endl;
		cout << "Loading " << graph_sample << endl;

		myGraph.readFileDIMACSCSR(graph_sample_path);

		cout << "Graph: " << graph_sample << endl;

		/*Greedy Sequential*/
		start = clock();
		myGraph.GreedySequential();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();
		
		cout << endl;
		cout << setw(output_width) << "Greedy Sequential Coloring: ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		/*Largest Degree First Standard*/
		/*start = clock();
		myGraph.LargestDegreeFirstStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Largest Degree First standard: ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Largest Degree First with one function to find and color nodes*/
		/*start = clock();
		myGraph.LargestDegreeFirstFindAndColor();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Largest Degree First (find and color): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Largest Degree First with overlaps*/
		/* TODO: the function does not work (deadlocks can happen, data structures are not properly locked) */
		/*start = clock();
		myGraph.LargestDegreeFirst();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Largest Degree First with overlaps: ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Smallest Degree Last sequential*/
		/*start = clock();
		myGraph.SmallestDegreeLastSequential();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Smallest Degree Last (sequential): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Smallest Degree Last*/
		/*start = clock();
		myGraph.SmallestDegreeLastStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Smallest Degree Last (sequential weighing): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		/*Smallest Degree Last parallel weighing*/
		/*start = clock();
		myGraph.SmallestDegreeLastParallelWeighing();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Smallest Degree Last (parallel weighing): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman sequential*/
		start = clock();
		myGraph.JonesPlassmanColoringSequential();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Jones-Plassman (sequential): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman standard with threadpool*/
		start = clock();
		myGraph.JonesPlassmanColoringParallelStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Jones-Plassman standard (with threadpool): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman standard without threadpool*/
		start = clock();
		myGraph.JonesPlassmanColoringParallelBarriers();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Jones-Plassman standard (without threadpool): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman with threadpool and find and color in the same function*/
		start = clock();
		myGraph.JonesPlassmanColoringParallelFindAndColor();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		cout << setw(output_width) << "Jones-Plassman standard (find and color): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		/*Jones Plassman with overlaps*/
		/*start = clock();
		myGraph.JonesPlassmanColoringParallelQueueCounter();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << setw(output_width) << "Jones-Plassman with overlaps: ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		/*Jones Plassman one node per thread -> COMMENTED OUT BECAUSE IT'S QUITE SLOW!*/
		/* 
		start = clock();
		myGraph.JonesPlassmanColoringParallelOneNodeThread();
		end = clock();

		color_parallel = myGraph.checkColoring();

		cout << setw(output_width) << "Jones-Plassman (one node per thread): ";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();
		*/
	}

	system("pause");
	return 0;
}






