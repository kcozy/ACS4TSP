

//: Quick help
/// Ver. 2009.09.24

/// ### Option
/// # make problem
/// [fileName] : set cityFile
/// -r [number] : make random cities list
/// -s [number] : rundom seed
///
/// # show
/// -n : ignore graphic
/// -a : ignore aspect ratio
/// -w [number] : wait percentage
/// 
/// # output
/// -c : output cities list
/// -t : output tour list
///
/// # for debug or test
/// -z [number] : rotate cities

/// ### Compile
/// gcc -O2 -I/usr/include/X11/ -L/usr/lib/ -lX11 -lm
//$     tspShow.c [inputFileName.c] -o [outputFileName]
///

/// ### InputFile
//;

//: Head
#include<stdio.h>
#include<stdlib.h>

/// tspr
#include<string.h>

/// tspg
#include<math.h>
#include<float.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>

/// tspr

/// max of number of cities 
#define MAX 20000
/// string buffer size
#define BUFSIZE 1024

//;

//: extern
extern int tspSolver(void); 
//;

//: Global

int n, length, tour[MAX];
float city[MAX][2];

int rndSeed = 127;
int rndRange = 1000;

/// tspg
int winSize = 400;
int winMarg = 50;

int bgColor = 0xFFFFFF;
int ctColor = 0x000072;
int toColor[] = {0x000072, 0xBB0000, 0x004000, 0x7f7f7f};
int toCNum = 4;

Display *dis;
Window win;
Pixmap pm;
XSetWindowAttributes att;
XEvent ev;
GC gc, gcb;

int *icity;
int waitPer = 100;
int gActive = 1;

//;

//: 'showing functions'

//: "showTour"
void showTour(int *tou, int wai, int color)
{
  int i;

  if(gActive == 0) return;

  //XClearWindow(dis, win);
  XFillRectangle(dis, pm, gcb, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2);
  
  XSetForeground(dis, gc, ctColor);
  for(i = 0; i < n; i++) {
    XDrawRectangle(dis, pm, gc, icity[i * 2] - 2, icity[i * 2 + 1] - 2, 4, 4);
  }
  
  if(color < 0 || toCNum <= color) color = 0;
  XSetForeground(dis, gc, toColor[color]);

  for(i = 0; i < n - 1; i++) {
    XDrawLine(dis, pm, gc, icity[tou[i] * 2],
        icity[tou[i] * 2 + 1], 
        icity[tou[i + 1] * 2],
        icity[tou[i + 1] * 2 + 1]);
  }
  XDrawLine(dis, pm, gc, icity[tou[n - 1] * 2],
      icity[tou[n - 1] * 2 + 1],
      icity[tou[0] * 2],
      icity[tou[0] * 2 + 1]);
 
  XCopyArea(dis, pm, win, gc, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2, 0, 0);
  XFlush(dis);

  if(wai != 0 && waitPer != 0) {
    usleep(wai * waitPer * 10);
  }
  
  return;
}
//;

//: "showCTour"
void showCTour(int *tou, int wai, int *colors)
{
  int t1;
  int i;
  
  if(gActive == 0) return;

  XFillRectangle(dis, pm, gcb, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2);
  
  XSetForeground(dis, gc, ctColor);
  for(i = 0; i < n; i++) {
    XDrawRectangle(dis, pm, gc, icity[i * 2] - 2, icity[i * 2 + 1] - 2, 4, 4);
  }
  
  for(i = 0; ; i++) {
    t1 = colors[i];
    if(t1 < 0 || toCNum <= t1) t1 = 0;
    XSetForeground(dis, gc, t1);
 
    if(i == n - 1) break;
    XDrawLine(dis, pm, gc, icity[tou[i] * 2],
        icity[tou[i] * 2 + 1], 
        icity[tou[i + 1] * 2],
        icity[tou[i + 1] * 2 + 1]);
  }
  XDrawLine(dis, pm, gc, icity[tou[n - 1] * 2],
      icity[tou[n - 1] * 2 + 1],
      icity[tou[0] * 2],
      icity[tou[0] * 2 + 1]);

 
  XCopyArea(dis, pm, win, gc, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2, 0, 0);
  XFlush(dis);

  if(wai != 0 && waitPer != 0) {
    usleep(wai * waitPer * 10);
  }

  return;
}
//;

//: "showNTour"
void showNTour(int *next, int start, int wai, int color) 
{
  int i, j;
  
  if(gActive == 0) return;
  
  //XClearWindow(dis, win);
  XFillRectangle(dis, pm, gcb, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2);
  
  XSetForeground(dis, gc, ctColor);
  for(i = 0; i < n; i++) {
    XDrawRectangle(dis, pm, gc, icity[i * 2] - 2, icity[i * 2 + 1] - 2, 4, 4);
  }
  
  if(color < 0 || toCNum <= color) color = 0;
  XSetForeground(dis, gc, toColor[color]);

  j = start;
  for(i = next[start]; ; i = next[i]) {
    XDrawLine(dis, pm, gc, icity[i * 2],
        icity[i * 2 + 1], 
        icity[j * 2],
        icity[j * 2 + 1]);
    if(i == start) break;
    j = i;
  }
  
  XCopyArea(dis, pm, win, gc, 0, 0, winSize + winMarg * 2, winSize + winMarg * 2, 0, 0);
  XFlush(dis);

  if(wai != 0 && waitPer != 0) {
    usleep(wai * waitPer * 10);
  }
  
  return;
}
//;

