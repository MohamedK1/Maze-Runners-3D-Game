#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <mmsystem.h>
#include <fstream>
#include <math.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <irrKlang.h>
#include <sstream>

using namespace irrklang;
using namespace std;


// Vector class

class Vector3d {
public:
	double x, y, z;

	Vector3d(double _x = 0.0f, double _y = 0.0f, double _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3d operator+(Vector3d& v) {
		return Vector3d(x + v.x, y + v.y, z + v.z);
	}

	Vector3d operator-(Vector3d& v) {
		return Vector3d(x - v.x, y - v.y, z - v.z);
	}

	Vector3d operator*(double n) {
		return Vector3d(x * n, y * n, z * n);
	}

	Vector3d operator/(double n) {
		return Vector3d(x / n, y / n, z / n);
	}

	Vector3d unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3d cross(Vector3d v) {
		return Vector3d(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

// Global Variables
double windowWidth = 1000;
double windowHeight = 600;

double mazeWidth = 10;
double wallThickness = 0.8;
double wallHeight = 2;

Model_3DS model_character;
double characterX = mazeWidth * 0.8;
double characterZ = mazeWidth * 0.9;
double scaleFactor = 0.015;
double characterAngle = 0;
double yWidth = 55 * scaleFactor;
double xWidth = 30 * scaleFactor;
double zWidth = 30 * scaleFactor;
double step = 0.1;

int curDirection = 0;
char directions[] = { 'N', 'E', 'S', 'W' };
Vector3d directionVectors[] = { Vector3d(0,0,-1), Vector3d(1,0,0), Vector3d(0,0,1), Vector3d(-1,0,0) };

Model_3DS model_Tree;
Model_3DS model_Key;

vector<vector<double>> lower;
vector<vector<double>> upper;

vector<double> doorUpper;
vector<double> doorLower;

vector<bool> drawBall = { true, true, true, true };
vector<bool> drawBadBall = { false, false, false, false };
vector<vector<double>> ballLower;
vector<vector<double>> ballUpper;
double ballRadius = 0.35;
int numberOfBalls = 4;
int collectedBalls = 0;

bool changeColor = false;

int view = 0;
double fov = 60;
double eyeX = 17;
double eyeY = 15;
double eyeZ = 17;
double centreX = 5;
double centreY = 0;
double centreZ = 5;

int currentTime = 0;
int remainingTime = 120;
int level = 1;
int score = 0;

bool wonGame = false;
bool lostGame = false;

ISoundEngine* SoundEngine;

GLuint tex;
GLuint texWall;
GLuint GroundTex;
GLuint texDoor;
GLuint texSnowBall;
GLuint texFlame;
GLuint texWater;
GLuint texSnowGround;
GLuint texSnowWall;
GLuint texSnowDoor;
GLuint texMarble;

//power ups
vector<bool> drawPowerUp = { true, true, true, true };
vector<vector<double>> powerUpLower;
vector<vector<double>> powerUpUpper;
int numberOfPowerUp = 1;
Model_3DS model_PowerUp;


// Logic Methods
int random(int x, int y) {
	return rand() % (y - x + 1) + x;
}

void randomizeBadBalls() {
	if (level == 2) {
		SoundEngine->play2D("sounds/surprise.mp3", false);

		for (int i = 0; i < drawBadBall.size(); i++) {
			if (random(0, 1) == 1) {
				drawBadBall[i] = true;
			}
			else {
				drawBadBall[i] = false;
			}
		}
		glutPostRedisplay();
	}
}

bool inside(double upperX, double upperZ, double lowerX, double lowerZ) {
	double charUpperX = characterX;
	double charLowerX = characterX + xWidth;
	double charUpperZ = characterZ;
	double charLowerZ = characterZ + zWidth;


	double IntersectionUpperX = max(charUpperX, upperX);
	double IntersectionUpperZ = max(charUpperZ, upperZ);

	double IntersectionLowerX = min(charLowerX, lowerX);
	double IntersectionLowerZ = min(charLowerZ, lowerZ);
	return (IntersectionUpperX <= IntersectionLowerX && IntersectionUpperZ <= IntersectionLowerZ);
}

bool collideWithMaze() {
	for (int i = 0; i < upper.size(); i++) {
		if (inside(upper[i][0], upper[i][1], lower[i][0], lower[i][1])) {
			return true;
		}
	}
	return false;
}

void collideWithBall() {
	for (int i = 0; i < numberOfBalls; i++) {
		if (drawBall[i] && inside(ballUpper[i][0], ballUpper[i][1], ballLower[i][0], ballLower[i][1])) {
			if (drawBadBall[i]) {
				lostGame = true;
				SoundEngine->setAllSoundsPaused();
				SoundEngine->play2D("sounds/game_over.mp3", false);
			}
			else {
				drawBall[i] = false;
				collectedBalls++;
				score += 30;
				SoundEngine->play2D("sounds/good_ball.mp3", false);
			}
		}
	}
}


void reachedTarget() {
	if (!inside(doorUpper[0], doorUpper[1], doorLower[0], doorLower[1])) {
		return;
	}

	if (level == 1) {
		level = 2;
		SoundEngine->setAllSoundsPaused();
		SoundEngine->play2D("sounds/level_2_background.mp3", false);
		score += remainingTime / 2;

		currentTime = 0;
		remainingTime = 150;
		collectedBalls = 0;

		characterAngle = 0;
		curDirection = 0;
		characterX = mazeWidth * 0.8;
		characterZ = mazeWidth * 0.9;
		drawPowerUp = { true, true, true, true };
		drawBall = { true,true,true,true };
		drawBadBall = { false,false,false,false };
	}
	else if (level == 2) {
		wonGame = true;
		SoundEngine->setAllSoundsPaused();
		SoundEngine->play2D("sounds/cheers.ogg", false);
		level = 0;
		//score += remainingTime / 2;
	}
}


// Setting up the scene
void setupLights() {
	if (level == 1) {

		glEnable(GL_LIGHT0);
		double x = directionVectors[curDirection].x;
		double y = directionVectors[curDirection].y;
		double z = directionVectors[curDirection].z;
		double factor = -3;
		GLfloat l0Position[] = { characterX + xWidth / 2+factor*x, 0.4*yWidth + factor * y, characterZ + zWidth / 2 + factor * z,1 };
		GLfloat l0Direction[] = { x,y,z };
		
		glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 20);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 20);

		GLfloat l0model_ambient[] = { 0.03f, 0.03f, 0.03f, 0.3f };
		//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, l0model_ambient);

		GLfloat l0Ambient[] = { 0.005f, 0.005f, 0.005f, 0.1f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
	
		GLfloat l0Diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat l0Spec[] = { 0.5f, 0.5f, 0.5f, 1.0f };

		//source 2
		glEnable(GL_LIGHT1);

		//GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0000008f);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0000008f);
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.0000008f);


		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 128);

		GLfloat l1Position[] = { 5, 6, -1, 0.0f };
		GLfloat l1Direction[] = { 5, 0, 5 };
		
		glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);

			GLfloat l1Ambient[] = { 49.0/255, 32.0/255, 102.0/255, 1.0f };
			glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
		

		GLfloat l1Diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat l1Spec[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	//	glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
	//	glLightfv(GL_LIGHT1, GL_SPECULAR, l1Spec);

	}
	else if (level == 2) {

		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_COLOR_MATERIAL);
		//source 2
		glEnable(GL_LIGHT1);


		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 128);

