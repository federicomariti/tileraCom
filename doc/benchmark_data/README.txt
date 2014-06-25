--------------------------------------------------------------------------------
Contenuto della directory:
--------------------------------------------------------------------------------
* error_handler.h
* calc_statistics.c
* calc_statistics
* createPlotPage.sh
* create_all_plots.sh
* plot_scalability.gp
* plot.sh
* plot_multicastTime.gp
* plot_rowCalcTimeRatio.gp
* plot_T-log.gp
* plot_T.gp
* plot_Tc-log.gp
* plot_Tc.gp
* script.sh
* file di dati: terminano per '.dat' o per '.dbg'
* file di statistiche: terminano per '.avg'
* relazione.tex

--------------------------------------------------------------------------------
Descrizione dei files
--------------------------------------------------------------------------------
calc_statistics.c

  Usato per tutti i tipi di dati che hanno due colonne rispettivamente
  per il grado di parallelismo e per il valore misurato, produce un
  file .avg che contiene per ogni grado di parallelismo il valore
  medio, massimo, minimo, sdev del valore considerato, inoltre viene
  fornito il raporto tra i valori medi del grado di parallelismo 1 e
  quello corrente, e il reciproco di tale rapporto.

  Row Entry: (1) n (2) avg (3) max (4) min (5) sdev (6) s (7) 1/s

  Tale programma viene usato per tutti i tipi di file .dat o .dbg
  prodotti dal benchmark.

  In testa al file .avg prodotto viene scritta la riga contenente un
  numero arbitrario di campi, i quali sono stringhe che descrivono in
  modo diverso il contenuto del file (concentrandosi su uno o pi\`u
  singoli asapetti: ad esempio la dimensione dei dati,
  l'implementazione, etc.) e possono essere utili per fornire il
  titolo di un grafico.

plot_scalability.gp

  Disegna il plot con pi\`u grafici di scalabilit\`a, come primo
  parametro prende in ingresso dei file di tipo .avg e usa le colonne
  1 e 6, come secondo parametro ha in ingresso l'identificatore di
  colonna della prima riga del file .avg che fornisce il titolo del
  grafico.

plot_Tc-log.gp
plot_Tc.gp

plot_rowCalcTimeRatio.gp

  Disegna il grafico del rapporto tra il tempo di calcolo di una
  singola riga con grado di parallelismo n e 1.

plot_T-log.gp
plot_T.gp

plot.sh

  Esegue una singola invocazione di un file .gp, prende in ingresso
  tutto il necessario per stabilire come fare il plot e come scrivere
  il risultato.

  GNUPLOT produce in uscita due file .esp e .tex, l'.esp viene
  convertito in pdf

  Puo` essere creato un file _main.tex e il corrispondente _main.pdf
  per un documento LaTex contenente solo il grafico.

script.sh

  Invoca pi\`u volte plot.sh producendo quindi diversi plot. 

  Prende in ingresso un parametro stringa per decidere quali grafici
  creare.

relazione.tex

  Contiene la relazione sul benchmark e sui risultati ottenuti,
  contiene i grafici prodotti pi\`u significativi.

--------------------------------------------------------------------------------
Cosa fare
--------------------------------------------------------------------------------
Il file create_all_plots.sh contiene le invocazioni a script.sh per
creare tutti i grafici. 
Eseguire ./create_all_plots.sh e ricompilare il file relazione.tex.

--------------------------------------------------------------------------------
Utility
--------------------------------------------------------------------------------
* ls -l nogatherInt_00_500_*_56.dat | awk '{print $10}' | cut -d "_" -f 4 | sort -n

  stampa tutti i tempi di interarrivo usati per il benchmark con
  dimensione dei dati 56, implementazione UDN e lunghezza dello stream
  500.