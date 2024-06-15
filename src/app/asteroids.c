/*
Noah Attwood
Asteroids
Summer 2018
GitHub: attwoodn/c-asteroids

--------------------------------------------------------------------------------

CustomOS
Copyright (C) 2024 D.Salzner <mail@dennissalzner.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * @file myglut.h
 * @brief Compatibility layer for OpenGL Asteroids to run on framebuffers X11 and CustomOS
 *
*/

#pragma once

static void myKey(unsigned char key, int x, int y);
static void myReshape(int w, int h);
static int started;
static void myDisplay();
static void myMenuTimer(int value);
static void myTimer(int value);
static void keyPress(int key, int x, int y);
static void keyRelease(int key, int x, int y);

const char * name() {
  return "Asteroids";
}

#ifdef __GNUC__ // gcc, assuming Linux

#include "linux-graphics.h"
#include "linux-input.h"

int keyDelay = 0;

void keyboardDown(char key, uint8_t scancode) {
  if (key == 113) keyPress(100, 0, 0); // -- left
  if (key == 111) keyPress(101, 0, 0); // -- up
  if (key == 116) keyPress(103, 0, 0); // -- right
  if (key == 114) keyPress(102, 0, 0); // -- down
  if (key == 65)  myKey(' ', 0, 0);    // -- space
  keyDelay = 0;
}

void keyboardUp(char key, uint8_t scancode) {
  if (key == 113) keyRelease(100, 0, 0); // -- left
  if (key == 111) keyRelease(101, 0, 0); // -- up
  if (key == 116) keyRelease(103, 0, 0); // -- right
  if (key == 114) keyRelease(102, 0, 0); // -- down
}
void update() {
  myDisplay();
  graphicsLoop();
  inputLoop();

  if (m_input.keyboardDown == 1 || keyDelay > 100) {
    keyboardDown(m_input.keyboardKeycode, 0);
  } else {
    keyboardUp(m_input.keyboardKeycode, 0);
  }
  keyDelay++;
}

void init() {
  graphicsInit();
  inputInit();
}

#else // other, assuming TinyCC in CustomOS

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

void* (*graphicsSetPixel)(uint16_t, uint16_t, uint8_t);
void* (*graphicsFlush)(void);
void* (*exit)(void);

void setGraphicsSetPixelCallback(void* (*graphicsSetPixel_)(uint16_t, uint16_t, uint8_t)) {
  graphicsSetPixel = graphicsSetPixel_;
}

void setGraphicsFlushCallback(void* (*graphicsFlush_)(void)) {
  graphicsFlush = graphicsFlush_;
}

void setExitCallback(void* (*exit_)(void)) {
  exit = exit_;
}
// --

void keyboardDown(char key, uint8_t scancode) {
  if(scancode == 01) exit(); // escape
  if(scancode == 75) keyPress(100, 0, 0); // -- left
  if(scancode == 72) keyPress(101, 0, 0); // -- up
  if(scancode == 77) keyPress(103, 0, 0); // -- right
  if(scancode == 80) keyPress(102, 0, 0); // -- down
}

void update() {
  myDisplay();
  graphicsFlush();
}

void init() { }

#endif

#define GL_COLOR_BUFFER_BIT 0
#define GL_PROJECTION 0
#define GL_FRONT_AND_BACK 0
#define GL_LINES 0
#define GL_LINE 0
#define GL_LINE_LOOP 0
#define GL_MODELVIEW 0
#define GL_FRONT_AND_BACK 0
#define GL_LINE 0
#define M_PI 3.14159265358979323846

