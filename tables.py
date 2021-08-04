import csv
import matplotlib.pyplot as plt
import numpy as np

alg_names_brief = {
    "Jones-Plassman standard (without threadpool)" : "JP1",
    "Largest Degree First (find and color)" : "LDF2",
    "Jones - Plassman standard(with threadpool)" : "JP2",
    "Jones-Plassman standard (find and color): " : "JP3",
    "Smallest Degree Last (sequential)" : "SDL0",
    "Smallest Degree Last (sequential weighing)" : "SDL1",
    "Jones - Plassman(sequential)" : "JP0",
    "Largest Degree First standard" : "LDF1",
    "Greedy Sequential Coloring" : "Greedy0"
}


if __name__ == "__main__":

    best_n_threads = 8
    best_coef = 10

    with open("table_script.txt", "w") as fw:
        with open("results_best.csv", "r") as fr:

            graph_names = []
            n_nodes = []
            alg_names = []
            times = []
            colors = []

            ## Read csv file with results
            csv_reader = csv.reader(fr, delimiter = ',')
            for row in csv_reader:
                if len(row) == 6:
                    graph_names.append(row[0])
                    n_nodes.append(int(row[1]))
                    alg_names.append(alg_names_brief[row[2]])
                    times.append(float(row[4]))
                    colors.append(int(row[5]))

            unique_graph_names = list(set(graph_names))
            unique_alg_names = list(set(alg_names))

            graph_names_np = np.array(graph_names)
            n_nodes_np = np.array(n_nodes)
            alg_names_np = np.array(alg_names)
            times_np = np.array(times)
            colors_np = np.array(colors)

            ##
            # Create table with results: TIME
            ##

            fw.write("\\begin{table}[h]\n")
            fw.write("\\begin{tabular}{l | ")
            
            for a_name in unique_alg_names:
                fw.write("c c ")
            fw.write("}\n")

            fw.write(" & Nodes & ")
            for (i, a_name) in enumerate(unique_alg_names):
                if i != len(unique_alg_names):
                    fw.write(a_name + " & ")
                else:
                    fw.write(a_name)
            fw.write("\\\\\n")
            fw.write("\hline\n")

            for g_name in unique_graph_names:

                nodes = n_nodes_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                fw.write(g_name.replace("_", "\_") + " & " + str(nodes[0]) + " & ")

                for (i, a_name) in enumerate(unique_alg_names):
                    
                    time = times_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                    if i != len(unique_alg_names) - 1:
                        fw.write(str(time[0]) + " & ")  
                    else:
                        fw.write(str(time[0]))  

                fw.write("\\\\\n")

            fw.write("\end{tabular}\n")
            fw.write("\caption{Times (sec) to color different graphs using different algorithms}\n")
            fw.write("\end{table}\n")

            fw.write("\n\n\n\n\n")

            ##
            # Create table with results: COLORS
            ##

            fw.write("\\begin{table}[h]\n")
            fw.write("\\begin{tabular}{l | ")
            
            for a_name in unique_alg_names:
                fw.write("c c ")
            fw.write("}\n")

            fw.write(" & Nodes & ")
            for (i, a_name) in enumerate(unique_alg_names):
                if i != len(unique_alg_names):
                    fw.write(a_name + " & ")
                else:
                    fw.write(a_name)
            fw.write("\\\\\n")
            fw.write("\hline\n")

            for g_name in unique_graph_names:

                nodes = n_nodes_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                fw.write(g_name.replace("_", "\_") + " & " + str(nodes[0]) + " & ")

                for (i, a_name) in enumerate(unique_alg_names):
                    
                    color = colors_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                    if i != len(unique_alg_names) - 1:
                        fw.write(str(color[0]) + " & ")  
                    else:
                        fw.write(str(color[0]))  

                fw.write("\\\\\n")

            fw.write("\end{tabular}\n")
            fw.write("\caption{Number of colors on different graphs using different algorithms}\n")
            fw.write("\end{table}\n")



