#include "GL\glew.h"
#include "GL\freeglut.h"
#include<iostream>

#include "shaderLoader.h"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>

#include <vector>

//ustawienie parametrow wykresu

#define MAX_X 6.0                  //zakresy x i y
#define MIN_X -MAX_X
#define MAX_Y 6.0
#define MIN_Y -MAX_Y
#define ROZ 700                    //rozdzielczosc wykresu
#define ROZ_S 10               //rozdzielczosc siatki


int screen_width = 640;
int screen_height = 480;

int pozycjaMyszyX;
int pozycjaMyszyY;
int mbutton;

double kameraX = 230;
double kameraZ = 270;
double kameraD = -10;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

float min_z = 1000;
float max_z = -1000;

glm::mat4 MV;
glm::mat4 P;

std::vector<float> wierzcholki;
std::vector<float> wierzcholki_XYZ;
std::vector<unsigned int> indeksy;
std::vector<unsigned int> indeksy_XYZ;

GLuint programID1 = 0, programID2 = 0;

unsigned int VBO, VBO2, VBO_XYZ;
unsigned int EBO, EBO2, EBO_XYZ;
unsigned int VAO[3];

double dx = -5;

typedef float (*funkcja_wsk)(float, float);
funkcja_wsk wybrana_funkcja;


float wierz_xyz[] = {                   //wierzcholki dla opisow osi xyz
    -0.5f, MAX_Y + 1.0f, min_z,                 //X
    0.5f, MAX_Y + 2.0f, min_z,
    -0.5f, MAX_Y + 2.0f, min_z,
    0.5f, MAX_Y + 1.0f, min_z,

    MAX_X + 1.0f, -0.5f, min_z,                //Y
    MAX_X + 1.5f, 0.0f, min_z,
    MAX_X + 1.0f, 0.5f, min_z,
    MAX_X + 2.3f, 0.0f, min_z,

    -MAX_X, MAX_Y + 1.0f, 0.0f,                //Z
    -MAX_X, MAX_Y + 2.0f, 0.0f,
    -MAX_X, MAX_Y + 1.0f, -1.0f,
    -MAX_X, MAX_Y + 2.0f, -1.0f,

};

unsigned int index_xyz[] = {            //indeksy dla opisow osi XYZ
    0,1,2,3,                            //X
    4,5,6,5,5,7,                        //Y
    8,9,9,10,10,11                      //Z
};

float funkcja_1(float x, float y) {
    return -exp(-(1/(x*x + y*y))/0.25);
}

float funkcja_2(float x, float y) {
    return sin(x)*cos(y)+0.04*y;
}

float funkcja_3(float x, float y) {
    return sin(pow(x + y, 2)/50)*10;
}

float funkcja_4(float x, float y) {
    return 1/(1+exp(0.5*x + pow(sin(y*2), 3)));
}

float funkcja_5(float x, float y) {
    return abs(sinh(cos(x*y)));
}

float funkcja_6(float x, float y) {
    return exp(-abs(sin(x) + cos(y)));
}


void mysz(int button, int state, int x, int y)
{
    mbutton = button;
    switch (state)
    {
    case GLUT_UP:
        break;
    case GLUT_DOWN:
        pozycjaMyszyX = x;
        pozycjaMyszyY = y;
        poprzednie_kameraX = kameraX;
        poprzednie_kameraZ = kameraZ;
        poprzednie_kameraD = kameraD;
        break;

    }
}

void mysz_ruch(int x, int y)
{
    if (mbutton == GLUT_LEFT_BUTTON)
    {
        kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
        kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
    }
    if (mbutton == GLUT_RIGHT_BUTTON)
    {
        kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
    }
}


void rozmiar(int width, int height)
{
    screen_width = width;
    screen_height = height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screen_width, screen_height);
    P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);
    glutPostRedisplay();
}

void rysuj()
{

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);               //zmiana koloru tła
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programID1);

    GLuint min_z_ID = glGetUniformLocation(programID1, "min_z");
    GLuint max_z_ID = glGetUniformLocation(programID1, "max_z");
    GLuint MVP_ID = glGetUniformLocation(programID1, "MVP");
    MV = glm::mat4(1.0);
    MV = glm::translate(MV, glm::vec3(1, -1, kameraD + dx));
    MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
    MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 0, 1));

    glUniform1f(min_z_ID, min_z);
    glUniform1f(max_z_ID, max_z);
    glm::mat4 MVP = P * MV;
    glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP[0][0]);
   
    glBindVertexArray(VAO[0]);
    glDrawElements(GL_TRIANGLES, indeksy.size(), GL_UNSIGNED_INT, 0);

    glUseProgram(programID2);

    glUniformMatrix4fv(MVP_ID, 1, GL_FALSE, &MVP[0][0]);

    glBindVertexArray(VAO[1]);
    glDrawElements(GL_LINES, indeksy_XYZ.size(), GL_UNSIGNED_INT, 0);


    glBindVertexArray(VAO[2]);
    glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

