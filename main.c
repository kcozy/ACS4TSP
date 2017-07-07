
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
#define ITERATIONS (int)2500
// アリの数
#define NUMBEROFANTS (int)10
// 都市の数
#define NUMBEROFCITIES (int)n
// フェロモンの優先度
// if (ALPHA == 0) { stochastic search & sub-optimal route }
#define ALPHA (double)1.0
// 距離の優先度
// if (BETA  == 0) { sub-optimal route }
#define BETA (double)2.0
// フェロモンの蒸発率
// Pheromones evaporation.
#define RO (double)0.1
// For ACS, global updateをする確率[0,1], フェロモン*距離の逆数が一番高い都市を選ぶ確率
#define Q0 (double)0.9
// For ACS, local updateの割合[0,1]
#define FI (double)0.1

#define INITIALCITY (int)0

// For NN-ACS
double XI;

int BESTLENGTH;
// 全経路のフェロモン初期値
double TAU0;

int MaxTauEta;
/* GRAPH[i][j] : if(i-jに道) 1 else 0 
 * RPUTES[i][j] : アリiがj番目に行く都市
 */
int **GRAPH, **ROUTES;
/* PHEROMONES[i][j] : 道i-jのフェロモン量
 * DELTAPHEROMONS[i][j] : 道i-jのフェロモン変化量 Q/アリkの順路長
 * PROB[i][0] : i番目の都市へ行く時のフェロモン値, PROB[i][1] : i番目の都市のID
 */
double **PHEROMONES, **DELTAPHEROMONES, **PROBS;

int *NN;

void init()
{
	NN = (int *)malloc(sizeof(int)*NUMBEROFCITIES);
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
			if(i != j) GRAPH[i][j] = 1;
			else	GRAPH[i][j] = 0;

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

	BESTLENGTH = INT_MAX;
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
	free(NN);
}

double Uniforme()
{
	return (double)rand()/RAND_MAX;
}

int distance(int cityi, int cityj)
{
	return (int)
		(sqrt(pow(city[cityi][0] - city[cityj][0], 2) +
			 pow(city[cityi][1] - city[cityj][1], 2))
			 + 0.5);
}

int exists(int cityi, int cityc)
{
	return (GRAPH[cityi][cityc] == 1);
}

void NearestNeighbor() {
    for(int i=0; i<NUMBEROFCITIES; i++) {
		NN[i] = i;
	}
	
	for (int i = 0; i < NUMBEROFCITIES-2; i++) {
        int posmin;
		int distmin = INT_MAX;
        for (int v = i+1; v < NUMBEROFCITIES; v++) {
            int dist = distance(NN[i], NN[v]);
            if (dist < distmin) {
                posmin = v;
                distmin = dist;
            }
        }
        int t = NN[i+1];
		NN[i+1] = NN[posmin];
		NN[posmin] = t;
    }

	int d = 0;
	for(int i=0; i<NUMBEROFCITIES-1; i++) {
		d += distance(NN[i], NN[i+1]);
	}
	d += distance(NN[NUMBEROFCITIES-1], NN[0]);

	TAU0 = (double) 1.0/(d*NUMBEROFCITIES);
	//showTour(NN, 5000, 0);
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
///*
	int d = 0;
	for(int i=0; i<NUMBEROFCITIES-1; i++) {
		d += distance(NN[i], NN[i+1]);
	}
	d += distance(NN[NUMBEROFCITIES-1], NN[0]);

	for(int i=0; i<NUMBEROFCITIES-1; i++) {
		if(distance(NN[i],NN[i+1]) < (double) d/NUMBEROFCITIES) {
			PHEROMONES[NN[i]][NN[i+1]] = TAU0*XI;
			PHEROMONES[NN[i+1]][NN[i]] = PHEROMONES[NN[i]][NN[i+1]];
		}
	}
	if(distance(NN[0],NN[NUMBEROFCITIES-1]) < (double) d/NUMBEROFCITIES) {
		PHEROMONES[NN[NUMBEROFCITIES-1]][NN[0]] = TAU0*XI;
		PHEROMONES[NN[0]][NN[NUMBEROFCITIES-1]] = PHEROMONES[NN[NUMBEROFCITIES-1]][NN[0]];
	}
//*/
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

double PHI(int cityi, int cityj, int antk)
{
	double ETAij = pow((double)(1.0 / distance(cityi, cityj)), BETA);
	double TAUij = pow(PHEROMONES[cityi][cityj], ALPHA);

	double sum = 0.0;
	for (int c = 0; c < NUMBEROFCITIES; c++)
	{
		if (exists(cityi, c))
		{
			if (!visited(antk, c))
			{
				double ETA = pow((double)(1.0 / distance(cityi, c)), BETA);
				double TAU = pow(PHEROMONES[cityi][c], ALPHA);
				sum += ETA * TAU;
			}
		}
	}
	return (sum != 0) ? (ETAij * TAUij) / sum : 0;
}

int Length(int antk)
{
	int sum = 0;
	for (int j = 0; j < NUMBEROFCITIES - 1; j++)
	{
		sum += distance(ROUTES[antk][j], ROUTES[antk][j + 1]);
	}
	sum += distance(ROUTES[antk][NUMBEROFCITIES-1], ROUTES[antk][0]);
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
			city = (int) PROBS[i][1];
		}
	}
	return city;
}

