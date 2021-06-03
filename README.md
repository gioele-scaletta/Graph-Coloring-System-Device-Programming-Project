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