                                                                                                     /*
/##################################################################################################\
# Description : #                                                                                  #
#################                                                                                  #
#                                                                                                  #
#  Ce fichier est le point d'entrée du moteur ( le "main()" ).                                     #
#                                                                                                  #
#                                                                                                  #
\##################################################################################################/
                                                                                                     */

#include "window.h"
#include "input.h"
#include "timer.h"
#include "ogl.h"
#include "load.h"
#include "camera.h"
#include <vector>
#include <iostream>

/****************************************************************************\
*                                                                            *
*                            Variables EXTERNES                              *
*                                                                            *
\****************************************************************************/


/****************************************************************************\
*                                                                            *
*                            Variables GLOBALES                              *
*                                                                            *
\****************************************************************************/
WINDOW    *win = NULL;
MY_INPUT     *inp = NULL;
TIMER     *tim = NULL;

Image *heightmap_terrain = NULL;
Image *heightmap_terrain_reference = NULL;

Texture *texture_wall = NULL;
Texture *texture_grassland = NULL;
Texture *texture_brick = NULL;

CAMERA *cam = NULL;

point posStart;
point posLast;
float posX = 0;
float posY = 0;
float posZ = 0;
float angleX = 0;
float angleY = 0;
float angleZ = 0;

float size1 = 5;
float size2 = 5;

bool b_init = false;
bool b_canMoveAtX = true;
bool b_canMoveAtZ = true;
float f_durationCreateCube = 1;
float f_durationCreateSphere = 1;
float f_durationDestroySphere = 3.5;
float f_durationParticle = 0.7;
float f_timerCube = 0;
float f_timerSphere = 0;
float f_timer = 0;
float f_timerLastJump = 0;

struct MySphere
{
	float f_timeCreated;
	bool b_phere;
};

bool **bPtr_bricks;
bool **bPtr_bonus;
MySphere **ptr_spheres;

struct MyParticleSystem
{
	float f_timeCreated;
	point centerCreated;
};

std::vector<MyParticleSystem> vector_particleSystem;
/****************************************************************************\
*                                                                            *
*                             Variables LOCALES                              *
*                                                                            *
\****************************************************************************/





/********************************************************************\
*                                                                    *
*  Démarre l'application (avant la main_loop). Renvoie false si      *
*  il y a une erreur.                                                *
*                                                                    *
\********************************************************************/

point convertPointFromPixelToWorld(int x, int y);
point convertPointFromWorldToPixel(point posInWorld);
float getHeightFromPixelToWorldByWorldPosition(point posInWorld);
point convertPointFromPixelToWorldV2(int x, int y);
point convertPointFromWorldToPixelV2(point posInWorld);
float getHeightFromPixelToWorldByWorldPositionV2(point posInWorld);
void cube(point position);

bool start()
{
	// initialisations de base
	// -----------------------
	win = new WINDOW();									// prépare la fenêtre
	win->create(800, 600, 16, 60, false);			// crée la fenêtre

	tim = new TIMER();									// crée un timer

	inp = new MY_INPUT(win);								// initialise la gestion clavier souris
	create_context(*win);								// crée le contexte OpenGL sur la fenêtre
	init_font(*win, "Courier");							// initialise la gestion de texte avec OpenGL

	// initialisations d'OpenGL
	// ------------------------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);					// set clear color for color buffer (RGBA, black)
	glViewport(0, 0, win->Xres, win->Yres);				// zone de rendu (tout l'écran)
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LESS);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// mapping quality = best
	glFrontFace(GL_CCW);								// front of face is defined counter clock wise
	glPolygonMode(GL_FRONT, GL_FILL);					// front of a face is filled
	//glPolygonMode(GL_FRONT, GL_LINE);					// front of a face is filled
	glPolygonMode(GL_BACK, GL_FILL);					// back of a face is made of lines
	glCullFace(GL_BACK);								// cull back face only
	glDisable(GL_CULL_FACE);						    // disable back face culling

	heightmap_terrain = new Image();
	heightmap_terrain->load_tga("./data/migong_heightmap_V2.tga");

	heightmap_terrain_reference = new Image();
	heightmap_terrain_reference->load_tga("./data/migong_heightmap_reference_V2.tga");

	texture_wall = new Texture();
	texture_wall->load_texture("./data/wall.tga", NULL);

	texture_grassland = new Texture();
	texture_grassland->load_texture("./data/grassland.tga", NULL);

	texture_brick = new Texture();
	texture_brick->load_texture("./data/brick.tga", NULL);

	cam = new CAMERA();
	//myMath = new MyMath();


	win->set_title("Mon jeu");

	bPtr_bricks = new bool*[heightmap_terrain->lenx / 4];
	for (int i = 0; i < heightmap_terrain->lenx / 4; ++i)
	{
		bPtr_bricks[i] = new bool[heightmap_terrain->leny / 4];

		for (int j = 0; j < heightmap_terrain->leny / 4; ++j)
		{
			*(bPtr_bricks[i] + j) = false;
		}
	}

	bPtr_bonus = new bool*[heightmap_terrain->lenx / 4];
	for (int i = 0; i < heightmap_terrain->lenx / 4; ++i)
	{
		bPtr_bonus[i] = new bool[heightmap_terrain->leny / 4];

		for (int j = 0; j < heightmap_terrain->leny / 4; ++j)
		{
			*(bPtr_bonus[i] + j) = false;
		}
	}

	ptr_spheres = new MySphere*[heightmap_terrain->lenx / 4];
	for (int i = 0; i < heightmap_terrain->lenx / 4; ++i)
	{
		ptr_spheres[i] = new MySphere[heightmap_terrain->leny / 4];

		for (int j = 0; j < heightmap_terrain->leny / 4; ++j)
		{
			(ptr_spheres[i] + j)->f_timeCreated = -1;
			(ptr_spheres[i] + j)->b_phere = false;
		}
	}

	posStart = convertPointFromPixelToWorldV2(6, 6);
	posStart.y += 3.5;
	posX = posStart.x;
	posY = posStart.y;
	posZ = posStart.z;
	posLast = posStart;

	b_init = true;

	return true;
}