		GLfloat l1Position[] = { 5, 6, -1, 0.0f };
		GLfloat l1Direction[] = { -5, 0,-5 };

		glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l1Direction);
		GLfloat l1Ambient[] = { 200.0 / 255, 200.0 / 255, 200.0 / 255, 1.0f };
		glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);


		GLfloat l1Diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat l1Spec[] = { 0.5f, 0.5f, 0.5f, 1.0f };

			glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
			glLightfv(GL_LIGHT1, GL_SPECULAR, l1Spec);

		glEnable(GL_LIGHT0);
		double x = directionVectors[curDirection].x;
		double y = directionVectors[curDirection].y;
		double z = directionVectors[curDirection].z;
		double factor = -3;
		GLfloat l0Position[] = { characterX + xWidth / 2 + factor * x, 0.4 * yWidth + factor * y, characterZ + zWidth / 2 + factor * z,1 };
		GLfloat l0Direction[] = { x,y,z };

		glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 10);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 60);



		GLfloat l0model_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, l0model_ambient);

		GLfloat l0Ambient[] = { 40.0 / 255, 40.0 / 255, 40.0 / 255, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);

		GLfloat l0Diffuse[] = { 19.0 / 97, 50.0 / 255, 252.0 / 255, 1.0f };
		GLfloat l0Spec[] = { 19.0 / 97, 50.0 / 255, 232.0 / 255, 1.0f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, l0Spec);
		//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1);
	}

}

