/*
@author:	Diogo A. B. Fernandes
@contact:	diogoabfernandes@gmail.com
@license:	see LICENSE
*/

#include <iostream>
#include <cstdlib>

#include <cmath>
#include <limits>
#include <climits>

#include "ACO.h"
#include "showTSP.h"
#include "readTSPLIB.h"

#define ITERATIONS		(int) 100

#define NUMBEROFANTS	(int) 10

// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA			(double) 0.5
// if (BETA  == 0) { sub-optimal route }
#define BETA			(double) 0.8
// Estimation of the suspected best route.
#define Q				(double) 80
// Pheromones evaporation. 
#define RO				(double) 0.2
// Maximum pheromone random number.
#define TAUMAX			(int) 2

#define INITIALCITY		(int) 0

int main(int argc, char** argv) {
	// input from *.tsp
    readTSPLIB rtsp;
    rtsp.readFile(argc, argv);
    int** city = rtsp.CITY;
    int N = rtsp.NUMBEROFCITIES;
    // initialize
    showTSP *s = new showTSP(N, city);
    s->initGL(argc, argv);

	ACO *ANTS = new ACO (NUMBEROFANTS, N, 
			 			ALPHA, BETA, Q, RO, TAUMAX,
			 			INITIALCITY);

	ANTS -> init(city);

	ANTS -> optimize (ITERATIONS, s);

	std::cout << "optimized" << std::endl;

	s->MainLoop();

	return 0;
}