point produit_vertoriel(point point1, point point2)
{
	point result;

	result.x = point1.y*point2.z - point1.z*point2.y;
	result.y = point1.z*point2.x - point1.x*point2.z;
	result.z = point1.x*point2.y - point1.y*point2.x;

	return result;
}

point convertPointFromPixelToWorld(int x, int y)
{
	point posInWorld = point(0, 0, 0);

	if (b_init == false)
	{
		posInWorld.x = (x - (int)heightmap_terrain->lenx / 2) * size1;
		posInWorld.y = size2*(float)heightmap_terrain->data[3 * (x + heightmap_terrain->lenx*y)];
		posInWorld.z = (y - (int)heightmap_terrain->leny / 2)*size1;
	}
	else
	{
		posInWorld.x = (x - (int)heightmap_terrain_reference->lenx / 2) * size1;
		posInWorld.y = size2*(float)heightmap_terrain_reference->data[3 * (x + heightmap_terrain_reference->lenx*y)];
		posInWorld.z = (y - (int)heightmap_terrain_reference->leny / 2)*size1;
	}
	return posInWorld;
}

point convertPointFromWorldToPixel(point posInWorld)
{
	point result;
	if (b_init == false)
	{
		result.x = (int)heightmap_terrain->lenx / 2 + (int)posInWorld.x / size1;
		result.y = (int)heightmap_terrain->leny / 2 + (int)posInWorld.z / size1;
		result.z = 0;
	}
	else
	{
		result.x = (int)heightmap_terrain_reference->lenx / 2 + (int)posInWorld.x / size1;
		result.y = (int)heightmap_terrain_reference->leny / 2 + (int)posInWorld.z / size1;
		result.z = 0;
	}
	return result;
}

float getHeightFromPixelToWorldByWorldPosition(point posInWorld)
{
	float heightInWorld;
	point posInPixel;
	posInPixel = convertPointFromWorldToPixel(posInWorld);

	if (b_init == false)
	{
		heightInWorld = size2*(float)heightmap_terrain->data[3 * ((int)posInPixel.x + heightmap_terrain->lenx*((int)posInPixel.y))];
	}	
	else
	{
		heightInWorld = size2*(float)heightmap_terrain_reference->data[3 * ((int)posInPixel.x + heightmap_terrain_reference->lenx*((int)posInPixel.y))];
	}
	return heightInWorld;
}

point convertPointFromWorldToPixelV2(point posInWorld)
{

	point test;

	point result;
	/*
	posInWorld.x += 0.5*size1;
	posInWorld.z += 0.5*size1;

	result.x = (int)(posInWorld.x / size1) * 4 - 2;
	result.y = (int)(posInWorld.z / size1) * 4 - 2;
	*/

	result.x = (int)(posInWorld.x / size1) * 4;
	result.y = (int)(posInWorld.z / size1) * 4;

	if ((posInWorld.x - ((int)(posInWorld.x/size1)) * size1) >= 0.75*size1)
	{
		result.x += 4;
	}
	else if ((posInWorld.x - ((int)(posInWorld.x / size1)) * size1) >= 0.5*size1)
	{
		result.x += 2;
	}
	else if ((posInWorld.x - ((int)(posInWorld.x / size1)) * size1) >= 0.25*size1)
	{
		result.x += 1;
	}

	if ((posInWorld.z - ((int)(posInWorld.z / size1)) * size1) >= 0.75*size1)
	{
		result.y += 4;
	}
	else if ((posInWorld.z - ((int)(posInWorld.z / size1)) * size1) >= 0.5*size1)
	{
		result.y += 2;
	}
	else if ((posInWorld.z - ((int)(posInWorld.z / size1)) * size1) >= 0.25*size1)
	{
		result.y += 1;
	}
    

	result.z = 0;

	return result;
}

point convertPointFromPixelToWorldV2(int x, int y)
{
	point posInWorld = point(0, 0, 0);

	posInWorld.x = (x / 4) * size1 + (x % 4) * 0.25 * size1;
	posInWorld.z = (y / 4) * size1 + (x % 4) * 0.25 * size1;

	if ((float)heightmap_terrain->data[3 * (x + heightmap_terrain->lenx*y)] > 125)
	{
	    posInWorld.y = size2;
	}
	else
	{
		posInWorld.y = 0;
	}

	return posInWorld;
}

float getHeightFromPixelToWorldByWorldPositionV2(point posInWorld)
{
	float heightInWorld;
	point posInPixel;

	posInPixel = convertPointFromWorldToPixelV2(posInWorld);

	

	if ((float)heightmap_terrain_reference->data[3 * ((int)posInPixel.x + heightmap_terrain->lenx*(int)posInPixel.y)] > 125)
	//if ((float)heightmap_terrain->data[3 * ((int)posInPixel.x + heightmap_terrain->lenx*(int)posInPixel.y)] > 125)
	{
		posInPixel = convertPointFromWorldToPixelV2(posInWorld);
		heightInWorld = size2;
	}
	else
	{
		heightInWorld = 0;
	}

	return heightInWorld;
}