void setupCamera() {

	if (view == 1) {
		fov = 80;
		eyeX = characterX + xWidth / 2;
		eyeY = yWidth + 0.5;
		eyeZ = characterZ + zWidth / 2;
		centreX = eyeX + directionVectors[curDirection].x;
		centreY = eyeY + directionVectors[curDirection].y;
		centreZ = eyeZ + directionVectors[curDirection].z;
	}
	else if (view == 3) {
		fov = 90;
		eyeX = characterX + xWidth / 2 - 0.5 * directionVectors[curDirection].x;
		eyeY = yWidth + 0.6 - 0.5 * directionVectors[curDirection].y;
		eyeZ = characterZ + zWidth / 2 - 0.5 * directionVectors[curDirection].z;
		centreX = eyeX + directionVectors[curDirection].x;
		centreY = eyeY + directionVectors[curDirection].y - 0.25;
		centreZ = eyeZ + directionVectors[curDirection].z;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, windowWidth / windowHeight, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		eyeX, eyeY, eyeZ,
		centreX, centreY, centreZ,
		0, 1, 0);
}

void loadAssets() {

	// Loading Model files
	model_character.Load("models/minion/Toy Minion N280319.3ds");
	model_Key.Load("models/key/Key_B_02.3ds");
	model_PowerUp.Load("models/star/star.3ds");
	// Loading texture files
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
	loadBMP(&texDoor, "Textures/door2.bmp", true);
	loadBMP(&GroundTex, "Textures/ground3.bmp", true);
	loadBMP(&texWall, "Textures/wall.bmp", true);
	loadBMP(&texSnowBall, "Textures/snowBall.bmp", true);
	loadBMP(&texWater, "Textures/water.bmp", true);
	loadBMP(&texFlame, "Textures/flame.bmp", true);
	loadBMP(&texSnowGround, "Textures/snow_ground.bmp", true);
	loadBMP(&texSnowWall, "Textures/snow_wall.bmp", true);
	loadBMP(&texSnowDoor, "Textures/snow_door.bmp", true);
	loadBMP(&texMarble, "Textures/stone.bmp", true);
	
}


// Drawing Methods
void print(int x, int y, char* string) {
	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}


