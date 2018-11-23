#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TIMER_PERIOD 25
#define SCALE_SPEED 1.020 //the scale amount applied to the hexagons in every frame
#define HEXAGON_DELAY 1250 //the amount of miliseconds passed between hexagons
#define COLOR_CHANGE true //true for repeatedly color change, false for no color change
typedef struct {
	float r, g, b;
} color_t;

typedef struct {
	color_t color;	 //randomly generated r,g,b color
	float scale;	 //scale of the hexagon, which increases in time
	int missingPart; //the empty part of the hexagon so that player can pass through
} hexagon_t;

typedef struct {
	bool isStarted, //checks if the game is started or is over
		pause,		//checks if the game is paused
		animate;
} game_control_t;

typedef struct {
	int current,
		max = -1;
} score_t;

score_t score; //keeps current and the maximum score
int width, height;
game_control_t game;
hexagon_t hexagons[4];
int input = 0;	   //the screen is splitted into 6 parts and as players uses arrow keys, the value of this variable changes accordingly
int timerCount = 0;//counts how many times timer function runned
color_t background;//background color

float maxScale; //after this scale, the scale of the hexagons will be initialized to initialScale
float initialScale; //

float rotation;	   //rotation of all objects, hexagons and players, which changes in time
float rotateSpeed;//rotation speed, which randomly switch between 1 and 3

float scale;	  //scale of the whole screen, used in the initial animation and in game in general
void initializeGlobals()
{
	input = 0;
	scale = 1;
	score.current = 0;
	game.isStarted = false;
	game.pause = false;
	game.animate = false;
	rotateSpeed = (rand() % 100) / 50.0 + 1;
	rotation = 0;
	float frameNeeded = HEXAGON_DELAY / TIMER_PERIOD;
	float scaleMultiplier = powf(SCALE_SPEED, frameNeeded);
	float hexagonScales[4];
	hexagonScales[3] = 0.2;
	for (int i = 2; i >= 0; i--)
		hexagonScales[i] = hexagonScales[i + 1] / scaleMultiplier;
	initialScale = hexagonScales[2];
	maxScale = hexagonScales[3] * pow(scaleMultiplier,3);
	for (int i = 0; i < 4; i++)
	{
		hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
		hexagons[i].scale = hexagonScales[i];
		hexagons[i].missingPart = rand() % 6;
	}
	background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
}

//
// to draw circle, center at (x,y)
//  radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void drawString(const char* string)
{
	glPushMatrix();
	while (*string)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
	glPopMatrix();
}

void displayBackground()
{
	glColor4f(0, 0, 0, 0.1);
	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glRotatef(i * 120.0 + rotation, 0, 0, 1);
		glBegin(GL_TRIANGLES);
		glVertex2f(0, 0);
		glVertex2f(-1500 / sqrt(3), -1500);
		glVertex2f(1500 / sqrt(3), -1500);
		glEnd();
		glPopMatrix();
	}
}

void displayHexagons()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (j == hexagons[i].missingPart)
				continue;
			glColor3f(hexagons[i].color.r, hexagons[i].color.g, hexagons[i].color.b);
			glPushMatrix();
			glScalef(hexagons[i].scale, hexagons[i].scale, 0);
			glRotatef(j * 60.0 + rotation, 0, 0, 1);
			glTranslatef(0, -100 * sqrt(3), 0);
			glRectf(-100, 0, 100, -5);
			glPopMatrix();
		}
	}
}

void displayPlayer()
{
	glPushMatrix();
	glColor3f(1, 1, 1);
	glRotatef(input * 60.0 + rotation, 0, 0, 1);
	glTranslatef(0, -200, 0);
	circle(0, 0, 5);
	glPopMatrix();
}