void terrain_heightmap_terrain(float size, float size2)
{
	glGenTextures(1, texture_wall->OpenGL_ID);                // crée un "nom" de texture (un identifiant associ??la texture)
	glBindTexture(GL_TEXTURE_2D, texture_wall->OpenGL_ID[0]);    // et on active ce "nom" comme texture courante (définie plus bas)

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // on répète la texture en cas de U,V > 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // ou < 0.0

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // indique qu'il faut mélanger la texture avec la couleur courante

	// charge le tableau de la texture en mémoire vidéo et crée une texture mipmap
	if (texture_wall->isRGBA)// with alpha
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, texture_wall->img_color->lenx, texture_wall->img_color->leny, GL_RGBA, GL_UNSIGNED_BYTE, texture_wall->img_all);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texture_wall->img_color->lenx, texture_wall->img_color->leny, GL_RGB, GL_UNSIGNED_BYTE, texture_wall->img_color->data);// no alpha

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	//glTranslatef(0, -100, 0);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < (int)heightmap_terrain->lenx - 1; i++){
		for (int j = 0; j < (int)heightmap_terrain->leny - 1; j++){
			float I = i - (int)heightmap_terrain->lenx / 2;
			float J = j - (int)heightmap_terrain->leny / 2;

			glTexCoord2f((float)i / (float)heightmap_terrain->lenx, (float)j / (float)heightmap_terrain->leny);
			glVertex3f((I)*size, size2*(float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*(j))], (J)*size);

			glTexCoord2f((float)i / (float)heightmap_terrain->lenx, (float)(j + 1) / (float)heightmap_terrain->leny);
			glVertex3f((I)*size, size2*(float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*(j + 1))], (J + 1)*size);

			glTexCoord2f((float)(i + 1) / (float)heightmap_terrain->lenx, (float)(j) / (float)heightmap_terrain->leny);
			glVertex3f((I + 1)*size, size2*(float)heightmap_terrain->data[3 * (i + 1 + heightmap_terrain->lenx*(j))], (J)*size);

			glTexCoord2f((float)(i + 1) / (float)heightmap_terrain->lenx, (float)(j + 1) / (float)heightmap_terrain->leny);
			glVertex3f((I + 1)*size, size2*(float)heightmap_terrain->data[3 * (i + 1 + heightmap_terrain->lenx*(j + 1))], (J + 1)*size);

			glTexCoord2f((float)i / (float)heightmap_terrain->lenx, (float)(j + 1) / (float)heightmap_terrain->leny);
			glVertex3f((I)*size, size2*(float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*(j + 1))], (J + 1)*size);

			glTexCoord2f((float)(i + 1) / (float)heightmap_terrain->lenx, (float)(j) / (float)heightmap_terrain->leny);
			glVertex3f((I + 1)*size, size2*(float)heightmap_terrain->data[3 * (i + 1 + heightmap_terrain->lenx*(j))], (J)*size);
		}
	}
	glEnd();
	glPopMatrix();
}