void drawTexturedCube(double length, GLuint text) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, text);    // Background texture

   // We will specify texture coordinates
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glPushMatrix();
	glScaled(length / 2, length / 2, length / 2);
	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	// Back Face
	glNormal3f(0, 0, -1);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	// Top Face
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	// Bottom Face
	glNormal3f(0, -1, 0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	// Right face
	glNormal3f(1, 0, 0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	// Left Face
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();

	glPopMatrix();

}

double repeat = 5;
void drawVerticalWall(double x, double z, double length) {
	for (int i = 0; i < repeat; i++) {
		glPushMatrix();
		//glColor3f(12 / 255.0, 250 / 255.0, 40 / 255.0);
		glTranslated(x, 0, z+i*length/repeat);
		glScaled(wallThickness, wallHeight, length/repeat);
		glTranslated(0.5, 0.5, 0.5);
		if (level == 1) {
			drawTexturedCube(1, texWall);
		}
		else if (level == 2) {
			drawTexturedCube(1, texSnowWall);
		}
		//glutSolidCube(1);
		glPopMatrix();
	}

	upper.push_back({ x,z });
	lower.push_back({ x + wallThickness,z + length });

}

void drawHorizontalWall(double x, double z, double length) {
	for (int i = 0; i < repeat; i++) {
		glPushMatrix();
		//glColor3f(12 / 255.0, 250 / 255.0, 40 / 255.0);
		glTranslated(x+i*length/repeat, 0, z);
		glScaled(length/repeat, wallHeight, wallThickness);
		glTranslated(0.5, 0.5, 0.5);

		if (level == 1) {
			drawTexturedCube(1, texWall);
		}
		else if (level == 2) {
			drawTexturedCube(1, texSnowWall);
		}
		//glutSolidCube(1);
		glPopMatrix();
	}

	upper.push_back({ x,z });
	lower.push_back({ x + length,z + wallThickness });
}

void drawGround() {
	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glPushMatrix();
	glTranslated(mazeWidth / 2, -0.04, mazeWidth / 2);
	glScaled(1.0, 0.02, 1.0);
	if (level == 1) {
		drawTexturedCube(mazeWidth, GroundTex);
	}
	else if (level == 2) {
		drawTexturedCube(mazeWidth, texSnowGround);
	}
	glPopMatrix();
}

void drawDoor(double x, double z, double length) {

	glPushMatrix();
	glColor3f(0.7, 0.7, 0.7);
	//glColor3f(168 / 255.0, 89 / 255.0, 3 / 255.0);
	glTranslated(x, 0, z);
	glScaled(length, wallHeight, wallThickness);
	glTranslated(0.5, 0.5, 0.5);

	if (level == 1) {
		drawTexturedCube(1, texDoor);
	}
	else if (level == 2) {
		drawTexturedCube(1, texSnowDoor);
	}
	//glutSolidCube(1);
	glPopMatrix();

	doorUpper = { x,z };
	doorLower = { x + length,z + wallThickness };
}

void drawMaze() {

	glColor3f(0.6, 0.6, 0.6);
	upper.clear();
	lower.clear();

	if (level == 1) {
		//A
		drawVerticalWall(mazeWidth / 3 - wallThickness / 2, 0.6 * mazeWidth, 0.3 * mazeWidth);
		//B
		drawHorizontalWall(mazeWidth - (wallThickness * 5), 2, 0.4 * mazeWidth);

		glPushMatrix();
		//C
		drawHorizontalWall(0.7 * mazeWidth, 2 * mazeWidth / 3 - wallThickness, 0.3 * mazeWidth);

		//D
		drawVerticalWall(0.6 * mazeWidth - wallThickness, 0.2 * mazeWidth, 0.4 * mazeWidth);
		glPopMatrix();

		//E
		drawHorizontalWall(0, 0.2 * mazeWidth, 0.4 * mazeWidth);

	}
	if (level == 2) {
		//A
		drawVerticalWall(mazeWidth / 2 - wallThickness / 2, 0.8 * mazeWidth, 0.2 * mazeWidth);
		//B
		drawVerticalWall(mazeWidth / 2 - wallThickness / 2, 0, 0.4 * mazeWidth);



		glPushMatrix();
		//glTranslated(0, 0, -0.6);

		//C
		drawHorizontalWall(0, 2 * mazeWidth / 3 - wallThickness * 0.5, 0.25 * mazeWidth);

		//D
		drawHorizontalWall(0.4 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5, 0.6 * mazeWidth);

		//E
		drawVerticalWall(0.4 * mazeWidth - wallThickness, 0.5 * mazeWidth, 0.3 * mazeWidth);

		glPopMatrix();


		//F
		drawHorizontalWall(0, 0.25 * mazeWidth, 0.35 * mazeWidth);

		//G
		drawHorizontalWall(mazeWidth / 2 + wallThickness / 2 + 0.1 * mazeWidth, 0.25 * mazeWidth, 0.25 * mazeWidth);

	}

}

void drawMazeBorders() {
	// upper horizontal Border that contains the door
	drawHorizontalWall(0, -wallThickness, mazeWidth);

	drawHorizontalWall(0, mazeWidth, mazeWidth);
	drawVerticalWall(-wallThickness, 0, mazeWidth);
	drawVerticalWall(mazeWidth, 0, mazeWidth);

	drawDoor(0.1 * mazeWidth, -wallThickness + 0.04, 0.1 * mazeWidth);

}

void drawABall(int idx, double upperX, double upperZ, double r, GLuint text) {
	if (drawBall[idx]) {
		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(upperX, 0, upperZ);
		glTranslated(r, 1.85 * r, r);
		glBindTexture(GL_TEXTURE_2D, text);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, r, 100, 100);
		gluDeleteQuadric(qobj);


		glPopMatrix();
	}
	ballUpper.push_back({ upperX, upperZ });
	ballLower.push_back({ upperX + 2 * r, upperZ + 2 * r });
}

