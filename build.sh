rm -f vnproch551
rm -f *.o
g++ -c main.cpp -o main.o
g++ -c KT_BinIO.cpp -o KT_BinIO.o
g++ -c KT_ProgressBar.cpp -o KT_ProgressBar.o
g++ main.o KT_BinIO.o KT_ProgressBar.o -lusb-1.0 -o vnproch551
