#this is a simple makefile might not be efficient but works well for this small project

all:
	g++ --std=c++11 clientA.cpp -o clientA
	g++ --std=c++11 clientB.cpp -o clientB
	g++ --std=c++11 central.cpp -o central
	g++ --std=c++11 serverT.cpp -o serverT
	g++ --std=c++11 serverS.cpp -o serverS
	g++ --std=c++11 serverP.cpp -o serverP

clientA:
	g++ --std=c++11 clientA.cpp -o clientA

clientB:
	g++ --std=c++11 clientB.cpp -o clientB

central:
	g++ --std=c++11 central.cpp -o central

serverT:
	g++ --std=c++11 serverT.cpp -o serverT

serverS:
	g++ --std=c++11 serverS.cpp -o serverS

serverP:
	g++ --std=c++11 serverP.cpp -o serverP

clean:
	rm -f clientA clientB central serverT serverS serverP

