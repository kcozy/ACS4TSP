
/// ###
/// int tspSolver(void) を宣言して，その中に自分のアルゴリズムを書いてください．
/// 入出力データはグローバル変数でやりとりします．
/// 入力データは 都市数 n，都市座標の二次元配列 city[n][2] として入力されます．
/// 出力データは 順回路 tour[n], 順回路長 length で出力しますが，
/// length は最後に計算し直されるので，あまり出力する意味はありません．

/// 使用するグローバル変数等．必須項目
#define MAX 20000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <string.h>

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

// イテレーション(ループ)回数
#define ITERATIONS (int)500
// アリの数
#define NUMBEROFANTS (int)10
// 都市の数
#define NUMBEROFCITIES (int)n
// フェロモンの優先度
// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA (double)1
// 距離の優先度
// if (BETA  == 0) { sub-optimal route }
#define BETA (double)2
// 総距離の影響を左右する定数
// Estimation of the suspected best route.
#define Q (double)1
// フェロモンの蒸発率
// Pheromones evaporation.
#define RO (double)0.1
// Maximum pheromone random number.
// #define TAUMAX (int)2
// For ACS, global updateをする確率[0,1], フェロモン*距離の逆数が一番高い都市を選ぶ確率
#define Q0 (double)0.8
// For ACS, local updateの割合[0,1]
#define FI (double)0.1

#define INITIALCITY (int)0

double BESTLENGTH, BEST;
// 全経路のフェロモン初期値
double TAU0;

int MaxTauEta;
/* GRAPH[i][j] : if(i-jに道) 1 else 0 
 * RPUTES[i][j] : アリiがj番目に行く都市
 */
int **GRAPH, **ROUTES, *BESTSOFAR;
/* PHEROMONES[i][j] : 道i-jのフェロモン量
 * DELTAPHEROMONS[i][j] : 道i-jのフェロモン変化量 Q/アリkの順路長
 * PROB[i][0] : i番目の都市へ行く時のフェロモン値, PROB[i][1] : i番目の都市のID
 */
double **PHEROMONES, **DELTAPHEROMONES, **PROBS;

void init()
{
	BESTSOFAR = (int *)malloc(sizeof(int)*NUMBEROFCITIES);
	GRAPH = (int **)malloc(sizeof(int *) * NUMBEROFCITIES);
	PHEROMONES = (double **)malloc(sizeof(double *) * NUMBEROFCITIES);
	DELTAPHEROMONES = (double **)malloc(sizeof(double *) * NUMBEROFCITIES);
	PROBS = (double **)malloc(sizeof(double *) * NUMBEROFCITIES - 1);
	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		GRAPH[i] = (int *)malloc(sizeof(int) * NUMBEROFCITIES);
		PHEROMONES[i] = (double *)malloc(sizeof(double) * NUMBEROFCITIES);
		DELTAPHEROMONES[i] = (double *)malloc(sizeof(double) * NUMBEROFCITIES);
		PROBS[i] = (double *)malloc(sizeof(double) * 2);
		for (int j = 0; j < 2; j++)
		{
			PROBS[i][j] = -1.0;
		}
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			GRAPH[i][j] = 0;
			PHEROMONES[i][j] = 0.0;
			DELTAPHEROMONES[i][j] = 0.0;
		}
	}

	ROUTES = (int **)malloc(sizeof(int *) * NUMBEROFANTS);
	for (int i = 0; i < NUMBEROFANTS; i++)
	{
		ROUTES[i] = (int *)malloc(sizeof(int) * NUMBEROFCITIES);
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			ROUTES[i][j] = -1;
		}
	}

	BESTLENGTH = DBL_MAX;
	BEST = DBL_MAX;
	showString("Initialized!");
	srand((unsigned)time(NULL));
	MaxTauEta = 0;
}

void end()
{
	for (int i = 0; i < NUMBEROFANTS; i++)
	{
		free(ROUTES[i]);
	}
	free(ROUTES);
	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		free(GRAPH[i]);
		free(PHEROMONES[i]);
		free(DELTAPHEROMONES[i]);
		free(PROBS[i]);
	}
	free(GRAPH);
	free(PHEROMONES);
	free(DELTAPHEROMONES);
	free(PROBS);
}

uint32_t xor (void) {
	static uint32_t y = 2463534242;
	y = y ^ (y << 13);
	y = y ^ (y >> 17);
	return y = y ^ (y << 5);
}

double Uniforme()
{
	uint32_t y = UINT32_MAX;
	return (double) xor() / y;
	//return (double)rand()/RAND_MAX;
}

