import csv
import matplotlib.pyplot as plt
import numpy as np
import math

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

def data_to_plot(n_nodes_np, alg_names_np, alg_name, to_plot):
    
    n_nodes = n_nodes_np[alg_names_np == alg_name]
    times = to_plot[alg_names_np == alg_name]
    log_nodes = np.linspace(int(min(np.ceil(np.log10(n_nodes)))), int(max(np.ceil(np.log10(n_nodes)))), 
        int(max(np.ceil(np.log10(n_nodes)))) - int(min(np.ceil(np.log10(n_nodes)))) + 1)
    times_nodes = np.zeros([len(log_nodes), ])
    count_nodes = np.zeros([len(log_nodes), ])

    for i in range(len(n_nodes)):
        int_n = math.ceil(np.log10(n_nodes[i]))
        times_nodes[np.where(log_nodes == int_n)] += times[i]
        count_nodes[np.where(log_nodes == int_n)] += 1
    
    times_nodes = times_nodes[count_nodes != 0]
    log_nodes = log_nodes[count_nodes != 0]
    count_nodes = count_nodes[count_nodes != 0]
    avg = times_nodes / count_nodes
    
    return log_nodes, avg


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
    """
            ##
            # Create table with results: TIME
            ##

            fw.write("\\begin{table}[h]\n")
            fw.write("\\begin{tabular}{l | c | ")

            for a_name in unique_alg_names:
                fw.write("c ")
            fw.write("}\n")

            fw.write(" & Nodes & ")
            for (i, a_name) in enumerate(unique_alg_names):
                if i != len(unique_alg_names) - 1:
                    fw.write(a_name + " & ")
                else:
                    fw.write(a_name)
            fw.write("\\\\\n")
            fw.write("\hline\n")

            for g_name in unique_graph_names:

                nodes = n_nodes_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                fw.write(g_name.replace("_", "\_").replace(".graph", "").replace(".gra", "") + " & " + str(nodes[0]) + " & ")

                for (i, a_name) in enumerate(unique_alg_names):
                    
                    time = times_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                    if i != len(unique_alg_names) - 1:
                        if min(times_np[graph_names_np == g_name]) == time[0]:
                            fw.write("\\textbf{" + str(time[0]) + "} & ")
                        else:
                            fw.write(str(time[0]) + " & ")
                    else:
                        if min(times_np[graph_names_np == g_name]) == time[0]:
                            fw.write("\\textbf{" + str(time[0]) + "} ")
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
            fw.write("\\begin{tabular}{l | c | ")
            
            for a_name in unique_alg_names:
                fw.write("c ")
            fw.write("}\n")

            fw.write(" & Nodes & ")
            for (i, a_name) in enumerate(unique_alg_names):
                if i != len(unique_alg_names) - 1:
                    fw.write(a_name + " & ")
                else:
                    fw.write(a_name)
            fw.write("\\\\\n")
            fw.write("\hline\n")

            for g_name in unique_graph_names:

                nodes = n_nodes_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                fw.write(g_name.replace("_", "\_").replace(".graph", "").replace(".gra", "") + " & " + str(nodes[0]) + " & ")

                for (i, a_name) in enumerate(unique_alg_names):
                    
                    color = colors_np[np.logical_and(graph_names_np == g_name, alg_names_np == a_name)]
                    if i != len(unique_alg_names) - 1:
                        if min(colors_np[graph_names_np == g_name]) == color[0]:
                            fw.write("\\textbf{" + str(color[0]) + "} & ")
                        else:
                            fw.write(str(color[0]) + " & ")                    
                    else:
                        if min(colors_np[graph_names_np == g_name]) == color[0]:
                            fw.write("\\textbf{" + str(color[0]) + "}")
                        else:
                            fw.write(str(color[0]))    

                fw.write("\\\\\n")

            fw.write("\end{tabular}\n")
            fw.write("\caption{Number of colors on different graphs using different algorithms}\n")
            fw.write("\end{table}\n")
    
    ###
    # Plot graph of time vs n_nodes
    ###
    
    log_nodes_Greedy, avg_times_Greedy = data_to_plot(n_nodes_np, alg_names_np, "Greedy0", times_np)
    log_nodes_JP, avg_times_JP = data_to_plot(n_nodes_np, alg_names_np, "JP2", times_np)
    log_nodes_LDF, avg_times_LDF = data_to_plot(n_nodes_np, alg_names_np, "LDF1", times_np)
    log_nodes_SDL, avg_times_SDL = data_to_plot(n_nodes_np, alg_names_np, "SDL1", times_np)

    plt.figure()
    plt.plot(10 ** log_nodes_Greedy, avg_times_Greedy, '-o')
    plt.plot(10 ** log_nodes_JP, avg_times_JP, '-o')
    plt.plot(10 ** log_nodes_LDF, avg_times_LDF, '-o')
    plt.plot(10 ** log_nodes_SDL, avg_times_SDL, '-o')
    plt.xscale("log")
    plt.ylabel("Time (sec)")
    plt.xlabel("Number of nodes")
    plt.legend(["Greedy", "JP", "LDF", "SDL"])
    plt.savefig("Images/times_nodes")
    #plt.show()


    
    ###
    # Plot graph of colors vs n_nodes
    ###

    # Remove colors >= 70 because they are outliers generated by some particular types of graph
    log_nodes_Greedy, avg_colors_Greedy = data_to_plot(n_nodes_np[colors_np < 70], alg_names_np[colors_np < 70], "Greedy0", colors_np[colors_np < 70])
    log_nodes_JP, avg_colors_JP = data_to_plot(n_nodes_np[colors_np < 70], alg_names_np[colors_np < 70], "JP2", colors_np[colors_np < 70])
    log_nodes_LDF, avg_colors_LDF = data_to_plot(n_nodes_np[colors_np < 70], alg_names_np[colors_np < 70], "LDF1", colors_np[colors_np < 70])
    log_nodes_SDL, avg_colors_SDL = data_to_plot(n_nodes_np[colors_np < 70], alg_names_np[colors_np < 70], "SDL1", colors_np[colors_np < 70])

    plt.figure()
    plt.plot(10 ** log_nodes_Greedy, avg_colors_Greedy, '-o')
    plt.plot(10 ** log_nodes_JP, avg_colors_JP, '-o')
    plt.plot(10 ** log_nodes_LDF, avg_colors_LDF, '-o')
    plt.plot(10 ** log_nodes_SDL, avg_colors_SDL, '-o')
    plt.xscale("log")
    plt.ylabel("Number of colors")
    plt.xlabel("Number of nodes")
    plt.legend(["Greedy", "JP", "LDF", "SDL"])
    plt.savefig("Images/colors_nodes")
    #plt.show()
    """
    ###
    # Plot graph of time vs n_nodes for parallel vs sequential
    ###
    
    log_nodes_JP2, avg_times_JP2 = data_to_plot(n_nodes_np, alg_names_np, "JP2", times_np)
    log_nodes_SDL1, avg_times_SDL1 = data_to_plot(n_nodes_np, alg_names_np, "SDL1", times_np)
    log_nodes_JP0, avg_times_JP0 = data_to_plot(n_nodes_np, alg_names_np, "JP0", times_np)
    log_nodes_SDL0, avg_times_SDL0 = data_to_plot(n_nodes_np, alg_names_np, "SDL0", times_np)

    plt.figure()
    plt.plot(10 ** log_nodes_JP2, avg_times_JP2, '-o', color='r')
    plt.plot(10 ** log_nodes_SDL1, avg_times_SDL1, '-o', color='b')
    plt.plot(10 ** log_nodes_JP0, avg_times_JP0, '--o', color='r')
    plt.plot(10 ** log_nodes_SDL0, avg_times_SDL0, '--o', color='b')
    plt.xscale("log")
    plt.ylabel("Time (sec)")
    plt.xlabel("Number of nodes")
    plt.legend(["JP - parallel", "SDL - parallel", "JP - sequential", "SDL - sequential"])
    plt.savefig("Images/times_nodes_par_seq")
    #plt.show()
    

    ###
    # Plot graph of time vs n_nodes for standard vs find and color
    ###
    
    log_nodes_JP2, avg_times_JP2 = data_to_plot(n_nodes_np, alg_names_np, "JP2", times_np)
    log_nodes_LDF1, avg_times_LDF1 = data_to_plot(n_nodes_np, alg_names_np, "LDF1", times_np)
    log_nodes_JP3, avg_times_JP3 = data_to_plot(n_nodes_np, alg_names_np, "JP3", times_np)
    log_nodes_LDF2, avg_times_LDF2 = data_to_plot(n_nodes_np, alg_names_np, "LDF2", times_np)

    plt.figure()
    plt.plot(10 ** log_nodes_JP2, avg_times_JP2, '-o', color='r')
    plt.plot(10 ** log_nodes_LDF1, avg_times_LDF1, '-o', color='b')
    plt.plot(10 ** log_nodes_JP3, avg_times_JP3, '--o', color='r')
    plt.plot(10 ** log_nodes_LDF2, avg_times_LDF2, '--o', color='b')
    plt.xscale("log")
    plt.ylabel("Time (sec)")
    plt.xlabel("Number of nodes")
    plt.legend(["JP - standard", "SDL - standard", "JP - find and color", "SDL - find and color"])
    plt.savefig("Images/times_nodes_std_findandcolor")
    #plt.show()