// -- https://de.wikipedia.org/wiki/Bresenham-Algorithmus#Kompakte_Variante_2 --
void drawLine(int x0, int y0, int x1, int y1, uint32_t colour) {
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2; /* error value e_xy */

  while (1) {
    graphicsSetPixel(x0, y0, colour);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}
// -- --

float translateX = 0;
float translateY = 0;
float translateZ = 0;
void glTranslated(float x, float y, float z) {
  translateX = x;
  translateY = y;
  translateZ = z;
}

#define DEG2RAD M_PI/180.0
float rotationAngle = 0;
void glRotatef(float angle, float x, float y, float z) {
  rotationAngle = angle * DEG2RAD;
}

// --
void drawScaledLine(float x1, float y1, float x2, float y2, uint8_t color) {
  float factor_y = 5;
  float factor_x = 4.8;

  // -- rotate
  /*x1 = x1 * cos(rotationAngle) - y1 * sin(rotationAngle);
  y1 = x1 * sin(rotationAngle) + y1 * cos(rotationAngle);

  x2 = x2 * cos(rotationAngle) - y2 * sin(rotationAngle);
  y2 = x2 * sin(rotationAngle) + y2 * cos(rotationAngle);*/

  // -- translate
  x1 += translateX;
  x2 += translateX;
  y1 += translateY;
  y2 += translateY;

  drawLine(x1 * factor_x, 480 - y1  * factor_y, x2  * factor_x, 480 - y2  * factor_y, color);
}

void glutInit() {
  init();

  myReshape(640, 480);
  myKey('s', 0, 0); // -- hit 's' to start game
}

void glutMainLoop() {
  int loop = 0;

  while(1) {
    update();


    if(loop % 10 == 0) {
      myTimer(1000);
    }
    usleep(10);

    loop++;
  }
}

void glViewport(int a, int b, int c, int d) { }

void glClear(int bufferBit) {
  for(uint16_t x = 0; x < 640; x++) {
    for(uint16_t y = 0; y < 480; y++) {
      graphicsSetPixel(x, y, 8);
    }
  }
}

void glutSwapBuffers() { }
void glutPostRedisplay() { }
void glMatrixMode(int p) { }
void glLoadIdentity() { }
void glOrtho(float a, float b, float c, float d, float e, float f) { };
void glPolygonMode(int a, int b) { }
void glutTimerFunc(int type, void * callback, int interval) {
  //printf("set timer %d, callback %p with interval %d\n", type, callback, interval);
}

typedef struct SPoint {
  float x;
  float y;
} SPoint;
SPoint points[20];
uint8_t point_no = 0;

void glBegin(int a) {
  point_no = 0;
}

void glVertex2f(float a, float b) {
  points[point_no].x = a;
  points[point_no].y = b;
  point_no++;
}

void glEnd() {
  for(uint8_t i = 0; i < point_no - 1; i++) {
    drawScaledLine(points[i].x, points[i].y, points[i+1].x, points[i+1].y, 15);
  }
  drawScaledLine(points[0].x, points[0].y, points[point_no-1].x, points[point_no-1].y, 15);

  // -- reset polygon points, translation, rotation
  point_no = 0;
  translateX = 0;
  translateY = 0;
  translateZ = 0;
  rotationAngle = 0;
}

/* ================================================================================
Noah Attwood
Asteroids
Summer 2018
GitHub: attwoodn/c-asteroids
================================================================================ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
//#include <GL/glut.h>
//#include "myglut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define myTranslate2D(x,y) glTranslated(x, y, 0.0)
#define myScale2D(x,y) glScalef(x, y, 1.0)
#define myRotate2D(angle) glRotatef(RAD2DEG*angle, 0.0, 0.0, 1.0)

#define TIME_DELTA      33

#define SPAWN_ASTEROID_PROB     0.004
#define MAX_SHIP_VELOCITY       0.05
#define PHOTON_VELOCITY         MAX_SHIP_VELOCITY * 2
#define PHOTON_LENGTH           2.5
#define MAX_PHOTONS             8
#define MAX_ASTEROIDS	        8
#define MAX_VERTICES	        16


/* -- type definitions ------------------------------------------------------ */

typedef struct Coords {
	double x, y;
} Coords;

typedef struct {
    int status; /* status: 0 = destroyed    1 = active, normal    2 = invincible */
	double x, y, phi, dx, dy, radius, turnSpeed, acceleration;
    Coords  coords[3];
} Ship;

typedef struct {
	int	active;
	double x1, y1, x2, y2, dx, dy, phi;
} Photon;

typedef struct {
	int	active, nVertices;
	double x, y, phi, dx, dy, dphi, radius;
	Coords	coords[MAX_VERTICES];
} Asteroid;


/* -- function prototypes --------------------------------------------------- */

static void	myDisplay(void);
static void	myTimer(int value);
static void myPauseTimer(int value);
static void myMenuTimer(int value);
static void	myKey(unsigned char key, int x, int y);
static void	keyPress(int key, int x, int y);
static void	keyRelease(int key, int x, int y);
static void	myReshape(int w, int h);

static void	initGame(void);
static void	initAsteroid(Asteroid *a, double x, double y, double size);
static void	drawShip(Ship *s);
static void	drawPhoton(Photon *p);
static void	drawAsteroid(Asteroid *a);

static void debug(void);
static void updateShip(void);
static void processUserInput(void);
static void updatePhotons(void);
static void spawnAsteroids(void);
static void advanceAsteroids(void);
static void destroyAndResetShip(void);

static double myRandom(double min, double max);
static double clamp(double value, double min, double max);
static void initPhoton(void);
static int isInBounds(double x, double y);
static void raycastForNewCoordinates(double x, double y, double dx, double dy, double * newCoords);
static void checkPhotonAsteroidCollision(void);
static void checkShipAsteroidCollision(void);
static void resetAsteroidShape(void);


/* -- global variables ------------------------------------------------------ */

static int	up = 0, down = 0, left = 0, right = 0, firing = 0, circularAsteroids = 0, pause2 = 0, started = 0; // state of user input
static double xMax, yMax;
static int timer = 0, respawnTimer = 0;
static Ship	ship;
static Photon	photons[MAX_PHOTONS];
static Asteroid	asteroids[MAX_ASTEROIDS];


/* -- main ------------------------------------------------------------------ */

int
main(int argc, char *argv[])
{
    srand((unsigned int) time(NULL));

    glutInit(&argc, argv);

    #ifndef CUSTOMOS
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Asteroids");
    glutDisplayFunc(myDisplay);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(myKey);
    glutSpecialFunc(keyPress);
    glutSpecialUpFunc(keyRelease);
    glutReshapeFunc(myReshape);
    glutTimerFunc(TIME_DELTA, myMenuTimer, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    #endif

    initGame();

    glutMainLoop();

    return 0;
}


/* ================================================ GLUT Callback Functions ============================================== */

void
myDisplay()
{
    /*
     *	display callback function
     */

    int	i;

    glClear(GL_COLOR_BUFFER_BIT);

    drawShip(&ship);

    for (i=0; i<MAX_PHOTONS; i++){
    	if (photons[i].active){
	    drawPhoton(&photons[i]);
	}
    }

    if(started){
	for (i=0; i<MAX_ASTEROIDS; i++){
		if (asteroids[i].active){
		drawAsteroid(&asteroids[i]);
	    }
	}
    }

    glutSwapBuffers();
}

void
myTimer(int value)
{
    /*
     *	timer callback function
     */

    debug();
    updatePhotons();
    processUserInput();
    updateShip();
    spawnAsteroids();
    advanceAsteroids();
    checkPhotonAsteroidCollision();
    checkShipAsteroidCollision();

    glutPostRedisplay();

    if(pause2){
	glutTimerFunc(TIME_DELTA, myPauseTimer, value);
    } else {
	glutTimerFunc(TIME_DELTA, myTimer, value);
    }
}

void
myPauseTimer(int value){
    if(pause2){
	glutTimerFunc(TIME_DELTA, myPauseTimer, value);
    } else {
	glutTimerFunc(TIME_DELTA, myTimer, value);
    }

}

void
myMenuTimer(int value){
    if(started){
	glutTimerFunc(TIME_DELTA, myTimer, value);
    } else {
	debug();
	updatePhotons();
	processUserInput();
	updateShip();

	glutPostRedisplay();

	glutTimerFunc(TIME_DELTA, myMenuTimer, value);
    }
}

void
myKey(unsigned char key, int x, int y) {
    /*
     *  keyboard callback function
     */
    if(started){
	switch(key) {
	    case ' ':
		if(ship.status && !pause2){
		    // ship can only shoot if it is "alive"
		    initPhoton();
		}
		break;
	    case 'q':
		exit(0); break;
	    case 'c':
		resetAsteroidShape(); break;
	    case 'p':
		pause2 = abs(pause2 - 1); break;
	}

    } else {
	switch(key) {
	    case 's':
		started = 1; break;
	    case ' ':
		initPhoton(); break;
	    case 'q':
		exit(0); break;
	}
    }
}

void
keyPress(int key, int x, int y) {
    /*
     *  this function is called when a special key is pressed; we are
     *  interested in the cursor keys only
     */

    if(ship.status){
	switch (key) {
	    case 100:
		left = 1; break;
	    case 101:
		up = 1; break;
	    case 102:
		right = 1; break;
	    case 103:
		down = 1; break;
	}
    }
}

void
keyRelease(int key, int x, int y) {
    /*
     *  this function is called when a special key is released; we are
     *  interested in the cursor keys only
     */

    switch (key) {
	case 100:
	    left = 0; break;
	case 101:
	    up = 0; break;
	case 102:
	    right = 0; break;
	case 103:
	    down = 0; break;
    }
}

void
myReshape(int w, int h) {
    /*
     *  reshape callback function; the upper and lower boundaries of the
     *  window are at 100.0 and 0.0, respectively; the aspect ratio is
     *  determined by the aspect ratio of the viewport
     */

    xMax = 100.0*w/h;
    yMax = 100.0;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, xMax, 0.0, yMax, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
}


/* ============================================ Processing Functions ======================================= */

/**
 *  Print debug information to the console
 */
void
debug() {
    timer += TIME_DELTA;

    if (timer >= 3000) {
	printf("ship coords: (%.3f, %.3f)\n", ship.x, ship.y);
	printf("direction: %.3f\n", ship.phi);
	printf("dx: %.4f\n", ship.dx);
	printf("dy: %.4f\n", ship.dy);

	printf("xMax: %.3f     yMax: %.3f\n", xMax, yMax);

	int i, numActiveAsteroids = 0;
	for(i = 0; i < MAX_ASTEROIDS; i++){
	    if(asteroids[i].active){
		numActiveAsteroids ++;
	    }
	}

	printf("numActiveAsteroids: %d\n", numActiveAsteroids);

	printf("\n\n");
	timer -= 3000;
    }
}

/**
 *  Respond to user input by turning or accelerating the ship.
 */
void
processUserInput() {
    if (right){
	ship.phi -= ship.turnSpeed;
	if (ship.phi < 0.0){
	    ship.phi += 360.0;
	}
    }

    if (left){
	ship.phi += ship.turnSpeed;
	if (ship.phi > 360.0){
	    ship.phi -= 360.0;
	}
    }

    if (up || down){
	double newDx;
	double newDy;

	if (up){
	    // up velocity calculation
	    newDx = ship.dx - (ship.acceleration * sin((ship.phi - 90.0) * DEG2RAD) * TIME_DELTA);
	    newDy = ship.dy + (ship.acceleration * cos((ship.phi - 90.0) * DEG2RAD) * TIME_DELTA);
	} else {
	    // down velocity calculation. Braking acceleration is 33% slower for extra challenge
	    newDx = ship.dx + ((ship.acceleration/1.5) * sin((ship.phi - 90.0) * DEG2RAD) * TIME_DELTA);
	    newDy = ship.dy - ((ship.acceleration/1.5) * cos((ship.phi - 90.0) * DEG2RAD) * TIME_DELTA);
	}

	double velocityMagnitude = sqrt(pow(newDx, 2) + pow(newDy, 2));

	if (velocityMagnitude <= MAX_SHIP_VELOCITY){
	    ship.dx = newDx;
	    ship.dy = newDy;
	} else {
	    // new velocity will exceed max ship velocity
	    // subtract the difference to make it the same magnitude as the max ship velocity
	    velocityMagnitude -= (velocityMagnitude - MAX_SHIP_VELOCITY);
	    double thetaRadians = atan(newDy/newDx);

	    if (newDx < 0){
		ship.dx = velocityMagnitude * cos(thetaRadians - M_PI);
		ship.dy = velocityMagnitude * sin(thetaRadians - M_PI);
	    } else {
		ship.dx = velocityMagnitude * cos(thetaRadians);
		ship.dy = velocityMagnitude * sin(thetaRadians);
	    }
	}
    }
}

/**
 *  Advances the ship. This entails incrementing the ship's position and resetting the ship
 *  within the bounds of the screen when it leaves.
 */
void
updateShip(){

    if(ship.status == 0){
	// ship has been destroyed and is waiting to respawn
	respawnTimer += TIME_DELTA;

	if(respawnTimer >= 3000){
	    respawnTimer = 0;
	    ship.status = 2;
	}

    } else if(ship.status == 2){
	// ship is invincible
	respawnTimer += TIME_DELTA;
	if (respawnTimer >= 4000){
	    // ship has been respawned for a while so take away invincibility
	    respawnTimer = 0;
	    ship.status = 1;
	}
    }


    // update x and y center coordinates
    ship.x += ship.dx*TIME_DELTA;
    ship.y += ship.dy*TIME_DELTA;

    // check if ship left the screen
    if (!isInBounds(ship.x, ship.y)){

	// ship is out of the screen. Need to reset the ship on the opposite side of the screen
	double newCoords[2];
	raycastForNewCoordinates(ship.x, ship.y, ship.dx, ship.dy, newCoords);

	ship.x = newCoords[0];
	ship.y = newCoords[1];
    }

    // update the 3 vertices that make up the ship
    float thetas[3];
    thetas[0] = (0.0f + ship.phi) * DEG2RAD;
    thetas[1] = (135.0f + ship.phi) * DEG2RAD;
    thetas[2] = (225.0f + ship.phi) * DEG2RAD;

    int i;
    for(i = 0; i < 3; i++){
	ship.coords[i].x = ship.x + (ship.radius * cos(thetas[i]));
	ship.coords[i].y = ship.y + (ship.radius * sin(thetas[i]));
    }
}

/**
 *  Iterate over all active photons and updates their position. When an active photon has left the screen,
 *  it is changed to not active.
 */
void
updatePhotons(){
    int i;
    for(i = 0; i < MAX_PHOTONS; i++){
	if(photons[i].active){
	    Photon p = photons[i];
	    p.x1 += p.dx*TIME_DELTA;
	    p.y1 += p.dy*TIME_DELTA;
	    p.x2 += p.dx*TIME_DELTA;
	    p.y2 += p.dy*TIME_DELTA;

	    if(!isInBounds(p.x1, p.y1)){
		p.active = 0;
	    }

	    photons[i] = p;
	}
    }
}

void
spawnAsteroids(){
    // determine the number of active asteroids on the screen
    int i, numActiveAsteroids = 0;
    for (i = 0; i < MAX_ASTEROIDS; i++){
	if (asteroids[i].active){
	    numActiveAsteroids++;
	}
    }

    if (numActiveAsteroids == 0 || (numActiveAsteroids < (MAX_ASTEROIDS-3) && myRandom(0.0, 1.0) < SPAWN_ASTEROID_PROB)){
	Asteroid newAsteroid;

	for (i = 0; i < MAX_ASTEROIDS; i++){
	    if (!asteroids[i].active){
		newAsteroid = asteroids[i];
		break;
	    }
	}

	// cast double return value to an integer, giving possible values in the range [0-3]
	int spawnSide = myRandom(0.0, 3.9999);

	double spawnX, spawnY;
	switch (spawnSide){
	    case 0:
		// spawn on the right side of the screen
		spawnX = xMax;
		spawnY = myRandom(0.0, yMax);
		break;
	    case 1:
		// spawn on the bottom side of the screen
		spawnX = myRandom(0.0, xMax);
		spawnY = 0.0;
		break;
	    case 2:
		// spawn on the left side of the screen
		spawnX = 0.0;
		spawnY = myRandom(0.0, yMax);
		break;
	    case 3:
		// spawn on the top side of the screen
		spawnX = myRandom(0.0, xMax);
		spawnY = yMax;
		break;
	}

	printf("Spawning an asteroid\nspawnSide: %i\nspawnX: %.3f    spawnY: %.3f\n\n", spawnSide, spawnX, spawnY);

	initAsteroid(&newAsteroid, spawnX, spawnY, 2.2);
	asteroids[i] = newAsteroid;
    }
}

void
advanceAsteroids(){
    int i;
    for (i = 0; i < MAX_ASTEROIDS; i++){
	if(asteroids[i].active){
	    //printf("updating asteroid at index %i\noldX: %.3f    oldY: %.3f\n", i, asteroids[i].x, asteroids[i].y);
	    Asteroid a = asteroids[i];

	    a.x += a.dx*TIME_DELTA;
	    a.y += a.dy*TIME_DELTA;
	    a.phi += a.dphi;
	    //printf("newX: %.3f    newY: %.3f\n", a.x, a.y);

	    if (a.x > xMax){
		a.x = 0.0;
	    } else if (a.x < 0.0){
		a.x = xMax;
	    }

	    if (a.y > yMax){
		a.y = 0.0;
	    } else if (a.y < 0.0){
		a.y = yMax;
	    }

	    asteroids[i] = a;
	}
    }
}

void
destroyAndResetShip(){
    left = 0;
    right = 0;
    up = 0;
    down = 0;
    ship.status = 0;
    ship.x = 66.66;
    ship.y = 50.0;
    ship.dx = 0.000000;
    ship.dy = 0.000000;
    ship.phi = 90.00000;
}

/* ============================================= Drawing Functions ========================================= */

void
drawShip(Ship *s) {
    int i;
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_LINE_LOOP);

    if(s->status == 1 || s->status == 2 && respawnTimer % (TIME_DELTA*12) < (TIME_DELTA*6)){
	for (i = 0; i < 3; i++){
	    glVertex2f(s->coords[i].x, s->coords[i].y);
	}
    }

    glEnd();
}

void
drawPhoton(Photon *p) {
    glLoadIdentity();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_LINES);
    glVertex2f(p->x1, p->y1);
    glVertex2f(p->x2, p->y2);
    glEnd();
}