void terrain_heightmap_terrainV2(float size, float size2)
{
	glGenTextures(1, texture_wall->OpenGL_ID);                // crée un "nom" de texture (un identifiant associ??la texture)
	glBindTexture(GL_TEXTURE_2D, texture_wall->OpenGL_ID[0]);    // et on active ce "nom" comme texture courante (définie plus bas)

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // on répète la texture en cas de U,V > 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // ou < 0.0

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // indique qu'il faut mélanger la texture avec la couleur courante

	// charge le tableau de la texture en mémoire vidéo et crée une texture mipmap
	if (texture_wall->isRGBA)// with alpha
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, texture_wall->img_color->lenx, texture_wall->img_color->leny, GL_RGBA, GL_UNSIGNED_BYTE, texture_wall->img_all);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texture_wall->img_color->lenx, texture_wall->img_color->leny, GL_RGB, GL_UNSIGNED_BYTE, texture_wall->img_color->data);// no alpha

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	for (int i = 0; i < (int)heightmap_terrain->lenx - 1; i++){
		if (i % 4 == 0)
		{ 
			for (int j = 0; j < (int)heightmap_terrain->leny - 1; j++){
				if (j % 4 == 0)
				{
					if ((float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*j)] > 125)
					{
						if (j-1 < 0 || (j-1 >= 0 && (float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*(j - 1))] < 125))
						{
							glBegin(GL_QUADS);
							glTexCoord2f(0, 0);
							glVertex3f((i / 4)*size1, 0, (j / 4)*size1);
							glTexCoord2f(1, 0);
							glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1);
							glTexCoord2f(1, 1);
							glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1);
							glTexCoord2f(0, 1);
							glVertex3f((i / 4)*size1, size2, (j / 4)*size1);
							glEnd();
						}

						if (i-1 < 0 || (i-1 >= 0 && (float)heightmap_terrain->data[3 * ((i - 1) + heightmap_terrain->lenx*j)] < 125))
						{
							glBegin(GL_QUADS);
							glTexCoord2f(0, 0);
							glVertex3f((i / 4)*size1, 0, (j / 4)*size1 + size1);
							glTexCoord2f(1, 0);
							glVertex3f((i / 4)*size1, 0, (j / 4)*size1);
							glTexCoord2f(1, 1);
							glVertex3f((i / 4)*size1, size2, (j / 4)*size1);
							glTexCoord2f(0, 1);
							glVertex3f((i / 4)*size1, size2, (j / 4)*size1 + size1);
							glEnd();
						}

						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1, size2, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1 + size1);

						glEnd();

						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1);
						glEnd();

						/*
						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1 + size1);
						glEnd();
						*/

						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1, size2, (j / 4)*size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1, size2, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, size2, (j / 4)*size1 + size1);
						glEnd();
					}
					else
					{/*
						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1 + size1);
						glEnd();
					 */
					}
				}
			}
		}
	}
	glPopMatrix();

	glGenTextures(1, texture_grassland->OpenGL_ID);                // crée un "nom" de texture (un identifiant associ??la texture)
	glBindTexture(GL_TEXTURE_2D, texture_grassland->OpenGL_ID[0]);    // et on active ce "nom" comme texture courante (définie plus bas)

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // on répète la texture en cas de U,V > 1.0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // ou < 0.0

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // indique qu'il faut mélanger la texture avec la couleur courante

	// charge le tableau de la texture en mémoire vidéo et crée une texture mipmap
	if (texture_grassland->isRGBA)// with alpha
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, texture_grassland->img_color->lenx, texture_grassland->img_color->leny, GL_RGBA, GL_UNSIGNED_BYTE, texture_grassland->img_all);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texture_grassland->img_color->lenx, texture_grassland->img_color->leny, GL_RGB, GL_UNSIGNED_BYTE, texture_grassland->img_color->data);// no alpha

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	for (int i = 0; i < (int)heightmap_terrain->lenx - 1; i++){
		if (i % 4 == 0)
		{
			for (int j = 0; j < (int)heightmap_terrain->leny - 1; j++){
				if (j % 4 == 0)
				{
					if ((float)heightmap_terrain->data[3 * (i + heightmap_terrain->lenx*j)] < 125)
					{
						glBegin(GL_QUADS);
						glTexCoord2f(0, 0);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 0);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1);
						glTexCoord2f(1, 1);
						glVertex3f((i / 4)*size1, 0, (j / 4)*size1 + size1);
						glTexCoord2f(0, 1);
						glVertex3f((i / 4)*size1 + size1, 0, (j / 4)*size1 + size1);
						glEnd();
					}
				}
			}
		}
	}
	glPopMatrix();
}

float distanceBetweenTwoPointsPlanXZ(point point1, point point2)
{
	float f_distance;
	point pointD;
	pointD.x = point1.x - point2.x;
	//pointD.y = point1.y - point2.y;
	pointD.z = point1.z - point2.z;

	f_distance = pointD.x * pointD.x + pointD.z * pointD.z;
	return f_distance;
}

void cube(int _i_x, int _i_y)
{
	//glGenTextures(1, texture_brick->OpenGL_ID);                // crée un "nom" de texture (un identifiant associ??la texture)
	//glBindTexture(GL_TEXTURE_2D, texture_brick->OpenGL_ID[0]);    // et on active ce "nom" comme texture courante (définie plus bas)

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // on répète la texture en cas de U,V > 1.0
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // ou < 0.0

	///glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // indique qu'il faut mélanger la texture avec la couleur courante

	// charge le tableau de la texture en mémoire vidéo et crée une texture mipmap
	//if (texture_brick->isRGBA)// with alpha
	//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, texture_brick->img_color->lenx, texture_brick->img_color->leny, GL_RGBA, GL_UNSIGNED_BYTE, texture_brick->img_all);
	//else
	//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texture_brick->img_color->lenx, texture_brick->img_color->leny, GL_RGB, GL_UNSIGNED_BYTE, texture_brick->img_color->data);// no alpha

	//glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(_i_x*size1, 0, _i_y*size1);
	for (int i = 0; i < 6; ++i)
	{
		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, 0);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(0, size2, 0);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(0, 0, size1);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(0, size2, 0);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(0, size2, size1);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, size1);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(0, 0, size1);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(0, size2, size1);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, size1);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, size1);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, size1);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, 0);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(0, 0, size1);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, 0, size1);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, 0);
		//glTexCoord2f(1, 0);
		glColor3f(1, 0, 0); glVertex3f(0, size2, 0);
		//glTexCoord2f(1, 1);
		glColor3f(1, 0, 0); glVertex3f(0, size2, size1);
		//glTexCoord2f(0, 1);
		glColor3f(1, 0, 0); glVertex3f(size1, size2, size1);
		glEnd();
		glColor3f(1, 1, 1);
	}
	glPopMatrix();
}

