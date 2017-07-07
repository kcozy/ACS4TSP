#include "showTSP.h"

#include <cstdlib>
#include <string>
#include <unistd.h>

#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>

typedef struct{
    // R,G,B must be [0,1]
    double R,G,B;
}COLOR;

int width, height;
int* CurrentRoute;
int* BestRoute;
double** Pheromones;
std::string BestLength;

double** dc;
int n;

showTSP::showTSP(int n, int** city) {
    NUMBEROFCITIES = n;
    CITIES = new int*[NUMBEROFCITIES];
    DispCITIES = new double*[NUMBEROFCITIES];
    Pheromones = new double*[NUMBEROFCITIES];
    for(int i=0; i<NUMBEROFCITIES; i++) {
        CITIES[i] = new int[2];
        DispCITIES[i] = new double[2];
        Pheromones[i] = new double[NUMBEROFCITIES];
    }
    CITIES = city;

    CurrentRoute = new int[NUMBEROFCITIES];
    BestRoute = new int[NUMBEROFCITIES];
}

showTSP::~showTSP() {
    for(int i=0; i<NUMBEROFCITIES; i++) {
        delete [] CITIES[i];
        delete [] DispCITIES[i];
        delete [] Pheromones[i];
    }
    delete [] CITIES;
    delete [] DispCITIES;
    delete [] Pheromones;
    delete [] CurrentRoute;
    delete [] BestRoute;
}

void keyboard(unsigned char key, int x, int y)
{
    // esc,q,Qが入力されたら終了
    switch(key) {
        case 'q':
        case 'Q':
        case '\033':
            exit(0);
        default:
            break;
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void render_string(float x, float y, std::string const& str){
    float z = -1.0f;
    glRasterPos3f(x, y, z);
    for(int i=0; i<str.size(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(4);

    // Current Route
    glViewport(0 , height/2 , width/2 , height/2);
    glBegin(GL_POINTS);
    glColor3d(0.0, 0.0, 0.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[i][0], dc[i][1]);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3d(1.0, 0.0, 0.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[CurrentRoute[i]][0], dc[CurrentRoute[i]][1]);
    }
    glEnd();

    // Best Route
    glViewport(width/2 , height/2 , width/2 , height/2);
    glBegin(GL_POINTS);
    glColor3d(0.0, 0.0, 0.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[i][0], dc[i][1]);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3d(0.0, 0.0, 1.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[BestRoute[i]][0], dc[BestRoute[i]][1]);
    }
    glEnd();

    // Pheromones
    glViewport(0 , 0 , width/2 , height/2);
    glBegin(GL_POINTS);
    glColor3d(0.0, 0.0, 0.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[i][0], dc[i][1]);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    glColor3d(0.0, 1.0, 0.0);
    for(int i=0; i<n; i++) {
        glVertex2d(dc[i][0], dc[i][1]);
        for(int j=0; j<n; j++) {
            glVertex2d(dc[j][0], dc[j][1]);
        }
    }
    glEnd();

    // Texts
    glViewport(width/2 , 0 , width/2 , height/2);
    glColor3d(0.0, 0.0, 0.0);
    render_string(-0.9f, 0.0f, "Best Route : "+BestLength);

    // Line
    glViewport(0 , 0 , width , height);
    glBegin(GL_LINES);
    glColor3d(0.0, 0.0, 0.0);
    glVertex2d(0, 1);
    glVertex2d(0, -1);
    glVertex2d(1, 0);
    glVertex2d(-1, 0);
    glEnd();

    // 描画終了
    glFlush();
}

int showTSP::Highest() {
    int High = 0;
    for(int i=0; i<NUMBEROFCITIES; i++) {
        if(CITIES[i][0] > High) {
            High = CITIES[i][0];
        }
        if(CITIES[i][1] > High) {
            High = CITIES[i][1];
        }
    }
    return High;
}

void reshape(int w, int h) {
	width = w; height = h;
	display();
}

void showTSP::MakeDisplayPosition() {
    int h = Highest();
    for(int i=0; i<NUMBEROFCITIES; i++) {
        DispCITIES[i][0] = 1.8 * ((double)CITIES[i][0] / h) - 0.9;
        DispCITIES[i][1] = 1.8 * ((double)CITIES[i][1] / h) - 0.9;
    }
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(100, timer, 0);
}

void update() {
    glutPostRedisplay();
}

void showTSP::initGL(int argc, char** argv) {
    n = NUMBEROFCITIES;
    MakeDisplayPosition();
    dc = DispCITIES;

    glutInitWindowPosition(50, 50);
    glutInitWindowSize(500, 500);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow("Show TSP");

    glutKeyboardFunc(keyboard);
    init();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(update);

    //glutTimerFunc(100, timer, 0);
}

void showTSP::showRoute(int* route) {
    CurrentRoute = route;
}

void showTSP::showBest(int* route) {
    BestRoute = route;
    glutPostRedisplay();
}

void showTSP::showPheromones(double** pheromones) {
    Pheromones = pheromones;
}

void showTSP::showText(double length) {
    char buff[std::numeric_limits<double>::max_exponent10
          + 6   // fixed precision (printf's default)
          + 3]; // '-' + '.' + NULL
    std::sprintf(buff, "%.2f", length);
    BestLength = buff;
}

void showTSP::MainLoop() {
    glutMainLoop();
}