void
drawAsteroid(Asteroid *a) {
    int i;
    glLoadIdentity();

    // move to the asteroid's x and y
    myTranslate2D(a->x, a->y);
    myRotate2D(a->phi);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_LINE_LOOP);
    for(i = 0; i < a->nVertices; i++){
	//printf("drawing asteroid vertex    x: %.3f    y: %.3f\n", a->coords[i].x, a->coords[i].y);
	glVertex2f(a->coords[i].x, a->coords[i].y);
    }
    glEnd();
}


/* ========================================== Initialization Functions ===================================== */

void
initGame()
{
    /*
     * set parameters including the numbers of asteroids and photons present,
     * the maximum velocity of the ship, the velocity of the laser shots, the
     * ship's coordinates and velocity, etc.
     */

    /** initialize ship **/
    ship.x = 66.66;
    ship.y = 50.0;
    ship.dx = 0.0;
    ship.dy = 0.0;
    ship.phi = 90.0;
    ship.radius = 2.5;
    ship.acceleration = 0.00002;
    ship.turnSpeed = 3.4;
    ship.status = 1;
}

void
initAsteroid(Asteroid *a, double x, double y, double size) {
    /*
     *  generate an asteroid at the given position; velocity, rotational
     *  velocity, and shape are generated randomly; size serves as a scale
     *  parameter that allows generating asteroids of different sizes;
     */

    double  theta, r;
    int     i;

    a->x = x;
    a->y = y;
    a->phi = 0.0;
    a->dx = myRandom(-0.02, 0.02);
    a->dy = myRandom(-0.02, 0.02);
    a->dphi = myRandom(-0.1, 0.1);

    a->nVertices = 6+rand()%(MAX_VERTICES-6);

    if(circularAsteroids){
	r = size * 2.5;
	a->radius = r;
	for (i=0; i<a->nVertices; i++) {
	   theta = 2.0*M_PI*i/a->nVertices;
	   a->coords[i].x = r*sin(theta);
	   a->coords[i].y = r*cos(theta);
	}
    } else {
	a->radius = size * 2.0;
	for (i=0; i<a->nVertices; i++) {
	   theta = 2.0*M_PI*i/a->nVertices;
	   r = size*myRandom(2.0, 3.0);
	   a->coords[i].x = r*sin(theta);
	   a->coords[i].y = r*cos(theta);
	}
    }

    a->active = 1;
}

