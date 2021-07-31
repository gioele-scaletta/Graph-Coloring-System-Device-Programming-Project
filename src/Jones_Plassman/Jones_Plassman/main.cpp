

#include "graph.h"
#include <iostream>
#include <fstream>  
#include <iomanip>
#include <experimental/filesystem> 
namespace fs = std::experimental::filesystem;

int main() {
	
	clock_t start, end;
	double time_not_parallel;
	string name;
	int color_parallel;
	double time, score;
	double min_time=0, curr_time;
	int best_coef, best_n_thread;
	string graph_sample, graph_sample_path;

	int output_width = 50;

	ofstream myfile;
	myfile.open("../../../results.csv", std::ofstream::trunc);
		
	for (auto& p : fs::recursive_directory_iterator("../../../../benchmarks/benchmarks/rgg")) {

		if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0 || p.path().string().compare(p.path().string().size() - 6, 6, ".graph") == 0) {
			graph myGraph = graph();

			graph_sample = p.path().filename().string();
			graph_sample_path = p.path().string();

			cout << "-------------------------------------------------" << endl;
			cout << "Loading " << graph_sample << endl;

			if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0)
				myGraph.readFileDIMACS10(graph_sample_path);
			else
				myGraph.readFileDIMACS(graph_sample_path);


			cout << "Graph: " << graph_sample << endl;

			for (int n_threads : {2, 3, 5, 8, 12, 20}) {
				for (int coef : { 1, 5, 10, 100 }) {

					curr_time = 0;
					/*Largest Degree First Standard*/
					start = clock();
					myGraph.LargestDegreeFirstStandard(n_threads, coef);
					end = clock();

					color_parallel = myGraph.checkColoringCSR();

					name = "Largest Degree First standard";
					cout << setw(output_width) << name << " :";
					if (color_parallel != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
						myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << color_parallel << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					myGraph.cancelColors();

					
					/*Smallest Degree Last*/
					start = clock();
					myGraph.SmallestDegreeLastStandard(n_threads, coef);
					end = clock();

					color_parallel = myGraph.checkColoringCSR();


					name = "Smallest Degree Last (sequential weighing)";
					cout << setw(output_width) << name << " :";

					if (color_parallel != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
						myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << color_parallel << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					myGraph.cancelColors();


					/*Jones Plassman standard with threadpool*/

					start = clock();
					myGraph.JonesPlassmanColoringParallelStandard(n_threads, coef);
					end = clock();

					color_parallel = myGraph.checkColoringCSR();

					name = "Jones - Plassman standard(with threadpool)";
					cout << setw(output_width) << name << " :";

					if (color_parallel != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
						myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << color_parallel << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					if (curr_time < min_time) {
						min_time = curr_time;
						best_coef = coef;
						best_n_thread = n_threads;
					}

					myGraph.cancelColors();
				}
			}
		}
		
	}

	myfile << "Run performed on DELL XPS" << endl;
	myfile.close();
	
	myfile.open("../../../results_best.csv", std::ofstream::trunc);


	/*
	 * Run with best n_threads and coef
	 */
	int n_threads = best_n_thread;
	int coef = best_coef;

	for (auto& p : fs::recursive_directory_iterator("../../../../benchmarks/benchmarks")) {

		if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0 || p.path().string().compare(p.path().string().size() - 6, 6, ".graph") == 0) {
			graph myGraph = graph();

			graph_sample = p.path().filename().string();
			graph_sample_path = p.path().string();

			cout << "-------------------------------------------------" << endl;
			cout << "Loading " << graph_sample << endl;

			if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0)
				myGraph.readFileDIMACS10(graph_sample_path);
			else
				myGraph.readFileDIMACS(graph_sample_path);


			cout << "Graph: " << graph_sample << endl;

			/*Greedy Sequential*/
			start = clock();
			myGraph.GreedySequential();
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			cout << endl;
			name = "Greedy Sequential Coloring";
			cout << setw(output_width) << name << " :";
			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;


			myGraph.cancelColors();

			/*Largest Degree First Standard*/
			start = clock();
			myGraph.LargestDegreeFirstStandard(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			name = "Largest Degree First standard";
			cout << setw(output_width) << name << " :";
			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Largest Degree First with one function to find and color nodes*/
			start = clock();
			myGraph.LargestDegreeFirstFindAndColor(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();
			name = "Largest Degree First (find and color)";
			cout << setw(output_width) << name << " :";
			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			start = clock();
			myGraph.SmallestDegreeLastSequential();
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			name = "Smallest Degree Last (sequential)";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Smallest Degree Last*/
			start = clock();
			myGraph.SmallestDegreeLastStandard(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();


			name = "Smallest Degree Last (sequential weighing)";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
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
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

				myGraph.cancelColors();
	
			/*Jones Plassman standard with threadpool*/

			start = clock();
			myGraph.JonesPlassmanColoringParallelStandard(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			name = "Jones - Plassman standard(with threadpool)";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman standard without threadpool*/

			start = clock();
			myGraph.JonesPlassmanColoringParallelBarriers(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();

			name = "Jones-Plassman standard (without threadpool)";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman with threadpool and find and color in the same function*/
			start = clock();
			myGraph.JonesPlassmanColoringParallelFindAndColor(n_threads, coef);
			end = clock();

			color_parallel = myGraph.checkColoringCSR();


			name = "Jones-Plassman standard (find and color): ";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				myfile << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << color_parallel << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();
		}
	}


	myfile << "Run performed on DELL XPS" << endl;
	myfile.close();

	system("pause");
	return 0;
}

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


/*Smallest Degree Last parallel weighing*/
			/*start = clock();
			myGraph.SmallestDegreeLastParallelWeighing();
			end = clock();

			color_parallel = myGraph.checkColoringCSR();


			name = "Smallest Degree Last (parallel weighing)";
			cout << setw(output_width) << name << " :";

			if (color_parallel != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << color_parallel << endl;
				saveToFile(myfile, graph_sample, name, time, color_parallel);
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();


/*map<string, string> GRAPHS = {{"citeseer_sub_10720.gra", "../../../../benchmark/small_dense_real/citeseer_sub_10720.gra"}, {"ba10k5d", "../../../../benchmark/scaleFree/ba10k5d.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmark/sigmod08/mtbrv_dag_uniq.gra"},
	{"v100.gra", "../../../../benchmark/manual/v100.gra"}, {"ba10k2d", "../../../../benchmark/scaleFree/ba10k2d.gra"},  {"agrocyc_dag_uniq.gra" , "../../../../benchmark/sigmod08/agrocyc_dag_uniq.gra"},
	{"anthra_dag_uniq.gra" , "../../../../benchmark/sigmod08/anthra_dag_uniq.gra"}, {"ecoo_dag_uniq.gra" , "../../../../benchmark/sigmod08/ecoo_dag_uniq.gra"}
	,{"citeseer.scc.gra", "../../../../benchmark/large/citeseer.scc.gra"}
	};*/

	/*map<string, string> GRAPHS = { {"uniprotenc_22m", "../../../../benchmark/large/uniprotenc_22m.scc.gra"},
	{"uniprotenc_100m", "../../../../benchmark/large/uniprotenc_100m.scc.gra"}};

	//PER PC GIO
map<string, string> GRAPHS = { /*{ "ba10k5d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k5d.gra"} , {"uniprotenc_100m", "../../../../benchmarks/benchmarks/benchmark/large/uniprotenc_100m.scc.gra"} , {"uniprotenc_22m", "../../../../benchmarks/benchmarks/benchmark/large/uniprotenc_22m.scc.gra"},
{"citeseer_sub_10720.gra", "../../../../benchmarks/benchmarks/benchmark/small_dense_real/citeseer_sub_10720.gra"},  {"mtbrv_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/mtbrv_dag_uniq.gra"},
{"v100.gra", "../../../../benchmarks/benchmarks/benchmark/manual/v100.gra"}, {"ba10k2d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k2d.gra"},  {"agrocyc_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/agrocyc_dag_uniq.gra"},
{"anthra_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/anthra_dag_uniq.gra"}, {"ecoo_dag_uniq.gra" , "../../../../benchmarks/benchmarks/benchmark/sigmod08/ecoo_dag_uniq.gra"}
, {"citeseer.scc.gra", "../../../../benchmarks/benchmarks/benchmark/large/citeseer.scc.gra"} };
{"citeseerx", "../../../../benchmarks/benchmarks/benchmark/large/citeseerx.gra"} */
	  

// This does not work :( -> or maybe it works but it takes very very long
//map<string, string> GRAPHS = { {"citeseerx", "../../../../benchmark/large/citeseerx.gra"} };

//map<string, string> GRAPHS = { {"go_uniprot", "../../../../benchmark/large/go_uniprot.gra"} };
//map<string, string> GRAPHS = { { "ba10k5d", "../../../../benchmarks/benchmarks/benchmark/scaleFree/ba10k5d.gra"} };

//map<string, string> GRAPHS = { {"citeseer.scc", "../../../../benchmark/large/citeseer.scc.gra"} };