void cubeParticle(point center, float _f_size)
{
	//glGenTextures(1, texture_brick->OpenGL_ID);                // crée un "nom" de texture (un identifiant associ??la texture)
	//glBindTexture(GL_TEXTURE_2D, texture_brick->OpenGL_ID[0]);    // et on active ce "nom" comme texture courante (définie plus bas)

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // on répète la texture en cas de U,V > 1.0
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    // ou < 0.0

	///glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // indique qu'il faut mélanger la texture avec la couleur courante

	// charge le tableau de la texture en mémoire vidéo et crée une texture mipmap
	//if (texture_brick->isRGBA)// with alpha
	//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, texture_brick->img_color->lenx, texture_brick->img_color->leny, GL_RGBA, GL_UNSIGNED_BYTE, texture_brick->img_all);
	//else
	//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texture_brick->img_color->lenx, texture_brick->img_color->leny, GL_RGB, GL_UNSIGNED_BYTE, texture_brick->img_color->data);// no alpha

	//glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glTranslatef(-0.5*_f_size, -0.5*_f_size, -0.5*_f_size);
	for (int i = 0; i < 6; ++i)
	{
		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, 0);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, 0);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, _f_size);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, 0);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, _f_size);
		glEnd();
		
		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, _f_size);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, _f_size);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, _f_size);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, _f_size);
		glEnd();
		
		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, _f_size);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, _f_size);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, 0);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, 0);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(0, 0, 0);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(0, 0, _f_size);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, 0, _f_size);
		glEnd();

		glBegin(GL_QUADS);
		//glTexCoord2f(0, 0);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, 0);
		//glTexCoord2f(1, 0);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, 0);
		//glTexCoord2f(1, 1);
		glColor3f(0, 0, 1); glVertex3f(0, _f_size, _f_size);
		//glTexCoord2f(0, 1);
		glColor3f(0, 0, 1); glVertex3f(_f_size, _f_size, _f_size);
		glEnd();
		glColor3f(1, 1, 1);
	}
	glPopMatrix();
}

void sphere(int _i_x, int _i_y, double r, int lats, int longs) {

	glPushMatrix();
	glTranslatef(_i_x*size1+0.5*size1, 0.5*size2, _i_y*size1+0.5*size1);
	glScalef(0.5*size1, 0.5*size1, 0.5*size1);
     int i, j;
      for (i = 0; i <= lats; i++) {
          double lat0 = 3.1415926 * (-0.5 + (double)(i - 1) / lats);
          double z0 = sin(lat0);
          double zr0 = cos(lat0);

		  double lat1 = 3.1415926 * (-0.5 + (double)i / lats);
          double z1 = sin(lat1);
          double zr1 = cos(lat1);

         glBegin(GL_QUAD_STRIP);
          for (j = 0; j <= longs; j++) {
			  double lng = 2 * 3.1415926 * (double)(j - 1) / longs;
              double x = cos(lng);
              double y = sin(lng);

			  glColor3f(0, 0, 0); glNormal3f(x * zr0, y * zr0, z0);
			  glColor3f(0, 0, 0); glVertex3f(x * zr0, y * zr0, z0);
			  glColor3f(0, 0, 0); glNormal3f(x * zr1, y * zr1, z1);
			  glColor3f(0, 0, 0); glVertex3f(x * zr1, y * zr1, z1);
		}

		  glColor3f(1, 1, 1);
        glEnd();

	}
	 glPopMatrix();

}

void particleSystem(point position, float distance)
{
	point centerLitterCube;
	float distanceTwoLittleCube = 0.5;
	int i_countDistance = 0;

	while (i_countDistance * distanceTwoLittleCube <= distance+0.5*size1)
	{
		cubeParticle(position + point(i_countDistance * distanceTwoLittleCube, 0, 0), 0.2);
		cubeParticle(position + point(-i_countDistance * distanceTwoLittleCube, 0, 0), 0.2);
		cubeParticle(position + point(0, 0, i_countDistance * distanceTwoLittleCube), 0.2);
		cubeParticle(position + point(0, 0, -i_countDistance * distanceTwoLittleCube), 0.2);

		//cube(position + point(i_countDistance * distanceTwoLittleCube, 0, 0), 0.4);
		//cube(position + point(-i_countDistance * distanceTwoLittleCube, 0, 0), 0.4);
		//cube(position + point(0, 0, i_countDistance * distanceTwoLittleCube), 0.4);
		//cube(position + point(0, 0, -i_countDistance * distanceTwoLittleCube), 0.4);
		i_countDistance++;
	}

}

/********************************************************************\
*                                                                    * 
*  les formes                                               *
*                                                                    *
\********************************************************************/

