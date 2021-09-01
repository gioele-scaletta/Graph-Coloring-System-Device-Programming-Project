import csv
import matplotlib.pyplot as plt
import numpy as np

graph_names_ids = {
    "rgg_n_2_15_s0.graph" : 1,
    "rgg_n_2_16_s0.graph" : 2,
    "rgg_n_2_17_s0.graph" : 3,
    "rgg_n_2_18_s0.graph" : 4,
    "rgg_n_2_19_s0.graph" : 5,
    "rgg_n_2_20_s0.graph" : 6
}

algorithms_names_ids = {
    "Largest Degree First standard":1,
    "Smallest Degree Last (sequential weighing)":2,
    "Jones - Plassman standard(with threadpool)":3
}

if __name__ == "__main__":

    graph_ids = []
    graph_nodes = []
    algorithm_ids = []
    n_threads = []
    coefs = []
    times = []
    colors = []

    with open('../results/results.csv', 'r') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter = ',')
        for row in csv_reader:
            if len(row) == 7:
                graph_ids.append(graph_names_ids[row[0]])
                graph_nodes.append(int(row[1]))
                algorithm_ids.append(algorithms_names_ids[row[2]])
                n_threads.append(int(row[3]))
                coefs.append(int(row[4]))
                times.append(float(row[5]))
                colors.append(int(row[6]))


    graph_ids_np = np.array(graph_ids)
    graph_nodes_np = np.array(graph_nodes)
    algorithm_ids_np = np.array(algorithm_ids)
    n_threads_np = np.array(n_threads)
    coefs_np = np.array(coefs)
    times_np = np.array(times)
    colors_np = np.array(colors)


    ##
    # Plot average of times VS n_threads for different algorithms (average over different graphs) 
    # using the best value for coef
    ##

    plt.figure()

    n_threads_val = n_threads_np[np.logical_and( np.logical_and((graph_ids_np==1), (coefs_np == 10)) , (algorithm_ids_np == 1))]

    avg_time_alg = np.zeros([len(algorithms_names_ids), n_threads_val.shape[0]])
    
    for alg_name, i in algorithms_names_ids.items():

        avg_time_alg[i-1, :] = times_np[np.logical_and( np.logical_and((graph_ids_np==1), (coefs_np == 10)) , (algorithm_ids_np == i))]

        for graph_name, j in graph_names_ids.items():
            if j != 1:
                avg_time_alg[i-1, :] += times_np[np.logical_and( np.logical_and((graph_ids_np==j), (coefs_np == 10)) , (algorithm_ids_np == i))]

        avg_time_alg[i-1, :] = avg_time_alg[i-1, :] / len(graph_names_ids)
        plt.plot(n_threads_val, avg_time_alg[i-1, :], '-o')

    plt.xlabel("Number of threads")
    plt.ylabel("Time")
    plt.legend(algorithms_names_ids.keys())
    plt.savefig("../images/times_threads")
    #plt.show()


    ##
    # Plot average of colors VS n_threads for different algorithms (average over different graphs)
    # using the best value for coef
    ##

    plt.figure()

    n_threads_val = n_threads_np[np.logical_and( np.logical_and((graph_ids_np==1), (coefs_np == 10)) , (algorithm_ids_np == 1))]

    avg_color_alg = np.zeros([len(algorithms_names_ids), n_threads_val.shape[0]])
    
    for alg_name, i in algorithms_names_ids.items():

        avg_color_alg[i-1, :] = colors_np[np.logical_and( np.logical_and((graph_ids_np==1), (coefs_np == 10)) , (algorithm_ids_np == i))]

        for graph_name, j in graph_names_ids.items():
            if j != 1:
                avg_color_alg[i-1, :] += colors_np[np.logical_and( np.logical_and((graph_ids_np==j), (coefs_np == 10)) , (algorithm_ids_np == i))]

        avg_color_alg[i-1, :] = avg_color_alg[i-1, :] / len(graph_names_ids)
        plt.plot(n_threads_val, avg_color_alg[i-1, :], '-o')

    plt.xlabel("Number of threads")
    plt.ylabel("Number of colors")
    plt.legend(algorithms_names_ids.keys())
    plt.savefig("../images/colors_threads")
    #plt.show()


    ##
    # Plot average of times VS coefficients for different algorithms (average over different graphs)
    # using the best value for n_threads
    ##

    plt.figure()

    n_coef_val = coefs_np[np.logical_and( np.logical_and((graph_ids_np==1), (n_threads_np == 8)) , (algorithm_ids_np == 1))]

    avg_time_alg = np.zeros([len(algorithms_names_ids), n_coef_val.shape[0]])
    
    for alg_name, i in algorithms_names_ids.items():

        avg_time_alg[i-1, :] = times_np[np.logical_and( np.logical_and((graph_ids_np==1), (n_threads_np == 8)) , (algorithm_ids_np == i))]

        for graph_name, j in graph_names_ids.items():
            if j != 1:
                avg_time_alg[i-1, :] += times_np[np.logical_and( np.logical_and((graph_ids_np==j), (n_threads_np == 8)) , (algorithm_ids_np == i))]

        avg_time_alg[i-1, :] = avg_time_alg[i-1, :] / len(graph_names_ids)
        plt.plot(n_coef_val, avg_time_alg[i-1, :], '-o')

    plt.xlabel("Values of coefficient")
    plt.ylabel("Time")
    plt.xscale("log")
    plt.legend(algorithms_names_ids.keys())
    plt.savefig("../images/times_coefs")
    #plt.show()



    ##
    # Plot average of colors VS coefficients for different algorithms (average over different graphs)
    # using the best value for n_threads
    ##

    plt.figure()

    n_coef_val = coefs_np[np.logical_and( np.logical_and((graph_ids_np==1), (n_threads_np == 8)) , (algorithm_ids_np == 1))]

    avg_color_alg = np.zeros([len(algorithms_names_ids), n_coef_val.shape[0]])
    
    for alg_name, i in algorithms_names_ids.items():

        avg_color_alg[i-1, :] = colors_np[np.logical_and( np.logical_and((graph_ids_np==1), (n_threads_np == 8)) , (algorithm_ids_np == i))]

        for graph_name, j in graph_names_ids.items():
            if j != 1:
                avg_color_alg[i-1, :] += colors_np[np.logical_and( np.logical_and((graph_ids_np==j), (n_threads_np == 8)) , (algorithm_ids_np == i))]

        avg_color_alg[i-1, :] = avg_color_alg[i-1, :] / len(graph_names_ids)
        plt.plot(n_coef_val, avg_color_alg[i-1, :], '-o')

    plt.xlabel("Values of coefficient")
    plt.ylabel("Number of colors")
    plt.xscale("log")
    plt.legend(algorithms_names_ids.keys())
    plt.savefig("../images/colors_coefs")
    #plt.show()