void klawisz(GLubyte key, int x, int y)
{
    switch (key) {
    case 27:
        exit(1);
    }
}

void generuj_wierz_indeksy_funk(int rozdz, funkcja_wsk wybrana_funkcja)
{

    float krokx = (MAX_X - MIN_X) / rozdz;
    float kroky = (MAX_Y - MIN_Y) / rozdz;
    float x = MIN_X;
    float y = MIN_Y;
    float z;
    float wsp_x;
    float wsp_y;
    float wsp_z;
    unsigned int obecny;
    unsigned int nastepny;


    for (int i = 0; i < rozdz; i++) {               //petla generujaca siatke wierzcholkow funkcji
        for (int j = 0; j < rozdz; j++) {
            z = wybrana_funkcja(x, y);
            min_z = std::min(min_z, z);
            max_z = std::max(max_z, z);
            wierzcholki.push_back(x);
            wierzcholki.push_back(y);
            wierzcholki.push_back(z);
            y += kroky;
        }
        x += krokx;
        y = MIN_Y;
    }

    wierz_xyz[2] = min_z;                           //aktualizacja wartosci min_z i max_z w tablicy wierz_xyz
    wierz_xyz[5] = min_z;
    wierz_xyz[8] = min_z;
    wierz_xyz[11] = min_z;

    wierz_xyz[14] = min_z;                           
    wierz_xyz[17] = min_z;
    wierz_xyz[20] = min_z;
    wierz_xyz[23] = min_z;

                                                                                            //modyfikacja wartosci Z dla opisu osi Z w celu poprawnego wyswietlania (mniej wiecej po srdoku osi) dla roz 700
    if ((min_z >= -1.0 && min_z <= -0.9) && (max_z <= 0.1 && max_z >= -0.1)) {              //funkcja 1
        wierz_xyz[26] = 0.0f;
        wierz_xyz[29] = 0.0f;
        wierz_xyz[32] = -1.0f;
        wierz_xyz[35] = -1.0f;
    }

    if ((min_z >= -1.3 && min_z <= -1.2) && (max_z <= 1.2 && max_z >= 1.1)) {              //funkcja 2
        wierz_xyz[26] = 0.6f;
        wierz_xyz[29] = 0.6f;
        wierz_xyz[32] = -0.6f;
        wierz_xyz[35] = -0.6f;
    }

    if ((min_z >= 0 && min_z <= 0.1) && (max_z <= 10.1 && max_z >= 9.9)) {       //funkcja 3
        wierz_xyz[26] = 5.5f;
        wierz_xyz[29] = 5.5f;
        wierz_xyz[32] = 4.5f;
        wierz_xyz[35] = 4.5f;
    }

    if ((min_z >= 0 && min_z <= 0.1) && (max_z <= 1.0 && max_z >= 0.9)) {       //funkcja 4
        wierz_xyz[26] = 1.0f;
        wierz_xyz[29] = 1.0f;
        wierz_xyz[32] = 0.0f;
        wierz_xyz[35] = 0.0f;
    }

    if ((min_z >= 0 && min_z <= 0.1) && (max_z <= 1.2 && max_z >= 1.1)) {       //funkcja 5
        wierz_xyz[26] = 1.1f;
        wierz_xyz[29] = 1.1f;
        wierz_xyz[32] = 0.1f;
        wierz_xyz[35] = 0.1f;
    }

    if ((min_z >= 0.13 && min_z <= 0.14) && (max_z <= 1.0 && max_z >= 0.9)) {       //funkcja 6
        wierz_xyz[26] = 1.0f;
        wierz_xyz[29] = 1.0f;
        wierz_xyz[32] = 0.0f;
        wierz_xyz[35] = 0.0f;
    }


    //std::cout << min_z << std::endl;
    //std::cout << max_z << std::endl;

    for (int i = 0; i < rozdz - 1; i++) {           //petla generujca indeksy dla wierzochkow funkcji
        for (int j = 0; j < rozdz - 1; j++) {
            obecny = i * rozdz + j;
            nastepny = obecny + rozdz;
            indeksy.push_back(obecny);
            indeksy.push_back(nastepny);
            indeksy.push_back(obecny + 1);
            indeksy.push_back(obecny + 1);
            indeksy.push_back(nastepny);
            indeksy.push_back(nastepny + 1);
        }
    }


    for (int i = 0; i <= ROZ_S; i++) {          //petla generujca wierzochlki siatki XY
        wsp_x = MIN_X + i * (MAX_X - MIN_X) / ROZ_S;
        wierzcholki_XYZ.push_back(wsp_x);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(min_z);
        wierzcholki_XYZ.push_back(wsp_x);
        wierzcholki_XYZ.push_back(MAX_Y);
        wierzcholki_XYZ.push_back(min_z);
        wsp_y = MIN_Y + i * (MAX_Y - MIN_Y) / ROZ_S;
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(wsp_y);
        wierzcholki_XYZ.push_back(min_z);
        wierzcholki_XYZ.push_back(MAX_X);
        wierzcholki_XYZ.push_back(wsp_y);
        wierzcholki_XYZ.push_back(min_z);
    }

    for (int i = 0; i <= ROZ_S; i++) {           //petla generujca wierzochlki siatki XZ
        wsp_x = MIN_X + i * (MAX_X - MIN_X) / ROZ_S;
        wierzcholki_XYZ.push_back(wsp_x);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(min_z);
        wierzcholki_XYZ.push_back(wsp_x);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(max_z);
        wsp_z = min_z + i * (max_z - min_z) / ROZ_S;
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(wsp_z);
        wierzcholki_XYZ.push_back(MAX_X);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(wsp_z);
    }

    for (int i = 0; i <= ROZ_S; i++) {           //petla generujca wierzochlki siatki YZ
        wsp_y = MIN_Y + i * (MAX_Y - MIN_Y) / ROZ_S;
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(wsp_y);
        wierzcholki_XYZ.push_back(min_z);
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(wsp_y);
        wierzcholki_XYZ.push_back(max_z);
        wsp_z = min_z + i * (max_z - min_z) / ROZ_S;
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(MIN_Y);
        wierzcholki_XYZ.push_back(wsp_z);
        wierzcholki_XYZ.push_back(MIN_X);
        wierzcholki_XYZ.push_back(MAX_Y);
        wierzcholki_XYZ.push_back(wsp_z);
    }

    for (int i = 0; i <= ROZ_S * 3 + 2; i++) {      //petla generujca indesky dla wierzcholkow siatki XYZ
        obecny = i * 4;
        indeksy_XYZ.push_back(obecny);
        indeksy_XYZ.push_back(obecny + 1);
        indeksy_XYZ.push_back(obecny + 2);
        indeksy_XYZ.push_back(obecny + 3);
    }
}

