#include "Randoms.cpp"
#include "showTSP.h"

class ACO {
public:
	ACO (int nAnts, int nCities, 
		double alpha, double beta, double q, double ro, double taumax,
		int initCity);
	virtual ~ACO ();
	
	void init (int** city);
	
	void printPHEROMONES ();
	void printGRAPH ();
	
	void optimize (int ITERATIONS, showTSP* s);

	int* GetRESULTS ();

private:
	double distance (int cityi, int cityj);
	bool exists (int cityi, int cityc);
	bool vizited (int antk, int c);
	double PHI (int cityi, int cityj, int antk);
	
	double length (int antk);
	
	int city ();
	void route (int antk);
	int valid (int antk, int iteration);
	
	void updatePHEROMONES ();

	
	int NUMBEROFANTS, NUMBEROFCITIES, INITIALCITY;
	double ALPHA, BETA, Q, RO, TAUMAX;
	
	double BESTLENGTH;
	int *BESTROUTE;

	int **GRAPH, **ROUTES, **CITIES;
	double **PHEROMONES, **DELTAPHEROMONES, **PROBS;

	Randoms *randoms;
};

