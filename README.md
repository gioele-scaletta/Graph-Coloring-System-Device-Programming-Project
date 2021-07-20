# Graph-Coloring-SDP-Project





### Questions for Quer

* Graph structure: can we use containers? Maps or vectors?
* Quali algoritmi implementare? Qual è l'obiettivo finale?
  * algoritmi del paper allwright (Jones Plassman, Smallest-Degree-Last, Largest-Degree-First)
  * HEAD (genetic algorithm) e monte carlo tree search algorithm (?)
* C'è libertà nel come strutturare i thread? 
  * ogni thread lavora su un nodo solo :(
  * 8 thread, ognuno lavora su un ottavo del grafo a ogni iterazione :)
  * thread pool, ogni job è una porzione di grafo (non si rispetta necessariamente l'ordine delle iterazioni) :D
* Obiettivo: velocità, avere una buona soluzione (pochi colori) o un tradeoff (privilegiando cosa)?
* Report: risultati per TUTTI i grafi caricati? 
* Perché .scc non funziona?

### Source for CSR representation of graphs

https://www.cs.cmu.edu/afs/cs/project/pscico-guyb/realworld/www/slidesS18/compression6.pdf

### Notes

* quando cerchiamo il minimo colore disponibile tra i nodi adiancenti, non stiamo in realtà trovando il minimo disponibile, ma il minimo colore che un vicino ha (ad esempio, se due vicini hanno colori 2 e 3, noi assegniamo come minimo colore disponibile 4, ma ci sarebbero anche 0 e 1), non so se vada bene 
* per usare le funzioni come `checkAndColorListOfNodesQueueCounter`, bisognerebbe separare l'identificazione dei nodi da colorare e la fase di colorazione, altrimenti si rischia che ci siano deadlock (per capire se un nodo va colorato o meno, devo controllare il colore di un nodo vicino, ma se un altro thread sta colorando questo nodo vicino, rischio un deadlock)

