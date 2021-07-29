

#include "graph.h"
#include <iostream>
#include <fstream>  
#include <iomanip>

void saveToFile(ofstream& myfile, string graph, string algo, double time, int colors);

int main() {
	
	clock_t start, end;
	double time_not_parallel;
	
	int color_parallel;
	double time, score;
	string graph_sample, graph_sample_path;
	double finalscoreJonesPlassmanColoringParallelQueueCounter=0, finalscoreJonesPlassmanColoringParallelQueueVcetor=0, finalscoreJonesPlassmanColoringParallelBarriers=0, finalscoreJonesPlassmanColoringParallelVector=0, finalscoreJonesPlassmanColoringParallelOnenodeThread=0, finalscoreLargestDegreeFirst = 0;
	double finaltimeJonesPlassmanColoringParallelQueueCounter = 0, finaltimeJonesPlassmanColoringParallelQueueVcetor = 0, finaltimeJonesPlassmanColoringParallelBarriers = 0, finaltimeJonesPlassmanColoringParallelVector = 0, finaltimeJonesPlassmanColoringParallelOnenodeThread = 0, finaltimeLargestDegreeFirst=0;

	int output_width = 50;

	/*map<string, string> GRAPHS = {{"citeseer_sub_10720.gra", "../../../../benchmark/small_dense_real/citeseer_sub_10720.gra"}, {"ba10k5d", "../../../../benchmark/scaleFree/ba10k5d.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../benchmark/manual/v100.gra"}, {"ba10k2d", "../../../../benchmark/scaleFree/ba10k2d.gra"},  {"agrocyc_dag_uniq.gra" , "../../../../benchmark/sigmod08/agrocyc_dag_uniq.gra"},
	{"anthra_dag_uniq.gra" , "../../../../benchmark/sigmod08/anthra_dag_uniq.gra"}, {"ecoo_dag_uniq.gra" , "../../../../benchmark/sigmod08/ecoo_dag_uniq.gra"}
	,{"citeseer.scc.gra", "../../../../benchmark/large/citeseer.scc.gra"}
	};*/
	
	/*map<string, string> GRAPHS = { {"uniprotenc_22m", "../../../../benchmark/large/uniprotenc_22m.scc.gra"},
	{"uniprotenc_100m", "../../../../benchmark/large/uniprotenc_100m.scc.gra"}};*/

	//PER PC GIO
	map<string, string> GRAPHS = { { "ba10k5d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k5d.gra"} , {"uniprotenc_100m", "../../../../benchmarks/benchmarks/benchmark/large/uniprotenc_100m.scc.gra"} , {"uniprotenc_22m", "../../../../benchmarks/benchmarks/benchmark/large/uniprotenc_22m.scc.gra"},
	{"citeseer_sub_10720.gra", "../../../../benchmarks/benchmarks/benchmark/small_dense_real/citeseer_sub_10720.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../benchmarks/benchmarks/benchmark/manual/v100.gra"}, {"ba10k2d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k2d.gra"},  {"agrocyc_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/agrocyc_dag_uniq.gra"},
	{"anthra_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/anthra_dag_uniq.gra"}, {"ecoo_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/ecoo_dag_uniq.gra"}
	, {"citeseer.scc.gra", "../../../../benchmarks/benchmarks/benchmark/large/citeseer.scc.gra"} }; // {"citeseerx", "../../../../benchmarks/benchmarks/benchmark/large/citeseerx.gra"} };


	// This does not work :( -> or maybe it works but it takes very very long
	//map<string, string> GRAPHS = { {"citeseerx", "../../../../benchmark/large/citeseerx.gra"} };

	//map<string, string> GRAPHS = { {"go_uniprot", "../../../../benchmark/large/go_uniprot.gra"} };
	//map<string, string> GRAPHS = { { "ba10k5d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k5d.gra"} };
	
	//map<string, string> GRAPHS = { {"citeseer.scc", "../../../../benchmark/large/citeseer.scc.gra"} };

	ofstream myfile;
	myfile.open("results.csv");
	

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
		string name = "Greedy Sequential Coloring";
		cout << setw(output_width) << name << " :" ;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample ,name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;


		myGraph.cancelColors();


		/*Largest Degree First Standard*/
		start = clock();
		myGraph.LargestDegreeFirstStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		name = "Largest Degree First standard";
		cout << setw(output_width) << name << " :" ;
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Largest Degree First with one function to find and color nodes*/
		start = clock();
		myGraph.LargestDegreeFirstFindAndColor();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();
		name= "Largest Degree First (find and color)";
		cout << setw(output_width) << name << " :";
		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
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
		start = clock();
		myGraph.SmallestDegreeLastSequential();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		name = "Smallest Degree Last (sequential)";
		cout << setw(output_width) << name << " :";

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Smallest Degree Last*/
		start = clock();
		myGraph.SmallestDegreeLastStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		
		name = "Smallest Degree Last (sequential weighing)";
		cout << setw(output_width) << name << " :" ;

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();


		/*Smallest Degree Last parallel weighing*/
		start = clock();
		myGraph.SmallestDegreeLastParallelWeighing();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		
		name ="Smallest Degree Last (parallel weighing)";
		cout << setw(output_width) << name << " :" ;

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman sequential*/

		start = clock();
		myGraph.JonesPlassmanColoringSequential();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		name = "Jones - Plassman(sequential)";
		cout << setw(output_width) << name << " :";

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman standard with threadpool*/

		start = clock();
		myGraph.JonesPlassmanColoringParallelStandard();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		name = "Jones - Plassman standard(with threadpool)";
		cout << setw(output_width) << name << " :";

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman standard without threadpool*/

		start = clock();
		myGraph.JonesPlassmanColoringParallelBarriers();
		end = clock();

		color_parallel = myGraph.checkColoringCSR();

		name = "Jones-Plassman standard (without threadpool)";
		cout << setw(output_width) << name << " :" ;

		if (color_parallel != -1) {
			time = double(end - start) / double(CLOCKS_PER_SEC);
			cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
			saveToFile(myfile, graph_sample, name, time, color_parallel);
		}
		else
			cout << "Coloring is wrong!" << endl;

		myGraph.cancelColors();

		/*Jones Plassman with threadpool and find and color in the same function*/
		
		//for (int maxThreads = 1; maxThreads <= 10; maxThreads += 1) {
			start = clock();
			myGraph.JonesPlassmanColoringParallelFindAndColor(8);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			
			name = "Jones-Plassman standard (find and color): ";
			cout << setw(output_width) << name << " :" ;

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				saveToFile(myfile, graph_sample, name, time, color_parallel);
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();
		//}


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
	myfile << "Run performed on DELL XPS" << endl;
	myfile.close();


	system("pause");
	return 0;
}

void saveToFile(ofstream& myfile, string name, string algo, double time, int colors) {

	myfile << name << "," << algo << "," << time << "," << colors << "\n";


}