/********************************************************************\
*                                                                    *
*  Boucle principale, appelée pour construire une image, gérer les   *
*  entrées.                                                          *
*                                                                    *
\********************************************************************/
void main_loop() 
{

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//				gestion des touches	et du temps													//
	//////////////////////////////////////////////////////////////////////////////////////////////////

	float f_heightJump = 0;

	inp->refresh();
	inp->get_mouse_movement();
	tim->update_horloge();
	f_timer = (float)tim->get_heure() * 3600 + (float)tim->get_minute() * 60 + (float)tim->get_seconde();

	if (inp->keys[KEY_CODE_ESCAPE]) 
	{	  
		PostMessage(win->handle,WM_CLOSE,0,0);	// Stoppe la "pompe ?message" en y envoyant le message "QUIT"
	}
	//if (inp->keys[KEY_CODE_SPACE])
	//{
	//	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	//}
	else
	{
		glClearColor(0.0f, 0.847059f, 1.0f, 1.0f);
	}

	/*
	if (tim->global_timer_25_Hz)				// augmente angleY tous les 20ème de seconde
        angleY += 1.0f;
	if (angleY >= 360) angleY -= 360;
	*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//						ça commence ici															//
	//////////////////////////////////////////////////////////////////////////////////////////////////

	glMatrixMode(GL_PROJECTION);  //la matrice de projection sera celle selectionnee
	//composition d'une matrice de projection
	glLoadIdentity(); //on choisit la matrice identit?
	gluPerspective(60, (double)win->Xres / (double)win->Yres, 0.1, 30000);   //mise en place d une proj angle de vue 60 deg near 10 far 30000
	terrain_heightmap_terrainV2(size1, size2);

	point dir = cam->direction - cam->position;

	if (inp->keys[KEY_CODE_SPACE])
	{
		f_heightJump = 5;
	}

	if (inp->keys[KEY_CODE_R])
	{
		posX += dir.x * 0.15;
		posZ += dir.z * 0.15;
		//posY += dir.y;
	}

	if (inp->keys[KEY_CODE_F])
	{
		posX -= dir.x * 0.15;
		posZ -= dir.z * 0.15;
		//posY -= dir.y;
	}
	if (inp->keys[KEY_CODE_G])
	{
		posX += point(produit_vertoriel(dir, cam->orientation)).x * 0.15;
		posZ += point(produit_vertoriel(dir, cam->orientation)).z * 0.15;
	}
	if (inp->keys[KEY_CODE_D])
	{
		posX -= point(produit_vertoriel(dir, cam->orientation)).x * 0.15;
		posZ -= point(produit_vertoriel(dir, cam->orientation)).z * 0.15;
	}

	if (posX > ((heightmap_terrain->lenx / 4)*size1 - size1*0.5))
		posX = (heightmap_terrain->lenx / 4)*size1 - size1*0.5;
	if (posZ > ((heightmap_terrain->leny / 4)*size1 - size1*0.5))
		posZ = (heightmap_terrain->leny / 4)*size1 - size1*0.5;
	if (posX < -size1*0.5)
		posX = -size1*0.5;
	if (posZ < -size1*0.5)
		posZ = -size1*0.5;


	if (angleY > 360) angleY -= 360;
	if (angleY < 0) angleY += 360;


	if (angleX > 60) angleX = 60;
	if (angleX < -60) angleX = -60;

	angleX -= 0.1*(float)inp->Yrelmouse;
	angleY -= 0.1*(float)inp->Xrelmouse;

	posY = getHeightFromPixelToWorldByWorldPositionV2(point(posX, posY, posZ));

	int i_xPlayerPredict = posX / size1;
	int i_zPlayerPredict = posZ / size1;

	if (posX >= 1.9*size1)
	{
		std::cout << std::endl;
	}

	if (
		(i_xPlayerPredict + 1 <= heightmap_terrain->lenx / 4 - 1) 
		&& (*(bPtr_bricks[i_xPlayerPredict + 1] + i_zPlayerPredict) == true 
		    || (ptr_spheres[i_xPlayerPredict + 1] + i_zPlayerPredict)->b_phere == true)
	    )
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict + 1)*size1 + 0.5*size1, posY, posZ))
			<= 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			float f = distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict + 1)*size1 + 0.5*size1, posY, i_zPlayerPredict*size1 + 0.5*size1));
			b_canMoveAtX = false;
		}
	}

	if (
		(i_xPlayerPredict - 1 >= 0) 
		&& (*(bPtr_bricks[i_xPlayerPredict - 1] + i_zPlayerPredict) == true
			|| (ptr_spheres[i_xPlayerPredict - 1] + i_zPlayerPredict)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict - 1)*size1 + 0.5*size1, posY, posZ))
			<= 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtX = false;
		}
	}

	if (
		(i_zPlayerPredict + 1 <= heightmap_terrain->leny / 4 - 1)
		&& (*(bPtr_bricks[i_xPlayerPredict] + i_zPlayerPredict + 1) == true
			|| (ptr_spheres[i_xPlayerPredict] + i_zPlayerPredict + 1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point(posX, posY, (i_zPlayerPredict + 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtZ = false;
		}
	}

	if (
		(i_zPlayerPredict - 1 >= 0) 
		&& (*(bPtr_bricks[i_xPlayerPredict] + i_zPlayerPredict - 1) == true
			|| (ptr_spheres[i_xPlayerPredict] + i_zPlayerPredict - 1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point(posX, posY, (i_zPlayerPredict - 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtZ = false;
		}
	}

	if (
		(i_xPlayerPredict + 1 <= heightmap_terrain->lenx / 4 - 1) && (i_zPlayerPredict + 1 <= heightmap_terrain->leny / 4 - 1)
		&& (*(bPtr_bricks[i_xPlayerPredict + 1] + i_zPlayerPredict + 1) == true
			|| (ptr_spheres[i_xPlayerPredict + 1] + i_zPlayerPredict + 1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict + 1)*size1 + 0.5*size1, posY, (i_zPlayerPredict + 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtX = false;
			b_canMoveAtZ = false;
		}
	}

	if (
		(i_xPlayerPredict + 1 <= heightmap_terrain->lenx / 4 - 1) && (i_zPlayerPredict - 1 >= 0)
		&& (*(bPtr_bricks[i_xPlayerPredict + 1] + i_zPlayerPredict - 1) == true
			|| (ptr_spheres[i_xPlayerPredict + 1] + i_zPlayerPredict -1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict + 1)*size1 + 0.5*size1, posY, (i_zPlayerPredict - 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtX = false;
			b_canMoveAtZ = false;
		}
	}

	if (
		(i_xPlayerPredict - 1 >= 0) && (i_zPlayerPredict + 1 <= heightmap_terrain->leny / 4 - 1)
		&& (*(bPtr_bricks[i_xPlayerPredict - 1] + i_zPlayerPredict + 1) == true
			|| (ptr_spheres[i_xPlayerPredict -1] + i_zPlayerPredict +1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict - 1)*size1 + 0.5*size1, posY, (i_zPlayerPredict + 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtX = false;
			b_canMoveAtZ = false;
		}
	}

	if (
		(i_xPlayerPredict - 1 >= 0) && (i_zPlayerPredict - 1 >= 0)
		&& (*(bPtr_bricks[i_xPlayerPredict - 1] + i_zPlayerPredict - 1) == true
			|| (ptr_spheres[i_xPlayerPredict - 1] + i_zPlayerPredict - 1)->b_phere == true)
		)
	{
		if (distanceBetweenTwoPointsPlanXZ(point(posX, posY, posZ), point((i_xPlayerPredict + 1)*size1 + 0.5*size1, posY, (i_zPlayerPredict + 1)*size1 + 0.5*size1))
			<= 0.5*size1 * 0.5*size1 + 0.5*size1 * 0.5*size1 + 0.25*size1)
		{
			b_canMoveAtX = false;
			b_canMoveAtZ = false;
		}
	}

	//if (posY > posStart.y + 2)
	if (posY > posStart.y)
	{
		if (getHeightFromPixelToWorldByWorldPositionV2(point(posLast.x, posLast.y, posZ)) > posStart.y)
		{
			b_canMoveAtZ = false;
		}
		if (getHeightFromPixelToWorldByWorldPositionV2(point(posX, posLast.y, posLast.z)) > posStart.y)
		{
			b_canMoveAtX = false;
		}
	}

	if (b_canMoveAtX == false)
	{
		posX = posLast.x;
	}
	if (b_canMoveAtZ == false)
	{
		posZ = posLast.z;
	}
	posY = posStart.y;
	cam->update(point(posX, posY + f_heightJump, posZ), angleX, angleY, angleZ);
	posLast = point(posX, posY, posZ);

	b_canMoveAtX = true;
	b_canMoveAtZ = true;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cam->position.x, cam->position.y, cam->position.z,
		cam->direction.x, cam->direction.y, cam->direction.z,
		cam->orientation.x, cam->orientation.y, cam->orientation.z);

	point directionCamera = cam->direction - cam->position;

	if (inp->keys[KEY_CODE_E])
	{
		if (f_timerCube == 0 || (f_timer - f_timerCube) >= 1)
		{
			int i_x = 0;
			int i_y = 0;

			i_x = cam->position.x / size1;
			i_y = cam->position.z / size1;	

			if (directionCamera.x *directionCamera.x >= directionCamera.z * directionCamera.z)
			{
				if (directionCamera.x > 0)
				{
					i_x++;
				}
				else if (directionCamera.x < 0)
				{
					i_x--;
				}
			}
			else
			{
				if (directionCamera.z > 0)
				{
					i_y++;
				}
				else if (directionCamera.z < 0)
				{
					i_y--;
				}
			}

			if (i_x >= 0 && i_x <= heightmap_terrain->lenx - 1
				&& i_y >= 0 && i_y <= heightmap_terrain->leny - 1
				&& heightmap_terrain->data[3 * (i_x*4 + heightmap_terrain->lenx*i_y*4)] < 125
				&& (ptr_spheres[i_x]+i_y)->b_phere == false)
			{
				*(bPtr_bricks[i_x] + i_y) = true;
			}

			f_timerCube = f_timer;
		}
	}
	if (inp->keys[KEY_CODE_T])
	{
		if (f_timerSphere == 0 || (f_timer - f_timerSphere) >= 1)
		{
			int i_x = 0;
			int i_y = 0;
			MySphere mySphere;
			
			i_x = cam->position.x / size1;
			i_y = cam->position.z / size1;

			if (directionCamera.x *directionCamera.x >= directionCamera.z * directionCamera.z)
			{
				if (directionCamera.x > 0)
				{
					i_x++;
				}
				else if (directionCamera.x < 0)
				{
					i_x--;
				}
			}
			else
			{
				if (directionCamera.z > 0)
				{
					i_y++;
				}
				else if (directionCamera.z < 0)
				{
					i_y--;
				}
			}

			if (i_x >= 0 && i_x <= heightmap_terrain->lenx - 1
				&& i_y >= 0 && i_y <= heightmap_terrain->leny - 1
				&& heightmap_terrain->data[3 * (i_x * 4 + heightmap_terrain->lenx*i_y * 4)] < 125
				&& *(bPtr_bricks[i_x]+i_y) == false)
			{
				(ptr_spheres[i_x] + i_y)->f_timeCreated = f_timer;
				(ptr_spheres[i_x] + i_y)->b_phere = true;
			}

			f_timerSphere = f_timer;
		}
	}

	for (int i = 0; i < heightmap_terrain->lenx / 4 - 1; ++i)
	{
		for (int j = 0; j < heightmap_terrain->leny / 4 - 1; ++j)
		{
			if ((ptr_spheres[i] + j)->b_phere == true && f_timer - (ptr_spheres[i] + j)->f_timeCreated > f_durationDestroySphere)
			{
				if (j + 1 <= heightmap_terrain->leny / 4 - 1 && *(bPtr_bricks[i] + j + 1) == true)
				{
					*(bPtr_bricks[i] + j + 1) = false;
				}

				if (j - 1 >= 0 && *(bPtr_bricks[i] + j - 1) == true)
				{
					*(bPtr_bricks[i] + j - 1) = false;
				}

				if (i + 1 <= heightmap_terrain->lenx / 4 - 1 && *(bPtr_bricks[i + 1] + j) == true)
				{
					*(bPtr_bricks[i + 1] + j) = false;
				}

				if (i - 1 > 0 && *(bPtr_bricks[i - 1] + j) == true)
				{
					*(bPtr_bricks[i - 1] + j) = false;
				}

				(ptr_spheres[i] + j)->f_timeCreated = -1;
				(ptr_spheres[i] + j)->b_phere = false;

				int i_xPlayerUpdate = cam->position.x / size1;
				int i_zPlayerUpdate = cam->position.z / size1;

				MyParticleSystem particleSystem;
				particleSystem.f_timeCreated = f_timer;
				particleSystem.centerCreated = point(i*size1+0.5*size1, 0.5*size2, j*size1+0.5*size1);
				vector_particleSystem.push_back(particleSystem);

				if ((i_xPlayerUpdate == i + 1 && i_zPlayerUpdate == j)
					|| (i_xPlayerUpdate == i - 1 && i_zPlayerUpdate == j)
					|| (i_xPlayerUpdate == i && i_zPlayerUpdate == j + 1)
					|| (i_xPlayerUpdate == i && i_zPlayerUpdate == j - 1))
				{
					debug("YOU DIED");
					PostMessage(win->handle, WM_CLOSE, 0, 0);	// Stoppe la "pompe ?message" en y envoyant le message "QUIT"
				}
			}
		}
	}

	for (int i = 0; i < heightmap_terrain->lenx/4; ++i)
	{
		for (int j = 0; j < heightmap_terrain->leny/4; ++j)
		{
			if (*(bPtr_bricks[i] + j) == true)
			{
				cube(i, j);
			}
		}
	}
	for (int i = 0; i < heightmap_terrain->lenx/4; ++i)
	{
		for (int j = 0; j < heightmap_terrain->leny/4; ++j)
		{
			if ((ptr_spheres[i] + j)->b_phere == true)
			{
				//std::cout << std::endl;
				sphere(i, j, 10, 10, 10);
			}
		}
	}

	for (int i = 0; i < vector_particleSystem.size(); i++)
	{
		if ((f_timer - vector_particleSystem[i].f_timeCreated) <= f_durationParticle)
		{
			particleSystem(vector_particleSystem[i].centerCreated, size1);
		}
		else
		{
			vector_particleSystem.erase(vector_particleSystem.begin()+i);
			i--;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//						ça finit ici															//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	swap_buffer(win);	// affiche l'image composée ?l'écran
}


/********************************************************************\
*                                                                    *
* Arrête l'application                                               *
*                                                                    *
\********************************************************************/
void stop()
{
	delete inp;
	delete tim;

	if (win)
	{
		kill_font();
		kill_context(*win);
		delete win;
	}
}


/********************************************************************\
*                                                                    *
* Point d'entrée de notre programme pour Windows ("WIN32")           *
*                                                                    *
\********************************************************************/
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR szCmdLine,int iCmdShow)
{
	MSG  msg;

	win	= NULL;
	inp	= NULL;
	tim = NULL;

	if (start() == false)								// initialise la fenêtre et OpenGL
	{
		debug("start() : échec !");
		stop();
		return 1;
	}

	// main loop //
	// --------- //
	//   __
	//  /  \_
	//  |  \/ 
	//  \__/

	while (true)
	{
		if (PeekMessage(&msg, NULL,0,0,PM_NOREMOVE))	// s'il y a un message, appelle WndProc() pour le traiter
		{
			if (!GetMessage(&msg,NULL,0,0))				// "pompe ?message"
				break;
			TranslateMessage(&msg);
			DispatchMessage (&msg);
		}
		else 
		{
			main_loop();								// sinon, appelle main_loop()
		}
	}


	stop();												// arrête OpenGL et ferme la fenêtre

	return 0;
}



