
#include "graph.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <experimental/filesystem>
#include <map>
#include "windows.h"
#include "psapi.h"
namespace fs = std::experimental::filesystem;

/* Used for the different paths and for the CPU specifications */
#define ANDRE
//#define GIO

using namespace std;

int main() {

	clock_t start, end;
	string name;
	int n_colors;
	double time, curr_time;
	double min = 9999999;
	int mini =0, minj=0;
	string graph_sample, graph_sample_path;
	vector<int> N_threads = { 2, 3, 5, 8, 12, 20}; 
	vector<int> coefs = { 1, 5, 10, 100 };
	int coef, n_threads;
	int output_width = 50, i, j;
#if defined(GIO)
	string path1 = "../../../../benchmarks/benchmarks/rgg", path2= "../../../../benchmarks/benchmarks";
#elif defined(ANDRE)
	string path1= "../../../../benchmark/parameter_tuning", path2= "../../../../benchmark";
#endif
	ofstream results_file, memory_file;
	
	/*
	 * HYPERPARAMETER OPTIMIZATION:
	 * For each n_thread and coef value, save sum of times to find combination of parameters with lowest value of time
	 */
	vector<vector<double>> best_res_mat;

	// Initialize matrix of results
	for (i = 0; i < N_threads.size(); i++) {
		best_res_mat.push_back(vector<double>());
		for (j = 0; j < coefs.size(); j++)
			best_res_mat[i].push_back(0.0);
	}

	results_file.open("../../../results/results.csv", std::ofstream::trunc);

	/* Load all graphs in the hyperparameter optimization set and run algorithms with different hyperparameters */
	for (auto& p : fs::recursive_directory_iterator(path1)) {

		if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0 || p.path().string().compare(p.path().string().size() - 6, 6, ".graph") == 0) {
			graph myGraph = graph();

			// Name and path of the graph
			graph_sample = p.path().filename().string();
			graph_sample_path = p.path().string();

			cout << "-------------------------------------------------" << endl;
			cout << "Loading " << graph_sample << endl;

			// Two different reading functions depending on the graph format (.gra -> DIMACS10, .graph -> DIMACS)
			if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0)
				myGraph.readFileDIMACS10(graph_sample_path);
			else
				myGraph.readFileDIMACS(graph_sample_path);

			cout << "Graph: " << graph_sample << endl;

			// Test different values of the hyperparameters
			for (i = 0; i < N_threads.size(); i++) {
				n_threads = N_threads[i];

				for (j = 0; j < coefs.size(); j++) {
					coef = coefs[j];

					curr_time = best_res_mat[i][j];

					/*Largest Degree First Standard*/
					start = clock();
					myGraph.LargestDegreeFirstStandard(n_threads, coef);
					end = clock();

					n_colors = myGraph.checkColoring();

					name = "Largest Degree First standard";
					cout << setw(output_width) << name << " :";
					if (n_colors != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
						results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << n_colors << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					myGraph.cancelColors();


					/*Smallest Degree Last*/
					start = clock();
					myGraph.SmallestDegreeLastStandard(n_threads, coef);
					end = clock();

					n_colors = myGraph.checkColoring();

					name = "Smallest Degree Last (sequential weighing)";
					cout << setw(output_width) << name << " :";

					if (n_colors != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
						results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << n_colors << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					myGraph.cancelColors();


					/*Jones Plassman standard with threadpool*/

					start = clock();
					myGraph.JonesPlassmanColoringParallelStandard(n_threads, coef);
					end = clock();

					n_colors = myGraph.checkColoring();

					name = "Jones - Plassman standard(with threadpool)";
					cout << setw(output_width) << name << " :";

					if (n_colors != -1) {
						time = double(end - start) / double(CLOCKS_PER_SEC);
						curr_time += time;
						cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
						results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << coef << "," << time << "," << n_colors << "\n";
					}
					else
						cout << "Coloring is wrong!" << endl;

					myGraph.cancelColors();

					// Store the sum of the times in the matrix
					best_res_mat[i][j] = curr_time;
				}
			}
		}
	}

	// Find best hyperparameters
	for (i = 0; i < N_threads.size(); i++){
		for (j = 0; j < coefs.size(); j++){
			if (min > best_res_mat[i][j]) {
				min = best_res_mat[i][j];
				mini = i;
				minj = j;
			}
		}
	}
	n_threads = N_threads[mini];
	coef = coefs[minj];
	
	// Check if selection worked properly
	if (n_threads > 20 || coef > 100) {
		cout << "not working" << endl;
		return 0;
	}

	results_file << "best results: n_thread= " << n_threads << " coef= " << coef << endl;
	cout << "best results: n_thread= " << n_threads << " coef= " << coef << endl;

#if defined(GIO)
	results_file << "Run performed on DELL XPS" << endl;
#elif defined(ANDRE)
	results_file << "Run performed on Intel i7-8550U" << endl;
#endif
	results_file.close();

	/*
     * Run different algorithms on many graphs with best n_threads and coef
	 */

	results_file.open("../../../results/results_best.csv", std::ofstream::trunc);
	memory_file.open("../../../results/memory.csv", std::ofstream::trunc);

	// Load all graphs in the test set and run algorithms on them
	for (auto& p : fs::recursive_directory_iterator(path2)) {

		if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0 || p.path().string().compare(p.path().string().size() - 6, 6, ".graph") == 0) {
			graph myGraph = graph();

			// Graph name and path
			graph_sample = p.path().filename().string();
			graph_sample_path = p.path().string();

			cout << "-------------------------------------------------" << endl;
			cout << "Loading " << graph_sample << endl;

			// Two different reading functions depending on the graph format (.gra -> DIMACS10, .graph -> DIMACS)
			if (p.path().string().compare(p.path().string().size() - 4, 4, ".gra") == 0)
				myGraph.readFileDIMACS10(graph_sample_path);
			else
				myGraph.readFileDIMACS(graph_sample_path);

			// Check memory usage when the graph is loaded
			PROCESS_MEMORY_COUNTERS_EX pmc;
			K32GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
			SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
			SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
			
			cout << "Virtual memory used: " << virtualMemUsedByMe << endl;
			cout << "Physical memory used: " << physMemUsedByMe << endl;
			memory_file << graph_sample << "," << myGraph.getNNodes() << "," << virtualMemUsedByMe << "," << physMemUsedByMe << endl;

			cout << "Graph: " << graph_sample << endl;

			/*Greedy Sequential*/
			start = clock();
			myGraph.GreedySequential();
			end = clock();

			n_colors = myGraph.checkColoring();

			cout << endl;
			name = "Greedy Sequential Coloring";
			cout << setw(output_width) << name << " :";
			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;


			myGraph.cancelColors();

			/*Largest Degree First Standard*/
			start = clock();
			myGraph.LargestDegreeFirstStandard(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Largest Degree First standard";
			cout << setw(output_width) << name << " :";
			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Largest Degree First with one function to find and color nodes*/
			start = clock();
			myGraph.LargestDegreeFirstFindAndColor(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();
			name = "Largest Degree First (find and color)";
			cout << setw(output_width) << name << " :";
			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			start = clock();
			myGraph.SmallestDegreeLastSequential();
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Smallest Degree Last (sequential)";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Smallest Degree Last*/
			start = clock();
			myGraph.SmallestDegreeLastStandard(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Smallest Degree Last (sequential weighing)";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman sequential*/

			start = clock();
			myGraph.JonesPlassmanColoringSequential();
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Jones - Plassman(sequential)";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << "1" << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman standard with threadpool*/

			start = clock();
			myGraph.JonesPlassmanColoringParallelStandard(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Jones - Plassman standard(with threadpool)";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman standard without threadpool*/

			start = clock();
			myGraph.JonesPlassmanColoringParallelNoThreadpool(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Jones-Plassman standard (without threadpool)";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();

			/*Jones Plassman with threadpool and find and color in the same function*/
			start = clock();
			myGraph.JonesPlassmanColoringParallelFindAndColor(n_threads, coef);
			end = clock();

			n_colors = myGraph.checkColoring();

			name = "Jones-Plassman standard (find and color): ";
			cout << setw(output_width) << name << " :";

			if (n_colors != -1) {
				time = double(end - start) / double(CLOCKS_PER_SEC);
				cout << "Time (sec): " << time << " \tColors: " << n_colors << endl;
				results_file << graph_sample << "," << myGraph.getNNodes() << "," << name << "," << n_threads << "," << time << "," << n_colors << "\n";
			}
			else
				cout << "Coloring is wrong!" << endl;

			myGraph.cancelColors();
		}
	}


#if defined(GIO)
	results_file << "Run performed on DELL XPS" << endl;
#elif defined(ANDRE)
	results_file << "Run performed on Intel i7-8550U" << endl;
#endif

	results_file.close();
	memory_file.close();

	system("pause");
	return 0;
}
