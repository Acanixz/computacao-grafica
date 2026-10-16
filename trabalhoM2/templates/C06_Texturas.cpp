#include <cstdlib>
#include <iostream>
#include <fstream>
#include "GL/glut.h"

using namespace std;


static unsigned int texture[2];
static unsigned char chessboard[64][64][3];
static float angle = 0.0;
static int id = 0;

struct BitMapFile
{
    int sizeX;
    int sizeY;
    unsigned char* data;
};


// Funciona somente com bmp de 24 bits
BitMapFile* getBMPData(string filename)
{
    BitMapFile* bmp = new BitMapFile;
    unsigned int size, offset, headerSize;

    // Ler o arquivo de entrada
    ifstream infile(filename.c_str(), ios::binary);

    // Pegar o ponto inicial de leitura
    infile.seekg(10);
    infile.read((char*)&offset, 4);

    // Pegar o tamanho do cabeзalho do bmp.
    infile.read((char*)&headerSize, 4);

    // Pegar a altura e largura da imagem no cabeзalho do bmp.
    infile.seekg(18);
    infile.read((char*)&bmp->sizeX, 4);
    infile.read((char*)&bmp->sizeY, 4);

    // Alocar o buffer para a imagem.
    size = bmp->sizeX * bmp->sizeY * 24;
    bmp->data = new unsigned char[size];

    // Ler a informaзгo da imagem.
    infile.seekg(offset);
    infile.read((char*)bmp->data, size);

    // Reverte a cor de bgr para rgb
    int temp;
    for (int i = 0; i < size; i += 3)
    {
        temp = bmp->data[i];
        bmp->data[i] = bmp->data[i + 2];
        bmp->data[i + 2] = temp;
    }

    return bmp;
}

void loadExternalTextures()
{
    BitMapFile* image[1];

    image[0] = getBMPData("Textures/launch.bmp");

    glBindTexture(GL_TEXTURE_2D, texture[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);
}

void loadProceduralTextures()
{
    glBindTexture(GL_TEXTURE_2D, texture[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, chessboard);
}

void createChessboard(void)
{
    int i, j;
    for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++)
            if ((((i / 8) % 2) && ((j / 8) % 2)) || (!((i / 8) % 2) && !((j / 8) % 2)))
            {
                chessboard[i][j][0] = 0x00;
                chessboard[i][j][1] = 0x00;
                chessboard[i][j][2] = 0x00;
            }
            else
            {
                chessboard[i][j][0] = 0xFF;
                chessboard[i][j][1] = 0xFF;
                chessboard[i][j][2] = 0xFF;
            }
}

void setup(void)
{
    glClearColor(0.8, 0.8, 0.8, 0.0);

    glGenTextures(2, texture);

    loadExternalTextures();
    createChessboard();
    loadProceduralTextures();

    glEnable(GL_TEXTURE_2D);

}


void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, texture[id]);

    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0); 
    glVertex3f(-10.0, -10.0, 0.0);
    glTexCoord2f(1.0, 0.0); 
    glVertex3f(10.0, -10.0, 0.0);
    glTexCoord2f(1.0, 1.0); 
    glVertex3f(10.0, 10.0, 0.0);
    glTexCoord2f(0.0, 1.0); 
    glVertex3f(-10.0, 10.0, 0.0);
    glEnd();

    glutSwapBuffers();
}

void resize(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case ' ':
        id++;
        if (id == 2) id = 0;
        glutPostRedisplay();
        break;
    case 127:
        angle = 0.0;
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

void specialKeyInput(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
    {
        angle -= 5.0;
        if (angle < 0.0) angle += 360.0;
    }
    if (key == GLUT_KEY_RIGHT)
    {
        angle += 5.0;
        if (angle > 360.0) angle -= 360.0;
    }
    glutPostRedisplay();
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Carregar Texturas");
    setup();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutSpecialFunc(specialKeyInput);
    glutMainLoop();

    return 0;
}