void
initPhoton(){
    int i;
    for(i = 0; i < MAX_PHOTONS; i++){
	Photon p = photons[i];
	if (p.active == 0){
	    p.active = 1;
	    p.phi = ship.phi;
	    p.x1 = ship.x + (ship.radius * cos(p.phi * DEG2RAD));
	    p.y1 = ship.y + (ship.radius * sin(p.phi * DEG2RAD));
	    p.x2 = p.x1 + (PHOTON_LENGTH * cos(p.phi * DEG2RAD));
	    p.y2 = p.y1 + (PHOTON_LENGTH * sin(p.phi * DEG2RAD));
	    p.dx = -PHOTON_VELOCITY * sin((p.phi - 90.0) * DEG2RAD);
	    p.dy = PHOTON_VELOCITY * cos((p.phi - 90.0) * DEG2RAD);
	    photons[i] = p;
	    //printf("firing a photon:\nv1: (%.2f, %.2f)    v2: (%.2f, %.2f)    dx: %.2f    dy: %.2f    phi: %.2f\n\n", p.x1, p.y1, p.x2, p.y2, p.dx, p.dy, p.phi);
	    break;
	}
    }
}


/* ============================================== Helper Functions ========================================= */

double
myRandom(double min, double max)
{
	/* return a random number uniformly draw from [min,max] */
	return min+(max-min)*(rand()%0x7fff)/32767.0;
}