float keyScaling = 0.1;
void drawAKey(int idx, double upperX, double upperZ) {
	if (drawBall[idx]) {
		glPushMatrix();
		glTranslated(upperX, 0, upperZ);
		glTranslated(0, 1, 0);
		glScaled(keyScaling,keyScaling, keyScaling);
		glRotated(90, 0, 0, 1);
		model_Key.Draw();

		glPopMatrix();
	}
	ballUpper.push_back({ upperX, upperZ });
	ballLower.push_back({ upperX + 1.5*keyScaling , upperZ + 1.5 * keyScaling });
}
void drawBalls() {
	ballUpper.clear();
	ballLower.clear();
	if (level == 1) {
		int idx = 0;
		// first ball between D,G
		drawAKey(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.15 * mazeWidth);
		idx++;
		drawAKey(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.55 * mazeWidth);
		idx++;
		drawAKey(idx, 0.4 * mazeWidth + 0.2 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth);
		idx++;
		drawAKey(idx, 0.4 * mazeWidth - 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth);
	}
	else {
		int idx = 0;
		// first ball between D,G
		drawABall(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.15 * mazeWidth, ballRadius, level == 1 ? texMarble : drawBadBall[idx] ? texFlame : texWater);
		idx++;
		drawABall(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.55 * mazeWidth, ballRadius, level == 1 ? texMarble : drawBadBall[idx] ? texFlame : texWater);
		idx++;
		drawABall(idx, 0.4 * mazeWidth + 0.2 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth, ballRadius, level == 1 ? texMarble : drawBadBall[idx] ? texFlame : texWater);
		idx++;
		drawABall(idx, 0.4 * mazeWidth - 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth, ballRadius, level == 1 ? texMarble : drawBadBall[idx] ? texFlame : texWater);
	}
}

void collideWithPowerUp() {
	for (int i = 0; i < numberOfPowerUp; i++) {
		if (drawPowerUp[i] && inside(powerUpUpper[i][0], powerUpUpper[i][1], powerUpLower[i][0], powerUpLower[i][1])) {
				drawPowerUp[i] = false;
				// do the effect of power up
				remainingTime += 20;
				SoundEngine->play2D("sounds/powerUp.mp3", false);
		}
	}
}

void drawAPowerUp(int idx, double upperX, double upperZ) {
	if (drawPowerUp[idx]) {
		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(upperX, 0, upperZ);
		glTranslated(0.5,0.7, 0.5);
		glRotated(20, 1, 0, 0);
		glRotated(90, 0, 0, 1);
		glScaled(0.3, 0.3, 0.3);
		model_PowerUp.Draw();
		//glutSolidCube(1);

		glPopMatrix();
	}
	powerUpUpper.push_back({ upperX, upperZ });
	powerUpLower.push_back({ upperX + 1, upperZ + 1 });
}
void drawPowerUps() {

	powerUpUpper.clear();
	powerUpLower.clear();
	if (level == 1) {
		int idx = 0;
		// first ball between D,G
	/*	drawAPowerUp(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.15 * mazeWidth);
		idx++;
		drawAPowerUp(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.55 * mazeWidth);
		idx++;
		drawAPowerUp(idx, 0.4 * mazeWidth + 0.2 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth);
		idx++;*/
		drawAPowerUp(idx, 0.4 * mazeWidth - 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.17 * mazeWidth);
	}
	else {
		int idx = 0;
		drawAPowerUp(idx, 0.4 * mazeWidth - 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.17 * mazeWidth);
		// first ball between D,G
	/*	drawAPowerUp(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.15 * mazeWidth);
		idx++;
		drawAPowerUp(idx, 0.4 * mazeWidth + 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 - 0.55 * mazeWidth);
		idx++;
		drawAPowerUp(idx, 0.4 * mazeWidth + 0.2 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth);
		idx++;
		drawAPowerUp(idx, 0.4 * mazeWidth - 0.3 * mazeWidth, 2 * mazeWidth / 3 - wallThickness * 0.5 + 0.15 * mazeWidth);*/
	}
}

void drawSky() {
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(25, 0, 0);
	if (level == 1) {
		glColor3f(0, 0, 0);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex);

	}
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 70, 80, 80);
	gluDeleteQuadric(qobj);
	glPopMatrix();
}

void drawMoon() {
	glPushMatrix();
	//glScaled(1, 1, 0.2);
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(5, 6, -1);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, texSnowBall);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 2, 100, 100);
	gluDeleteQuadric(qobj);

	glPopMatrix();
}


