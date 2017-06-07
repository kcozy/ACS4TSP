
/// ### 
/// int tspSolver(void) を宣言して，その中に自分のアルゴリズムを書いてください．
/// 入出力データはグローバル変数でやりとりします．
/// 入力データは 都市数 n，都市座標の二次元配列 city[n][2] として入力されます．
/// 出力データは 順回路 tour[n], 順回路長 length で出力しますが，
/// length は最後に計算し直されるので，あまり出力する意味はありません．

/// 使用するグローバル変数等．必須項目
#define MAX 20000

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>

#include "Random.c"

extern int n, length, tour[MAX];
extern float city[MAX][2];

/// 順回路表示のための関数
/// tou[] に順回路の配列を，wai にウェイト時間(ミリ秒)，colorに色 0~3を指定．
extern void showTour(int *tou, int wai, int color);
/// color[] に配列を指定して，枝ごとに色を変えられる順回路表示．
extern void showCTour(int *tou, int wai, int *color);
/// 文字列を表示ウィンドウの上部バーに表示．数字等が表示したい場合 sprintf 等と組み合わせる．
extern void showString(char *str);
/// leng を暫定解として表示(標準エラー出力)
extern void showLength(int leng);
/// ###

//extern double Uniforme();

int tr[MAX];

/// 距離の計算はこの関数と同等の方法で行う．
/// 必ずしもこの関数を残しておく必要は無い．
int Dis(int i, int j)
{
  float xd, yd;

  xd = city[i][0] - city[j][0];
  yd = city[i][1] - city[j][1];
  return (int)(sqrt(xd * xd + yd * yd) + .5);
}

/// 以下 int tspSolver(void) が必要なこと以外は自由に作る．
///

#define ITERATIONS		(int)500

#define NUMBEROFANTS	(int)20
#define NUMBEROFCITIES	(int)n

// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA			(double)0.2
// if (BETA  == 0) { sub-optimal route }
#define BETA			(double)1.0
// Estimation of the suspected best route.
#define Q				(double)100
// Pheromones evaporation. 
#define RO				(double)0.4
// Maximum pheromone random number.
#define TAUMAX			(int)2

#define INITIALCITY		(int)0


double BESTLENGTH;

int **GRAPH, **ROUTES;
double /* **CITIES,*/ **PHEROMONES, **DELTAPHEROMONES, **PROBS;

void init () {
	GRAPH 			= (int **) malloc(sizeof(int*)*NUMBEROFCITIES);
	//CITIES 			= (double **) malloc(sizeof(double)*NUMBEROFCITIES);
	PHEROMONES 		= (double **) malloc(sizeof(double*)*NUMBEROFCITIES);
	DELTAPHEROMONES = (double **) malloc(sizeof(double*)*NUMBEROFCITIES);
	PROBS 			= (double **) malloc(sizeof(double*)*NUMBEROFCITIES-1);
	for(int i=0; i<NUMBEROFCITIES; i++) {
		GRAPH[i] 			= (int *)malloc(sizeof(int)*NUMBEROFCITIES);
		//CITIES[i] 			= new double[2];
		PHEROMONES[i] 		= (double *) malloc(sizeof(double)*NUMBEROFCITIES);
		DELTAPHEROMONES[i] 	= (double *)malloc(sizeof(double)*NUMBEROFCITIES);
		PROBS[i] 			= (double *) malloc(sizeof(double)*2);
		for (int j=0; j<2; j++) {
			//CITIES[i][j] = -1.0;
			PROBS[i][j]  = -1.0;
		}
		for (int j=0; j<NUMBEROFCITIES; j++) {
			GRAPH[i][j] 			= 0;
			PHEROMONES[i][j] 		= 0.0;
			DELTAPHEROMONES[i][j] 	= 0.0;
		}
	}	

	ROUTES = (int **) malloc(sizeof(int*)*NUMBEROFANTS);
	for (int i=0; i<NUMBEROFANTS; i++) {
		ROUTES[i] = (int *) malloc(sizeof(int)*NUMBEROFCITIES);
		for (int j=0; j<NUMBEROFCITIES; j++) {
			ROUTES[i][j] = -1;
		}
	}
	
	BESTLENGTH = (double) INT_MAX;
  showString("Initialized!");
  srand((unsigned)time(NULL));
}

