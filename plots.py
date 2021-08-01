import csv
import matplotlib.pyplot as plt

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

    with open('results.csv', 'r') as csv_file:
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


    a = graph_ids==1

    plt.figure()
    plt.plot(n_threads[graph_ids==1 and coefs == 10 and algorithm_ids == 1],
                times[graph_ids==1 and coefs == 10 and algorithm_ids == 1])
    plt.show()