void drawCharacter() {
	glPushMatrix();
	glTranslated(characterX, 0, characterZ);
	glTranslated(xWidth / 2, yWidth / 2, zWidth / 2);// now the character upper left point is 0,0
	glScaled(scaleFactor, scaleFactor, scaleFactor);
	glRotated(characterAngle, 0, 1, 0);
	glRotated(180, 0, 1, 0);
	model_character.Draw();
	glPopMatrix();
}






void display() {

	// Update window dimensions
	windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Display winning screen
	if (wonGame) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

		glDisable(GL_LIGHTING);
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glColor3d(0.0, 1.0, 0.0);
		char* message[20];
		sprintf((char*)message, "YOU WON!!!  Score: %d", score);
		print(windowWidth / 2 - windowWidth * 0.125, windowHeight / 2, (char*)message);

		glFlush();
		glutSwapBuffers();
		return;
	}

	// Display losing screen
	if (lostGame) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

		glDisable(GL_LIGHTING);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glColor3d(1.0, 0.0, 0.0);
		char* message[20];
		sprintf((char*)message, "GAME OVER");
		print(windowWidth / 2 - windowWidth * 0.07, windowHeight / 2, (char*)message);

		glFlush();
		glutSwapBuffers();
		return;
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen
	glMatrixMode(GL_PROJECTION); // Tell opengl that we are doing project matrix work
	glLoadIdentity(); // Clear the matrix
	gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
//	glOrtho(-9.0, 9.0, -9.0, 9.0, 0.0, 30.0); // Setup an Ortho view
	glMatrixMode(GL_MODELVIEW); // Tell opengl that we are doing model matrix work. (drawing)
	glLoadIdentity(); // Clear the model matrix

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glColor3d(1.0, 0.0, 0.0);
	char* message[20];
	sprintf((char*)message, " Score: %d", score);
	print(0.05*windowWidth, 0.95*windowHeight, (char*)message);
	sprintf((char*)message, " time: %d", remainingTime);
	print(0.85 * windowWidth, 0.95 * windowHeight, (char*)message);


	glDepthMask(GL_FALSE);
	
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/

	//LoadAssets();
	//resetPerspectiveProjection();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glDepthMask(GL_TRUE);



	


	




	setupCamera();
	setupLights();
	
	drawGround();
	drawMaze();
	drawMazeBorders();
	drawCharacter();
	drawBalls();
	drawPowerUps();
	if (level == 1) {
		drawMoon();
		drawSky();

	}
	else if (level == 2) {
		drawSky();
	}


	glFlush();
	glutSwapBuffers();
}


// Moving and changing the direction of the character
void moveBack(bool skip);

void swap(double* x, double* z) {
	double tmp = *x;
	*x = *z;
	*z = tmp;
}

void moveFront(bool skip = true) {

	if (directions[curDirection] == 'N')
		characterZ -= step;
	if (directions[curDirection] == 'S')
		characterZ += step;
	if (directions[curDirection] == 'E')
		characterX += step;
	if (directions[curDirection] == 'W')
		characterX -= step;

	if (skip && collideWithMaze()) {
		//SoundEngine->play2D("sounds/wall_hit.mp3", false);
		moveBack(false);
	}

	if (skip) {
		reachedTarget();
		collideWithBall();
		collideWithPowerUp();
	}
}

void moveBack(bool skip = true) {

	if (directions[curDirection] == 'N')
		characterZ += step;
	if (directions[curDirection] == 'S')
		characterZ -= step;
	if (directions[curDirection] == 'E')
		characterX -= step;
	if (directions[curDirection] == 'W')
		characterX += step;

	if (skip && collideWithMaze()) {
		//SoundEngine->play2D("sounds/wall_hit.mp3", false);
		moveFront(false);
	}

	if (skip) {
		reachedTarget();
		collideWithBall();
		collideWithPowerUp();
	}
}

void moveRight() {

	curDirection += 1;
	curDirection %= 4;
	characterAngle -= 90;
	swap(&xWidth, &zWidth);
	moveFront();
}

void moveLeft() {

	curDirection = ((curDirection - 1) % 4 + 4) % 4;
	characterAngle += 90;
	swap(&xWidth, &zWidth);
	moveFront();
}


