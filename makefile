#this is a simple makefile, might not be efficient but works well for this small project

all:   #to compile all the files together
	g++ --std=c++11 clientA.cpp -o clientA
	g++ --std=c++11 clientB.cpp -o clientB
	g++ --std=c++11 central.cpp -o serverC
	g++ --std=c++11 serverT.cpp -o serverT
	g++ --std=c++11 serverS.cpp -o serverS
	g++ --std=c++11 serverP.cpp -o serverP

clean:	#to clear all object files
	rm -f clientA clientB central serverT serverS serverP