double distance(int cityi, int cityj)
{
	return (double)
		sqrt(pow(city[cityi][0] - city[cityj][0], 2) +
			 pow(city[cityi][1] - city[cityj][1], 2));
}

int exists(int cityi, int cityc)
{
	return (GRAPH[cityi][cityc] == 1);
}

void NearestNeighbor()
{
	int visited[NUMBEROFCITIES];
	double totalLen = 0.0;
	int NN[NUMBEROFCITIES];

	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		memset(visited, -1, sizeof(visited));
		int cityi = i;
		for (int j = 0; j < NUMBEROFCITIES - 1; j++)
		{
			double leng = DBL_MAX;
			NN[j] = cityi;
			visited[cityi] = 1;
			for (int cityj = 0; cityj < NUMBEROFCITIES; cityj++)
			{
				if (visited[cityj] != -1)
					continue;
				if (distance(cityi, cityj) < leng)
				{
					leng = distance(cityi, cityj);
					cityi = cityj;
				}
			}
			totalLen += leng;
		}
		NN[NUMBEROFCITIES - 1] = i;
		totalLen += distance(NN[NUMBEROFCITIES - 2], NN[NUMBEROFCITIES - 1]);
		//showTour(NN,1000,0);
		//for(int k = 0; k < NUMBEROFCITIES; k++) printf("%d ", NN[k]);
	}
	
	TAU0 = 1.0/totalLen;
}

void initPHEROMONES() {
	for(int i=0; i<NUMBEROFCITIES; i++) {
		for(int j=0; j<NUMBEROFCITIES; j++) {
			if(exists(i,j)) {
				PHEROMONES[i][j] = TAU0;
				PHEROMONES[j][i] = PHEROMONES[i][j];
			}
		}
	}
}

void connectCITIES(int cityi, int cityj)
{
	GRAPH[cityi][cityj] = 1;
	//PHEROMONES[cityi][cityj] = TAU0; //Uniforme() * TAUMAX;
	GRAPH[cityj][cityi] = 1;
	//PHEROMONES[cityj][cityi] = PHEROMONES[cityi][cityj];
}

int visited(int antk, int c)
{
	for (int l = 0; l < NUMBEROFCITIES; l++)
	{
		if (ROUTES[antk][l] == -1)
		{
			break;
		}
		if (ROUTES[antk][l] == c)
		{
			return 1;
		}
	}
	return 0;
}

void printPHEROMONES()
{
	printf(" PHEROMONES: \n");
	printf("  |");
	for (int i = 0; i < NUMBEROFCITIES; i++)
		printf("%5d   ", i);
	printf("\n- | ");
	for (int i = 0; i < NUMBEROFCITIES; i++)
		printf("--------");
	printf("\n");
	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		printf("%d | ", i);
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			if (i == j)
			{
				printf("%5s   ", "x");
				continue;
			}
			if (exists(i, j))
			{
				printf("%7.3f ", PHEROMONES[i][j]);
			}
			else
			{
				if (PHEROMONES[i][j] == 0.0)
				{
					printf("%5.0f   ", PHEROMONES[i][j]);
				}
				else
				{
					printf("%7.3f ", PHEROMONES[i][j]);
				}
			}
		}
		printf("\n");
	}
	printf("\n");
}

double PHI(int cityi, int cityj, int antk)
{
	double ETAij = (double)pow(1 / distance(cityi, cityj), BETA);
	double TAUij = (double)pow(PHEROMONES[cityi][cityj], ALPHA);

	double sum = 0.0;
	for (int c = 0; c < NUMBEROFCITIES; c++)
	{
		if (exists(cityi, c))
		{
			if (!visited(antk, c))
			{
				double ETA = (double)pow(1 / distance(cityi, c), BETA);
				double TAU = (double)pow(PHEROMONES[cityi][c], ALPHA);
				sum += ETA * TAU;
			}
		}
	}
	return (ETAij * TAUij) / sum;
}

double Length(int antk)
{
	double sum = 0.0;
	for (int j = 0; j < NUMBEROFCITIES - 1; j++)
	{
		sum += distance(ROUTES[antk][j], ROUTES[antk][j + 1]);
	}
	return sum;
}

// arg max {tau * eta^beta}
int MaxTauEtaCity(int count)
{
	double t = 0.0;
	int city;
	for(int i=0; i<count; i++){
		if(PROBS[i][0] >= t) {
			t = PROBS[i][0];
			city = PROBS[i][1];
		}
	}
	return city;
}

int City(int count)
{
	double xi = Uniforme();
	if (xi <= Q0)
	{
		// ACS
		return MaxTauEtaCity(count);
	}
	else
	{
		// AS
		int i = 0;
		double sum = PROBS[i][0];
		while (sum < xi)
		{
			i++;
			sum += PROBS[i][0];
		}
		return (int)PROBS[i][1];
	}
}