// Input Methods
void mouse(int button, int state, int x, int y) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			SoundEngine->play2D("sounds/turn.mp3", false);
			moveLeft();
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			SoundEngine->play2D("sounds/turn.mp3", false);
			moveRight();
		}
		break;
	}

	glutPostRedisplay();
}

void key(unsigned char key, int x, int y) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	double d = 1;

	switch (key) {
		// Change the camera position (different views)

		// 1st person view
	case '1':
		view = 1;
		break;
		// 3rd person view
	case '3':
		view = 3;
		break;
		// Top view
	case 't':
		view = 2;
		fov = 60;
		eyeX = mazeWidth / 2;
		eyeY = 2 * mazeWidth;
		eyeZ = mazeWidth / 2;
		centreX = mazeWidth / 2;
		centreY = 0;
		centreZ = 0.49 * mazeWidth;
		break;
		// Axonometric view
	case 'a':
		view = 0;
		fov = 60;
		eyeX = 17;
		eyeY = 15;
		eyeZ = 17;
		centreX = 5;
		centreY = 0;
		centreZ = 5;
		break;

		// for testing
	case '4':
		eyeX += d;
		break;
	case '5':
		eyeX -= d;
		break;
	case '6':
		eyeY += d;
		break;
	case '7':
		eyeY -= d;
		break;
	case '8':
		eyeZ += d;
		break;
	case '9':
		eyeZ -= d;
		break;

		// Exit the game and close the window
	case 27:
		exit(EXIT_SUCCESS);
	}

	glutPostRedisplay();
}

void keyUp(unsigned char key, int x, int y) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	glutPostRedisplay();
}

void spKey(int key, int x, int y) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	switch (key) {
	case GLUT_KEY_UP:
		moveFront();
		break;
	case GLUT_KEY_DOWN:
		moveBack();
		break;
	case GLUT_KEY_RIGHT:
		SoundEngine->play2D("sounds/turn.mp3", false);
		moveRight();
		break;
	case GLUT_KEY_LEFT:
		SoundEngine->play2D("sounds/turn.mp3", false);
		moveLeft();
		break;
	}

	glutPostRedisplay();
}

void spKeyUp(int key, int x, int y) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	switch (key) {
	case GLUT_KEY_UP:
		SoundEngine->play2D("sounds/footsteps.mp3", false);
		moveFront();
		break;
	case GLUT_KEY_DOWN:
		SoundEngine->play2D("sounds/footsteps.mp3", false);
		moveBack();
		break;
	}
	glutPostRedisplay();
}


// Timer Methods
void timer(int value) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	currentTime++;
	remainingTime--;

	if (level == 1 && remainingTime == 0) {
		lostGame = true;
		SoundEngine->setAllSoundsPaused();
		SoundEngine->play2D("sounds/game_over.mp3", false);
	}
	else if (level == 2 && remainingTime == 0) {
		lostGame = true;
		SoundEngine->setAllSoundsPaused();
		SoundEngine->play2D("sounds/game_over.mp3", false);
	}
	else if (level == 2 && currentTime % 10 == 0) {
		changeColor = !changeColor;
	}

	// Redisplay and set up the next call
	glutPostRedisplay();
	glutTimerFunc(1000, timer, 0);
}

void timer2(int value) {
	if (wonGame || lostGame) {
		glutPostRedisplay();
		return;
	}

	// In second level check which balls are obstacles
	randomizeBadBalls();
	drawBalls();

	// Redisplay and set up the next call
	glutPostRedisplay();
	glutTimerFunc(random(1, 10) * 1000, timer2, 0);
}



// Main Method
void main(int argc, char** argv) {

	// Seed the random function
	srand(time(0));

	// Initializing OpenGL
	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Maze");

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutKeyboardFunc(key);
	glutKeyboardUpFunc(keyUp);
	glutSpecialFunc(spKey);
	glutSpecialUpFunc(spKeyUp);
	glutTimerFunc(1000, timer, 0);
	glutTimerFunc(1000, timer2, 0);

	// Initializing the scene
	glClearColor(0.0, 0.0, 0.0, 0.0);
	setupLights();
	setupCamera();
	loadAssets();

	// Initializing sound library
	SoundEngine = createIrrKlangDevice();
	SoundEngine->play2D("sounds/level_1_background.mp3", false);

	// Start rendering
	glutMainLoop();
}