double Uniforme() {
    return (double)rand()/RAND_MAX;
}

void connectCITIES (int cityi, int cityj) {
	GRAPH[cityi][cityj] = 1;
	PHEROMONES[cityi][cityj] = Uniforme() * TAUMAX;
	GRAPH[cityj][cityi] = 1;
	PHEROMONES[cityj][cityi] = PHEROMONES[cityi][cityj];
}

double distance (int cityi, int cityj) {
	return (double) 
		sqrt (pow (city[cityi][0] - city[cityj][0], 2) + 
 			  pow (city[cityi][1] - city[cityj][1], 2));
}

int exists (int cityi, int cityc) {
	return (GRAPH[cityi][cityc] == 1);
}

int visited (int antk, int c) {
	for (int l=0; l<NUMBEROFCITIES; l++) {
		if (ROUTES[antk][l] == -1) {
			break;
		}
		if (ROUTES[antk][l] == c) {
			return 1;
		}
	}
	return 0;
}

void printPHEROMONES() {
  printf(" PHEROMONES: \n");
  printf("  |");
  for(int i=0; i<NUMBEROFCITIES; i++) printf("%5d   ",i);
  printf("\n- | ");
  for(int i=0; i<NUMBEROFCITIES; i++) printf("--------");
  printf("\n");
  for(int i=0; i<NUMBEROFCITIES; i++) {
    printf("%d | ",i);
    for(int j=0; j<NUMBEROFCITIES; j++) {
      if(i == j) {
        printf("%5s   ", "x");
        continue;
      }
      if (exists(i, j)) {
				printf ("%7.3f ", PHEROMONES[i][j]);
			}
			else {
				if(PHEROMONES[i][j] == 0.0) {
					printf ("%5.0f   ", PHEROMONES[i][j]);
				}
				else {
					printf ("%7.3f ", PHEROMONES[i][j]);
				}
			}
    }
    printf("\n");
  }
  printf("\n");
}

double PHI (int cityi, int cityj, int antk) {
	double ETAij = (double) pow (1 / distance (cityi, cityj), BETA);
	double TAUij = (double) pow (PHEROMONES[cityi][cityj],   ALPHA);

	double sum = 0.0;
	for (int c=0; c<NUMBEROFCITIES; c++) {
		if (exists(cityi, c)) {
			if (!visited(antk, c)) {
				double ETA = (double) pow (1 / distance (cityi, c), BETA);
				double TAU = (double) pow (PHEROMONES[cityi][c],   ALPHA);
				sum += ETA * TAU;
			}	
		}	
	}
	return (ETAij * TAUij) / sum;
}

double Length (int antk) {
	double sum = 0.0;
	for (int j=0; j<NUMBEROFCITIES-1; j++) {
		sum += distance (ROUTES[antk][j], ROUTES[antk][j+1]);	
	}
	return sum;
}

int City () {
	double xi =  Uniforme();
	int i = 0;
	double sum = PROBS[i][0];
	while (sum < xi) {
		i++;
		sum += PROBS[i][0];
	}
	return (int) PROBS[i][1];
}

void route (int antk) {
	ROUTES[antk][0] = INITIALCITY;
	for (int i=0; i<NUMBEROFCITIES-1; i++) {		
		int cityi = ROUTES[antk][i];
		int count = 0;
		for (int c=0; c<NUMBEROFCITIES; c++) {
			if (cityi == c) {
				continue;	
			}
			if (exists (cityi, c)) {
				if (!visited (antk, c)) {
					PROBS[count][0] = PHI (cityi, c, antk);
					PROBS[count][1] = (double) c;
					count++;
				}

			}
		}
		
		// deadlock
		if (0 == count) {
			return;
		}
		
		ROUTES[antk][i+1] = City();
	}
}