void idle()
{
    glutPostRedisplay();
}

/*###############################################################*/

void timer(int value) {

    /////////////////////////
    glutTimerFunc(20, timer, 0);
}

funkcja_wsk wybor_funkcji() {
    char key;
    std::cout << "----Wybierz funkcje:----\n";
    std::cout << "\n";
    std::cout << "1: -exp(-(1/(x*x + y*y))/0.25)\n";
    std::cout << "2: sin(x)*cos(y)+0.04*y\n";
    std::cout << "3: sin(pow(x + y, 2)/50)*10\n";
    std::cout << "4: 1/(1+exp(0.5*x + pow(sin(y*2), 3)))\n";
    std::cout << "5: abs(sinh(cos(x*y)))\n";
    std::cout << "6: exp(-abs(sin(x) + cos(y)))\n";
    std::cout << "\n";
    std::cout << "----Kliknij e i enter aby wyjsc----\n";
    std::cin >> key;

    switch (key) {
        case ('1'):
            return wybrana_funkcja = funkcja_1;
        case ('2'):
            return wybrana_funkcja = funkcja_2;
        case ('3'):
            return wybrana_funkcja = funkcja_3;
        case ('4'):
            return wybrana_funkcja = funkcja_4;
        case ('5'):
            return wybrana_funkcja = funkcja_5;
        case ('6'):
            return wybrana_funkcja = funkcja_6;
        case ('e'):
            exit(1);
    }
}


int main(int argc, char** argv)
{
    wybor_funkcji();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("Wykresy 3D");

    glewInit();

    generuj_wierz_indeksy_funk(ROZ, wybrana_funkcja);

    glutDisplayFunc(rysuj);
    glutIdleFunc(idle);
    glutTimerFunc(20, timer, 0);
    glutReshapeFunc(rozmiar);

    glutKeyboardFunc(klawisz);
    glutMouseFunc(mysz);
    glutMotionFunc(mysz_ruch);

    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(3, VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * wierzcholki.size(), wierzcholki.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * wierzcholki_XYZ.size(), wierzcholki_XYZ.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &VBO_XYZ);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_XYZ);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 36, wierz_xyz, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indeksy.size(), indeksy.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indeksy_XYZ.size(), indeksy_XYZ.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO_XYZ);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_XYZ);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 16, index_xyz, GL_STATIC_DRAW);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_XYZ);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_XYZ);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


    programID1 = loadShaders("vertex_shader.glsl", "fragment_shader_1.glsl");
    programID2 = loadShaders("vertex_shader.glsl", "fragment_shader_2.glsl");

    glutMainLoop();

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &VBO_XYZ);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &EBO2);
    glDeleteBuffers(1, &EBO_XYZ);
    glDeleteBuffers(3, VAO);

    return 0;
}