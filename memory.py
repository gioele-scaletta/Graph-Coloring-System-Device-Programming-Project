import csv
import matplotlib.pyplot as plt
import numpy as np

algorithms_names_ids = {
    "Largest Degree First standard":1,
    "Smallest Degree Last (sequential weighing)":2,
    "Jones - Plassman standard(with threadpool)":3
}

if __name__ == "__main__":

    graph_ids = []
    graph_nodes = []
    vm = []
    pm = []
    
    with open('memory.csv', 'r') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter = ',')
        for row in csv_reader:
            graph_nodes.append(int(row[1]))
            vm.append(int(row[2]))
            pm.append(int(row[3]))

    graph_ids_np = np.array(graph_ids)
    graph_nodes_np = np.array(graph_nodes)
    vm_np = np.array(vm)
    pm_np = np.array(pm)

    ##
    # Plot vm and pm versus nodes
    ##

    plt.figure()

    plt.plot(graph_nodes, vm, 'o')
    plt.plot(graph_nodes, pm, 'o')

    plt.xlabel("Number of nodes")
    plt.ylabel("Memory (B)")
    plt.legend(["Virtual Memory", "Physical Memory"])
    plt.xscale("log")
    plt.savefig("Images/memory")
    #plt.show()