int valid (int antk, int iteration) {
	for(int i=0; i<NUMBEROFCITIES-1; i++) {
		int cityi = ROUTES[antk][i];
		int cityj = ROUTES[antk][i+1];
		if (cityi < 0 || cityj < 0) {
			return -1;	
		}
		if (!exists(cityi, cityj)) {
			return -2;	
		}
		for (int j=0; j<i-1; j++) {
			if (ROUTES[antk][i] == ROUTES[antk][j]) {
				return -3;
			}	
		}
	}
	
	if (!exists (INITIALCITY, ROUTES[antk][NUMBEROFCITIES-1])) {
		return -4;
	}
	
	return 0;
}

void updatePHEROMONES () {
	for (int k=0; k<NUMBEROFANTS; k++) {
		double rlength = Length(k);
		for (int r=0; r<NUMBEROFCITIES-1; r++) {
			int cityi = ROUTES[k][r];
			int cityj = ROUTES[k][r+1];
			DELTAPHEROMONES[cityi][cityj] += Q / rlength;
			DELTAPHEROMONES[cityj][cityi] += Q / rlength;
		}
	}
	for (int i=0; i<NUMBEROFCITIES; i++) {
		for (int j=0; j<NUMBEROFCITIES; j++) {
			PHEROMONES[i][j] = (1 - RO) * PHEROMONES[i][j] + DELTAPHEROMONES[i][j];
			DELTAPHEROMONES[i][j] = 0.0;
		}	
	}
}


void optimize () {
   //printPHEROMONES();
	for (int iterations=1; iterations<=ITERATIONS; iterations++) {
    //printf("iteration start-----------------------------\n");
		for (int k=0; k<NUMBEROFANTS; k++) {
			char str[100];
      sprintf(str,"[%d] ant %d has been released!",iterations, k);
      //printf("%s\n",str);
      showString(str);
      //showTour(tour, 10, 0);
			while (0 != valid(k, iterations)) {
				//printf("\treleasing ant %d again!\n", k);;
				for (int i=0; i<NUMBEROFCITIES; i++) {
					ROUTES[k][i] = -1;	
				}
				route(k);
			}
/*
			printf("route() done\n");
			for (int i=0; i<NUMBEROFCITIES; i++) {
				printf("%d ",ROUTES[k][i]);	
			}
			printf("\n");
	*/		
			double rlength = Length(k);

			if (rlength < BESTLENGTH) {
				BESTLENGTH = rlength;
				for (int i=0; i<NUMBEROFCITIES; i++) {
					tour[i] = ROUTES[k][i];
				}
        showString("Updated");
        showTour(tour, 1000, 0);
        showString("serching");
			} else {
        showTour(ROUTES[k], 10, 1);
      }
		}		

		//printf("\nupdating PHEROMONES . . .\n");
		updatePHEROMONES ();
    //printPHEROMONES();
    //printf(" done!");
		
		for (int i=0; i<NUMBEROFANTS; i++) {
			for (int j=0; j<NUMBEROFCITIES; j++) {
				ROUTES[i][j] = -1;
			}
		}

		//printf("\nITERATION %d HAS ENDED!\n\n", iterations);
	}
}

int tspSolver(void)
{  
  init();
  printf("initialized\n");
  
  for(int i = 0; i < NUMBEROFCITIES; i++) {
    for(int j = i+1; j < NUMBEROFCITIES; j++) {
      connectCITIES(i,j);
    }
  } 


  printf("connected!\n");

  optimize();

  printf("BEST LENGTH = %f\n", BESTLENGTH);

  return 1;
}