/********************************************************************\
*                                                                    *
*  Boucle des messages                                               *
*                                                                    *
\********************************************************************/
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

  switch (msg)
    {

      case WM_MOVE:        win->Xpos = (dword) LOWORD(lParam);
                           win->Ypos = (dword) HIWORD(lParam);
                           return 0;

      case WM_CLOSE:       PostQuitMessage(0); // dit ?GetMessage() de renvoyer 0
                           return 0;

      case WM_SYSCOMMAND:  // évite l'économiseur d'écran
                           switch (wParam)
                             {
                               case SC_SCREENSAVE:
                               case SC_MONITORPOWER:
                               return 0;
                             }
                           break;
/*
      case WM_CHAR:        la touche est traduite dans ce msg 
                           return 0;
*/

      case WM_KEYDOWN:     inp->set_key_down ((dword)wParam);
                           return 0;

      case WM_KEYUP:       inp->set_key_up   ((dword)wParam);
                           return 0;

      case WM_LBUTTONDOWN: inp->set_mouse_left_down();
                           return 0;

      case WM_LBUTTONUP:   inp->set_mouse_left_up();
                           return 0;

      case WM_RBUTTONDOWN: inp->set_mouse_right_down();
                           return 0;

      case WM_RBUTTONUP:   inp->set_mouse_right_up();
                           return 0;

      case WM_MBUTTONDOWN: inp->set_mouse_middle_down();
                           return 0;

      case WM_MBUTTONUP:   inp->set_mouse_middle_up();
                           return 0;

   }

  return DefWindowProc(hwnd,msg,wParam,lParam);
}