int City(int count)
{
	double Q = Uniforme();
	if (Q <= Q0)
	{
		// ACS
		return MaxTauEtaCity(count);
	}
	else
	{
		// AS
		int i = 0;
		double sum = PROBS[i][0];
		double t = Uniforme();
		while (sum < t)
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
int valid(int antk)
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

void GlobalUpdatePHEROMONES()
{
	for (int k = 0; k < NUMBEROFANTS; k++)
	{
		if (Length(k) < BESTLENGTH)
		{
			BESTLENGTH = Length(k);
			for(int i=0; i<NUMBEROFCITIES; i++) 
				tour[i] = ROUTES[k][i];
			
			//showString("Updated");
			//showTour(tour, 10, 0);
		}
	}
	for (int r = 0; r < NUMBEROFCITIES - 1; r++)
	{
		int cityi = tour[r];
		int cityj = tour[r + 1];
		DELTAPHEROMONES[cityi][cityj] += (double) (1.0 / BESTLENGTH);
		DELTAPHEROMONES[cityj][cityi] += (double) (1.0 / BESTLENGTH);
	}
	DELTAPHEROMONES[tour[NUMBEROFCITIES-1]][tour[0]] += (double) (1.0 / BESTLENGTH);
	DELTAPHEROMONES[tour[0]][tour[NUMBEROFCITIES-1]] += (double) (1.0 / BESTLENGTH);

	for (int i = 0; i < NUMBEROFCITIES; i++)
	{
		for (int j = 0; j < NUMBEROFCITIES; j++)
		{
			PHEROMONES[i][j] = (1.0 - RO) * PHEROMONES[i][j] + RO * DELTAPHEROMONES[i][j];
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
			if(PHEROMONES[i][j] != 0) PHEROMONES[i][j] = (1.0 - FI) * PHEROMONES[i][j] + FI * TAU0;
		}
	}
}

void optimize()
{
	for (int iterations = 1; iterations <= ITERATIONS; iterations++)
	{
		for (int k = 0; k < NUMBEROFANTS; k++)
		{
			while (0 != valid(k))
			{
				route(k);
			}
			LocalUpdatePHEROMONES();	
		}

		GlobalUpdatePHEROMONES();

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
	double param[10] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0};
	FILE *fp;
	char s[256];
	if ((fp = fopen("output.txt", "w")) == NULL) {
		printf("file open error!!\n");
		exit(EXIT_FAILURE);	/* (3)エラーの場合は通常、異常終了する */
	}

	for(int p=0; p<10; p++) {
		XI = param[p];
		sprintf(s, "XI = %f\n", XI);
		fputs(s,fp);
	for(int i=0; i<25; i++) {
	init();
	//printf("initialized\n");

	NearestNeighbor();
	initPHEROMONES();

	//printf("connected!\n");

	optimize();

	sprintf(s, "%d\n", BESTLENGTH);
	fputs(s, fp);
	//printf("BEST LENGTH = %d\n", BESTLENGTH);

	end();
	}
	}

	fclose(fp);

	return 1;
}