void displayUI()
{
	glPushMatrix();
	if (!game.isStarted)
	{
		glColor4f(0, 0, 0, 0.7);
		glRectf(-300, -100, 300, 100);
		glColor3f(1, 1, 1);
		glTranslatef(-100, 0, 0);
		glScalef(0.3, 0.3, 0);
		drawString("F1 to Start");
		glTranslatef(-40, -100, 0);
		glScalef(0.5, 0.5, 0);
		drawString("Arrow keys to move < >");

		if (score.max != -1)
		{
			char str[100];
			glTranslatef(150, -200, 0);
			sprintf(str, "Max Score: %d", score.max);
			drawString(str);
			glTranslatef(0, -150, 0);
			sprintf(str, "Last Score: %d", score.current);
			drawString(str);
		}

	}
	else
	{
		glTranslatef(-290, 280, 0);
		glScalef(0.1, 0.1, 0);
		if (!game.pause)
			drawString("F2 to Pause");
		else
			drawString("F2 to Continue");
		char str[100];
		glPopMatrix();
		glPushMatrix();
		sprintf(str, "Score: %d", score.current);
		glTranslatef(230, 280, 0);
		glScalef(0.1, 0.1, 0);
		drawString(str);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(230, -280, 0);
		glScalef(0.1, 0.1, 0);
		drawString("F1 to reset");
	}
	glPopMatrix();

}

//
// To display onto window using OpenGL commands
//
void display()
{
	glClearColor(background.r, background.g, background.b, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glScalef(scale, scale, 0);
	if (game.animate)
		glRotatef(rotation, 0, 0, 1);

	displayBackground();
	displayHexagons();
	displayPlayer();
	displayUI();

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void ASCIIKeyDown(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

void ASCIIKeyUp(unsigned char key, int x, int y)
{
}


//
// Special Key like F1, F2, F3, Arrow Keys, Page UP, ...
//
void SpecialKeyDown(int key, int x, int y)
{
	if (game.isStarted && !game.pause)
	{
		switch (key) {
		case GLUT_KEY_LEFT:
			input = (input + 5) % 6;
			break;
		case GLUT_KEY_RIGHT:
			input = (input + 1) % 6;
			break;
		}
	}
	if (key == GLUT_KEY_F1)
	{
		initializeGlobals();
		game.animate = true;
		scale = 9;

	}
	else if (key == GLUT_KEY_F2)
		game.pause = !game.pause;
}


void SpecialKeyUp(int key, int x, int y)
{
}


//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void MouseClick(int button, int stat, int x, int y)
{

}


void MouseMove(int x, int y)
{
}


void MouseDownMove(int x, int y)
{
}


//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	//initial animation of the game
	if (game.animate)
	{
		timerCount++;
		scale -= 0.1;
		rotation += 9;
		if (scale <= 1)
		{
			scale = 1;
			rotation = 0;
			game.animate = false;
			game.isStarted = true;
		}
	}
	if (game.isStarted && !game.pause)
	{
		timerCount++;
		rotation += rotateSpeed;
		for (int i = 0; i < 4; i++)
		{
			if (fabs(hexagons[i].scale - 1.130) < 0.01)
				if (input != hexagons[i].missingPart)
				{
					game.isStarted = false;
					if (score.current > score.max)
						score.max = score.current;
				}
				else
					score.current++;
			hexagons[i].scale *= SCALE_SPEED;
			if (hexagons[i].scale >= maxScale)
			{
				hexagons[i].scale = initialScale;
			}
		}
		if (timerCount % 25 == 0 && COLOR_CHANGE)
		{
			for (int i = 0; i < 4; i++)
			{
				hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
			}
			background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
			rotateSpeed = (rand() % 100) / 20.0 - 2.5f;
		}
		else if (timerCount % 50 == 0)
		{
			int rnd = rand() % 60 - 30;
			if (rnd == 0)
				rnd = 180;
			rotation += rnd;
			timerCount = 0;
		}
	}
	glutPostRedisplay();
}

void main(int argc, char *argv[])
{
	initializeGlobals();
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Template");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//
	// keyboard registration
	//
	glutKeyboardFunc(ASCIIKeyDown);
	glutKeyboardUpFunc(ASCIIKeyUp);

	glutSpecialFunc(SpecialKeyDown);
	glutSpecialUpFunc(SpecialKeyUp);
	//
	// mouse registration
	//
	glutMouseFunc(MouseClick);
	glutPassiveMotionFunc(MouseMove);
	glutMotionFunc(MouseDownMove);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	glutMainLoop();
}