/**
 *  clamps value between min and max
 */
double
clamp (double value, double min, double max){
    value = value <= max ? value : max;
    value = value >= min ? value : min;
    return value;
}

/**
 *  checks to see if x and y are within the bounds of the screen
 */
int
isInBounds(double x, double y){
    if(x < 0 || x > xMax || y < 0 || y > yMax){
	return 0;
    }

    return 1;
}


void
raycastForNewCoordinates(double x, double y, double dx, double dy, double * newCoords){
    // assume that x and y are outside of the bounds of the screen. They must be brought back into the screen
    double reEnterX = clamp(x, 0.0, xMax);
    double reEnterY = clamp(y, 0.0, yMax);

    // perform a raycast using the opposite of the object's current velocity. When the ray goes out of the screen,
    // this gives an approximation of where the object should re-enter
    while (isInBounds(reEnterX, reEnterY)){
	reEnterX -= dx*2;
	reEnterY -= dy*2;
    }

    // clamp the re-entry values to be within the screen's bounds
    newCoords[0] = clamp(reEnterX, 0.0, xMax);
    newCoords[1] = clamp(reEnterY, 0.0, yMax);
}

void
checkPhotonAsteroidCollision(){
    int photonIndex, asteroidIndex;
    for(photonIndex = 0; photonIndex < MAX_PHOTONS; photonIndex ++){
	if (photons[photonIndex].active){
	    // for each active photon...
	    for (asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex ++){
		if (asteroids[asteroidIndex].active){
		    // ... and for each active asteroid, check if there was a collision
		    Photon p = photons[photonIndex];
		    Asteroid a = asteroids[asteroidIndex];

		    double xPow1 = pow((p.x2 - a.x), 2);
		    double yPow1 = pow((p.y2 - a.y), 2);
		    double xPow2 = pow((p.x1 - a.x), 2);
		    double yPow2 = pow((p.y1 - a.y), 2);
		    double r2 = pow(a.radius, 2);

		    if(xPow1 + yPow1 <= r2 || xPow2 + yPow2 <= r2){
			// there was a collision between this photon and this asteroid
			photons[photonIndex].active = 0;
			asteroids[asteroidIndex].active = 0;

			if(a.radius > 2.2){
			    // if the asteroid is big enough, create some child asteroids
			    int i, maxChildAsteroids = 3, childAsteroidCount = 0;
			    for(i = 0; i < MAX_ASTEROIDS; i++){
				if (!asteroids[i].active && childAsteroidCount < maxChildAsteroids){
				    childAsteroidCount++;

				    initAsteroid(&asteroids[i], a.x, a.y, a.radius/4.0);
				}
			    }
			}

			break;
		    }
		}
	    }
	}
    }
}

void
checkShipAsteroidCollision(){
    int asteroidIndex, shipVertexIndex;
    for(asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; asteroidIndex++){
	if(asteroids[asteroidIndex].active && ship.status == 1){
	    for(shipVertexIndex = 0; shipVertexIndex < 3; shipVertexIndex++){
		Asteroid a = asteroids[asteroidIndex];

		double xPow = pow((ship.coords[shipVertexIndex].x - a.x), 2);
		double yPow = pow((ship.coords[shipVertexIndex].y - a.y), 2);
		double r2 = pow(a.radius, 2);

		if(xPow + yPow <= r2){
		    // there was a collision between the ship and this asteroid
		    destroyAndResetShip();
		    break;
		}
	    }
	}
    }
}

void
resetAsteroidShape(){
    // change value from 0 -> 1 or from 1 -> 0 and clear asteroids
    circularAsteroids = abs(circularAsteroids - 1);
    int i;
    for (i = 0; i < MAX_ASTEROIDS; i ++){
	asteroids[i].active = 0;
    }
}