void route(int antk)
{
	ROUTES[antk][0] = INITIALCITY;
	for (int i = 0; i < NUMBEROFCITIES - 1; i++)
	{
		int cityi = ROUTES[antk][i];
		int count = 0;
		for (int c = 0; c < NUMBEROFCITIES; c++)
		{
			if (cityi == c)
			{
				continue;
			}
			if (exists(cityi, c))
			{
				if (!visited(antk, c))
				{
					PROBS[count][0] = PHI(cityi, c, antk);
					PROBS[count][1] = (double)c;
					count++;
				}
			}
		}

		// deadlock
		if (0 == count)
		{
			return;
		}

		ROUTES[antk][i + 1] = City(count);
	}
}

// antkが有効な巡回路を通ったか
int valid(int antk, int iteration)
{
	for (int i = 0; i < NUMBEROFCITIES - 1; i++)
	{
		int cityi = ROUTES[antk][i];
		int cityj = ROUTES[antk][i + 1];
		if (cityi < 0 || cityj < 0)
		{
			return -1;
		}
		if (!exists(cityi, cityj))
		{
			return -2;
		}
		for (int j = 0; j < i - 1; j++)
		{
			if (ROUTES[antk][i] == ROUTES[antk][j])
			{
				return -3;
			}
		}
	}

	if (!exists(INITIALCITY, ROUTES[antk][NUMBEROFCITIES - 1]))
	{
		return -4;
	}

	return 0;
}

void OfflineUpdatePHEROMONES()
{
	/*
	for (int k=0; k<NUMBEROFANTS; k++) {
		double rlength = Length(k);
		for (int r=0; r<NUMBEROFCITIES-1; r++) {
			int cityi = ROUTES[k][r];
			int cityj = ROUTES[k][r+1];
			DELTAPHEROMONES[cityi][cityj] += Q / rlength;
			DELTAPHEROMONES[cityj][cityi] += Q / rlength;
		}
	}
	*/
	// ACS
	for (int k = 0; k < NUMBEROFANTS; k++)
	{
		if (Length(k) < BEST)
		{
			BEST = Length(k);
			BESTSOFAR = ROUTES[k];
		}
	}
	for (int r = 0; r < NUMBEROFCITIES - 1; r++)
	{
		int cityi = BESTSOFAR[r];
		int cityj = BESTSOFAR[r + 1];
		DELTAPHEROMONES[cityi][cityj] += Q / BEST;
		DELTAPHEROMONES[cityj][cityi] += Q / BEST;
	}

	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			PHEROMONES[i][j] = (1 - RO) * PHEROMONES[i][j] + RO * DELTAPHEROMONES[i][j];
			DELTAPHEROMONES[i][j] = 0.0;
		}
	}
}

void LocalUpdatePHEROMONES()
{
	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			if(exists(i,j))
				PHEROMONES[i][j] = (1 - FI) * PHEROMONES[i][j] + FI * TAU0;
		}
	}
}

void optimize()
{
	for (int iterations = 1; iterations <= ITERATIONS; iterations++)
	{
		for (int k = 0; k < NUMBEROFANTS; k++)
		{
			while (0 != valid(k, iterations))
			{
				for (int i = 0; i < NUMBEROFCITIES; i++)
				{
					ROUTES[k][i] = -1;
				}
				route(k);
			}
			double rlength = Length(k);

			if (rlength < BESTLENGTH)
			{
				BESTLENGTH = rlength;
				for (int i = 0; i < NUMBEROFCITIES; i++)
				{
					tour[i] = ROUTES[k][i];
				}
				showString("Updated");
				showTour(tour, 100, 0);
				showString("serching");
			}
			else
			{
				//showTour(ROUTES[k], 10, 1);
			}
			LocalUpdatePHEROMONES();
		}

		OfflineUpdatePHEROMONES();
		//printPHEROMONES();

		for (int i = 0; i < NUMBEROFANTS; i++)
		{
			for (int j = 0; j < NUMBEROFCITIES; j++)
			{
				ROUTES[i][j] = -1;
			}
		}
	}
}

int tspSolver(void)
{
	init();
	printf("initialized\n");

	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		for (int j = i + 1; j < NUMBEROFCITIES; j++)
		{
			connectCITIES(i, j);
		}
	}

	NearestNeighbor();
	initPHEROMONES();

	printf("connected!\n");

	optimize();

	printf("BEST LENGTH = %f\n", BESTLENGTH);

	end();

	return 1;
}