//: "showString"
void showString(char *str)
{
  if(gActive == 0) return;
  
  XStoreName(dis, win, str);
  
  return;
}
//;

//: "showLength"
void showLength(int leng)
{
  fprintf(stderr, "length = %d           \r", leng);
  
  return;
}
//;

//;

//: "main"
int main(int argc, char *argv[]) 
{ 
  //: local variable
  /// tspr
  FILE *fp;
  int fflag = 0;
  char buf[BUFSIZE];
  char *c1, *c2;
  char ct;
  float cBuf[3];
  
  int rndm = 0;
  int sed = rndSeed;
  int zur = 0;
  int bar = 0;
  int cis = 0;
  int tos = 0;

  /// tspg
  double maxx = DBL_MIN;
  double maxy = DBL_MIN;
  double minx = DBL_MAX;
  double miny = DBL_MAX;
  double cenx, ceny;
  double wid, hei;
  double d;
  
  int as = 0;
  
  float xd, yd;
  int t1;
  int i, j, fl;
  //;

  //: option
  for(i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      for(j = 1; ; j++) {
        if(argv[i][j] == 'r') {
          rndm = 1;
          i++;
          if(i >= argc) {
            fprintf(stderr, "no number!\n");
            exit(1);
          }
          n = atoi(argv[i]);
          if(n < 3 || MAX < n) {
            fprintf(stderr, "bad number!\n");
            exit(1);
          }
          break;
        } else if(argv[i][j] == 's') {
          i++;
          if(i >= argc) {
            fprintf(stderr, "no seed!\n");
            exit(1);
          }
          sed = atoi(argv[i]);
          break;
        } else if(argv[i][j] == 'z') {
          i++;
          if(i >= argc) {
            fprintf(stderr, "no zurashi!\n");
            exit(1);
          }
          zur = atoi(argv[i]);
          if(zur < 0) {
            fprintf(stderr, "bad zurashi!\n");
            exit(1);
          }
          break;
        } else if(argv[i][j] == 'b') {
          i++;
          if(i >= argc) {
            fprintf(stderr, "no barashi!\n");
            exit(1);
          }
          bar = atoi(argv[i]);
          if(bar < 0) {
            fprintf(stderr, "bad barashi!\n");
            exit(1);
          }
          break;
        } else if(argv[i][j] == 'c') {
          cis = 1;
        } else if(argv[i][j] == 't') {
          tos = 1;
        } else if(argv[i][j] == 'a') {
          as = 1;
        } else if(argv[i][j] == 'n') {
          gActive = 0;
        } else if(argv[i][j] == 'w') {
          i++;
          if(i >= argc) {
            fprintf(stderr, "no wait!\n");
            exit(1);
          }
          waitPer = atoi(argv[i]);
          if(waitPer < 0) {
            waitPer = 0;
          }
          break;
        } else {
          break;
        }
      }
    } else if(fflag == 0) {
      if((fp = fopen(argv[i], "r")) == NULL) {
        fprintf(stderr, "cannot open file!\n");
        exit(1);
      }
      fflag = 1;
    }
  }
  //;
  
  //: make cities list
  if(rndm == 0 || fflag == 1) {
    
    if(fflag == 0) {
      fp = stdin;
    }

    //: read dimension
    n = 0;
    while(fgets(buf, BUFSIZE, fp) != NULL) {
      ct = buf[0];
      if(ct == 'N') {
        if(strncmp("NODE_CORD_SECTION", buf, 17) == 0) break;
        if(strncmp("NODE_COORD_SECTION", buf, 18) == 0) break;
        if(strncmp("NAME", buf, 4) == 0) { }
      } else if(ct == 'D') {
        if(strncmp("DIMENSION", buf, 9) == 0) {
          for(c1 = buf + 9; *c1 != ':'; c1++); 
          n = atoi(c1 + 1); 
        }
      } else if(ct == 'T') {
        if(strncmp("TYPE:", buf, 5) == 0) { }
      } else if(ct == 'C') {
        if(strncmp("COMMENT:", buf, 5) == 0) { }
      } else if(ct == 'E') {
        if(strncmp("EDGE_WEIGHT_TYPE:", buf, 17) == 0) { }
      } else if('0' <= ct && ct <= '9') {
        n = atoi(buf);
        break;
      }
    }
    
    if(n < 3 || n > MAX) {
      fprintf(stderr, "bad DIMENSION!\n");
      exit(1);
    }
    //;
    
    //: read cities list
    for(i = 0; i < n; i++) {
      if(fgets(buf, BUFSIZE, fp) == NULL) {
        fprintf(stderr, "city data error!\n");
        exit(1);
      }
      
      c1 = buf;
      j = 0;
      fl = 0;
      for(c2 = buf; ; c2++) {
        if(*c2 == ' ' || *c2 == '\0') {
          if(*c2 == ' ') {
            *c2 = '\0';
            fl = 1;
          }
          
          if(c1 != c2) {
            cBuf[j] = (float)atof(c1);
            j++;
            if(j >= 3) break;
          }
          
          c1 = c2 + 1;
          if(fl == 1) {
            fl = 0;
          } else {
            break;
          }
        }
      }
      
      t1 = (i + zur) % n;
      if(j == 3) {
        if((int)(cBuf[0]) != i + 1) {
          fprintf(stderr, "city data error!\n");
          exit(1);
        }
        city[t1][0] = cBuf[1];
        city[t1][1] = cBuf[2];
      } else {
        city[t1][0] = cBuf[0];
        city[t1][1] = cBuf[1];
      }
    }
    //;
    
    if(fflag == 1) {
      fclose(fp);
    } else {
      fp = NULL;
    }
    
  //;

  //: make random cities list
  } else {
    
    srand(sed);  

    for(i = 0; i < n; i++) {
      for(j = 0; j < 2; j++) {
        city[(i + zur) % n][j] = (float)( (double)rand() / ((double)RAND_MAX / rndRange) );
      }
    }
  }
  //;

  //: tspg init
  icity = (int *)malloc(sizeof(int) * n * 2);
  
  if(gActive != 0) {
  
    for(i = 0; i < n; i++) {
      d = (double)(city[i][0]);
      if(d > maxx) maxx = d;
      if(d < minx) minx = d;
      d = (double)(city[i][1]);
      if(d > maxy) maxy = d;
      if(d < miny) miny = d;
    }

    cenx = (maxx + minx) / 2;
    ceny = (maxy + miny) / 2;
    
    if(maxx != minx) {
      wid = maxx - minx; 
    } else {
      wid = 1.0;
    }
    if(maxy != miny) {
      hei = maxy - miny;
    } else {
      wid = 1.0;
    }
    if(as == 0) {
      if(wid <= hei) {
        wid = hei;
      } else {
        hei = wid;
      }
    }
    for(i = 0; i < n; i++) {
      icity[i * 2] = (int)((city[i][0] - cenx) * winSize / wid) + winMarg + winSize / 2;
      icity[i * 2 + 1] = - (int)((city[i][1] - ceny) * winSize / hei) + winMarg + winSize / 2;
    }
    for(i = 0; i < n; i++) {
      tour[i] = i;
    }
    
    dis = XOpenDisplay(NULL);
    win = XCreateSimpleWindow(dis, RootWindow(dis, 0),
            50, 50, winSize + winMarg * 2, winSize + winMarg * 2,
            3, ctColor, bgColor);
    pm = XCreatePixmap(dis, win, winSize + winMarg * 2, winSize + winMarg * 2,
           DefaultDepth(dis, 0));

    XSetStandardProperties(dis, win, "tspShow", "tspShow", None, argv, argc, NULL);
    
    //att.backing_store = WhenMapped;
    //XChangeWindowAttributes(dis, win, CWBackingStore, &att);
    
    XMapWindow(dis, win);
    XFlush(dis);
    
    XSelectInput(dis, win, ButtonPressMask | ExposureMask);
    
    /*
      do {
      XNextEvent(dis, &ev);
      }while(ev.type != Expose);
      printf("0K\n");
    */
    usleep(500000);

    gc = XCreateGC(dis, win, 0, 0);
    gcb = XCreateGC(dis, win, 0, 0);
    XSetForeground(dis, gcb, bgColor);
    XSetBackground(dis, gcb, bgColor);
  }
  //;

  //enu
  tspSolver();
  
  //: solved
  if(gActive != 0) {
    showTour(tour, 0, 0);
    showString("finish!");
  }
  
  if(cis == 1) {
    printf("%d\n", n);
    for(i = 0; i < n; i++) {
      printf("%f %f\n", city[tour[i]][0], city[tour[i]][1]);
    }
  }
  if(tos == 1) {
    for(i = 0; i < n; i++) {
      printf("%d ", tour[i]);
    }
    printf("\n");
  }
  /* hide length showing
  t1 = 0;
  for(i = 0; i < n; i++) {
    xd = city[tour[i]][0] - city[tour[(i + 1) % n]][0];
    yd = city[tour[i]][1] - city[tour[(i + 1) % n]][1];
    t1 += (int)(sqrt(xd * xd + yd * yd) + .5);
  }
  if(t1 != length) {
    fprintf(stderr, "length = %d (%d?)\n", t1, length); 
  } else {
    fprintf(stderr, "length = %d            \n", t1);
  }
  */
  //;
  
  //: end
  if(gActive != 0) {
    do {
      XNextEvent(dis, &ev);
    }while(ev.type != ButtonPress);
  
    XFreeGC(dis, gc);
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
  }
  //;
  
  return 0;
}
//;


