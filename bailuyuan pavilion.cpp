#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace std;
#define GLUT_WHEEL_UP 3
#define GLUT_WHEEL_DOWN 4
#define root_3 1.732
#define sin_45 0.707

// arrays to save different textures
GLuint textures[20];	// array to saving the id of textures
GLubyte* wood1;
GLubyte* wood2;
GLubyte* stone1;
GLubyte* stone2;
GLubyte* sea;
GLubyte* sand;
GLubyte* grass;
GLubyte* words;
GLubyte* sky_data[5];
GLubyte* night_data[5];
GLubyte* wood3;
GLubyte* grass2;

bool tree_state = 1;
float degree = 485;	// view rotate degree
int sea_wave = 0;	// movement of spindrift
GLfloat cloud_configure[20][6];
GLfloat tree_wave = 0;	// wave degree of tree
int day_state = 0;
GLfloat walk_distance = 4;	// move speed

const int GL_MULTISAMPLE = 0x809D;
GLfloat camera_x = 0.0, camera_y = 170.0, camera_z = 1100.86; // viewing co-ordinate origin
GLfloat lookat_x = 0.0, lookat_y = 250.0, lookat_z = 0.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0; // view-up vector
GLfloat fltFOV = 90.0;
GLint winWidth = 700, winHeight = 500;

struct point1 {
	GLfloat x, y, z;
};

//Cuboid
struct Vertex {
	GLfloat position[3];
	GLfloat texCoord[2];
};

//cloud
struct CloudSphere {
	GLfloat pos_x;
	GLfloat pos_y;
	GLfloat pos_z;
	GLfloat size;
};

struct SpotLight {
	GLfloat position[4];
	GLfloat direction[3];
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat spotExponent;
	GLfloat spotCutoff;
	GLfloat constantAttenuation;
	GLfloat linearAttenuation;
};

struct LampProperties {
	GLfloat posi_x;
	GLfloat y_loc;
	GLfloat posi_z;
	int day_state;
};

struct RoadElement {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat width;
	GLfloat height;
	GLfloat length;
};

struct ChairPart {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat width;
	GLfloat height;
	GLfloat length;
	GLuint textureIndex;
};

struct StreetLightPart {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat width;
	GLfloat height;
	GLfloat length;
	bool isEmissionOn;
};

typedef struct {
	GLfloat vertices[4][3];
	GLfloat texCoords[4][2];
} ObjectData, SeaQuadData;

struct SkyFaceData {
	GLfloat texCoords[8][2];
	GLfloat vertices[8][3];
};

struct CeilingLayerParams {
	GLfloat height;
	GLfloat len;
	GLfloat len2;
};

struct LeafLevel {
	GLfloat colorR;
	GLfloat colorG;
	GLfloat colorB;
	GLfloat rotateAngle;
	GLfloat cuboidXOffset;
	GLfloat cuboidYFunctionCoefficient;
	GLfloat cuboidZ;
	GLfloat cuboidWidth;
	GLfloat cuboidHeight;
	GLfloat cuboidDepth;
};

typedef struct {
	float x, y, z, sizeX, sizeY, sizeZ;
} Shape;

typedef struct {
	float x, y, z;
	float width, height, depth;
} CuboidParams;
void myIdleFunc(void)
{

	// branches sway in the wind
	if (tree_wave >= 5) tree_state = 0;
	else if (tree_wave <= -5) tree_state = 1;
	tree_state == 1 ? tree_wave += 0.3 : tree_wave -= 0.5;
	sea_wave += 6;
	// the clouds floating
	for (int i = 0; i < 10; i++)
	{
		cloud_configure[i][0] += cloud_configure[i][4];
		if (cloud_configure[i][0] >= 2650)
			cloud_configure[i][0] = -2650;
	}
	glutPostRedisplay();	// redisplay
}

void Keyboards_input(unsigned char key, int x, int y)
{
	lookat_x = 220 * sin(degree / 200) + camera_x;
	lookat_z = 220 * cos(degree / 200) + camera_z;
	switch (key) {
	case 'q': case 'Q': exit(0); break;
	case 'w': case 'W': lookat_y += 15; break;
	case 's': case 'S': lookat_y -= 15; break;
	case 'a': case 'A': degree += 15; break;
	case 'd': case 'D': degree -= 15; break;
	//day_state = 0 : morning
	//day_state = 1 : afternoon (evening?)
	//day_state = 2 : evening (night?)
	case 't': case 'T': day_state = (day_state + 1) % 3; break;
	case 'm': case 'M': walk_distance += 4;; break;
	}
	if (key == 'm' && walk_distance >= 30) {
		for (walk_distance = 30; walk_distance > 0; walk_distance--) 
			walk_distance -= 4;
		
	}
}




bool move_restriction(GLfloat pos_x, GLfloat pos_z) {
	// Behave like an unseen air wall.
	return !(
		(pos_z > 3200 || pos_z < -620) ||
		(pos_x > 1475 || pos_x < -1175) ||
		(abs(pos_x) > 69 && 655 < pos_z && pos_z < 1620) ||
		(abs(pos_x) > 370 && pos_z < 1590 && ((abs(pos_x) - 350) * root_3 >= 600 - abs(pos_z)))
		);
}
void mySpecialKeys(int key, int xx, int yy) {
	// Record the position of the camera and observation point before moving
	GLfloat ori_camera_x = camera_x;
	GLfloat ori_camera_z = camera_z;
	GLfloat ori_lookat_x = lookat_x;
	GLfloat ori_lookat_z = lookat_z;

	GLfloat dx = 0.0;
	GLfloat dz = 0.0;

	switch (key) {
	case GLUT_KEY_RIGHT:
		dx = -walk_distance * cos(degree / 200);
		dz = walk_distance * sin(degree / 200);
		break;
	case GLUT_KEY_LEFT:
		dx = walk_distance * cos(degree / 200);
		dz = -walk_distance * sin(degree / 200);
		break;
	case GLUT_KEY_DOWN:
		dx = -walk_distance * sin(degree / 200);
		dz = -walk_distance * cos(degree / 200);
		break;
	case GLUT_KEY_UP:
		dx = walk_distance * sin(degree / 200);
		dz = walk_distance * cos(degree / 200);
		break;
	}
	camera_x += dx;
	camera_z += dz;
	lookat_x += dx;
	lookat_z += dz;

	// Determine whether the movement is restricted, and if so, return to the position before the movement
	if (!move_restriction(camera_x, camera_z)) {
		camera_x = ori_camera_x;
		camera_z = ori_camera_z;
		lookat_x = ori_lookat_x;
		lookat_z = ori_lookat_z;
	}
}
// mouse interaction
void mouseInput(int button, int state, int x, int y) {
	if (button == GLUT_WHEEL_UP && state == GLUT_UP) {	// Ascend upwards by scrolling the glut wheel up.
		if (camera_y < 1200)
			camera_y += walk_distance;
		lookat_y += walk_distance;
	}
	else if (button == GLUT_WHEEL_DOWN && state == GLUT_UP) {	// Descending downwards by scrolling the glut wheel down.
		if (camera_y > 100)
			camera_y -= walk_distance;
		lookat_y -= walk_distance;
	}
}
// load a image as a texture
GLubyte* loadImg(string filename) {
	GLint image_width;
	GLint image_height;
	GLint pixel_len;
	GLubyte* data;
	// Read in and open an image file
	FILE* pfile = NULL;
	fopen_s(&pfile, filename.c_str(), "rb"); // read the image in binary mode
	if (pfile == 0) exit(0);	// fail to read
	fseek(pfile, 0x0012, SEEK_SET);
	fread(&image_width, sizeof(image_width), 1, pfile);
	fread(&image_height, sizeof(image_height), 1, pfile);
	pixel_len = image_width * 3;
	while (pixel_len % 4 != 0)
		pixel_len++;
	pixel_len *= image_height;		// total size to save the image

	data = (GLubyte*)malloc(pixel_len);	// memory allocation
	if (data == 0)
		exit(0);	// read fail
	fseek(pfile, 54, SEEK_SET);
	fread(data, pixel_len, 1, pfile);
	fclose(pfile);
	for (int i = 0; i < pixel_len / 3; i++) {		// the original order of color information from the file is BGR, change the order to RGB
		GLubyte x = data[i * 3];
		GLubyte z = data[i * 3 + 2];
		data[i * 3 + 2] = x;
		data[i * 3] = z;
	}
	return data;	// save the texture information in data and return
}
// save a position in rhe point structure
point1 toPoint(GLfloat x, GLfloat y, GLfloat z) {
	point1 p;
	p.x = x, p.y = y, p.z = z;
	return p;
}
// Lighting setup: Create spot lights to mimic sunlight
void spotingLight(GLfloat spot_x, GLfloat spot_y, GLfloat spot_z) {
	// Define spotlight properties in different states
	SpotLight daySpotLight = {
		{spot_x, spot_y, spot_z, 1.0},
		{-0.01, -1.0, 0.0},
		{1.0, 1.0, 0.1, 1.0},
		{1.0, 1.0, 0.4, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		10,
		23,
		0.3,
		0.0
	};

	SpotLight afternoonSpotLight = {
		{spot_x, spot_y, spot_z, 1.0},
		{-0.8, -0.5, 1.45},
		{1.0, 1.0, 0.1, 1.0},
		{1.0, 1.0, 0.4, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		10,
		16,
		0.0,
		0.0005
	};

	// Select the corresponding spotlight attribute according to the day_state and set the light
	SpotLight* currentSpotLight;
	if (day_state == 0) {
		currentSpotLight = &daySpotLight;
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, currentSpotLight->position);
		glLightfv(GL_LIGHT0, GL_AMBIENT, currentSpotLight->ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, currentSpotLight->diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, currentSpotLight->specular);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, currentSpotLight->direction);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, currentSpotLight->spotExponent);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, currentSpotLight->spotCutoff);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, currentSpotLight->linearAttenuation);
	}
	else if (day_state == 1) {
		currentSpotLight = &afternoonSpotLight;
		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_POSITION, currentSpotLight->position);
		glLightfv(GL_LIGHT1, GL_AMBIENT, currentSpotLight->ambient);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, currentSpotLight->diffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, currentSpotLight->specular);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, currentSpotLight->direction);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, currentSpotLight->spotExponent);
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, currentSpotLight->spotCutoff);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, currentSpotLight->linearAttenuation);
	}


}
////create point lights as a wider range of sunlight
void point_Light(GLfloat pos_x, GLfloat pos_y, GLfloat pos_z) {
	GLfloat light_position[] = { pos_x, pos_y, pos_z, 1.0 };

	struct LightProperties {
		GLfloat ambient[4];
		GLfloat diffuse[4];
		GLfloat specular[4];
		GLfloat constant_attenuation;
	};
	// Daytime lighting properties
	LightProperties dayLight = {
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		7.0
	};
	// Afternoon lighting properties
	LightProperties afternoonLight = {
		{1.0, 1.0, 0.6, 1.0},
		{1.0, 1.0, 0.9, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		10.0
	};
	// Night light properties
	LightProperties nightLight = {
		{166 / 255.0, 73 / 255.0, 219 / 255.0, 1.0},
		{219 / 255.0, 73 / 255.0, 165 / 255.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		2.0
	};
	// Set light properties and enable lights according to different day_state
	LightProperties* currentLight;
	if (day_state == 0) {
		currentLight = &dayLight;
	}
	else if (day_state == 1) {
		currentLight = &afternoonLight;
	}
	else {
		currentLight = &nightLight;
	}
	glEnable(GL_LIGHT2);
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, currentLight->constant_attenuation);
	glLightfv(GL_LIGHT2, GL_AMBIENT, currentLight->ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, currentLight->diffuse);
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
	glLightfv(GL_LIGHT2, GL_SPECULAR, currentLight->specular);
}
// adjust the overall brightness
void parallelingLights() {
	// Determine the light intensity coefficient based on day/night conditions
	GLfloat intensityFactor = (day_state == 0) ? 1.0 : 0.3;

	GLfloat light_ambient[] = { 0.5 * intensityFactor, 0.5 * intensityFactor, 0.5 * intensityFactor, 1.0 };
	GLfloat light_diffuse[] = { 0.5 * intensityFactor, 0.5 * intensityFactor, 0.5 * intensityFactor, 1.0 };
	GLfloat light_specular[] = { 0.5 * intensityFactor, 0.5 * intensityFactor, 0.5 * intensityFactor, 1.0 };
	GLfloat lightDirection[] = { 0.0, 1.0, 1.0, 0.0 };

	// set light properties
	glLightfv(GL_LIGHT3, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT3, GL_POSITION, lightDirection);

	if (day_state == 0)
		glEnable(GL_LIGHT3);
	else
		glDisable(GL_LIGHT3);
}

// Basic shapes: Draw a hexagon with a specified thickness
void Hexagon(GLfloat cen_x, GLfloat cen_y, GLfloat cen_z, GLfloat len, GLfloat thickness, GLfloat tex_size) {
	// Define the points for each vertex of the hexagon
	point1 points[6];
	GLfloat root_3_value = std::sqrt(3);
	points[0] = { cen_x - len / 2, cen_y + thickness, cen_z + len / 2 * root_3_value };
	points[1] = { cen_x + len / 2, cen_y + thickness, cen_z + len / 2 * root_3_value };
	points[2] = { cen_x + len, cen_y + thickness, cen_z };
	points[3] = { cen_x + len / 2, cen_y + thickness, cen_z - len / 2 * root_3_value };
	points[4] = { cen_x - len / 2, cen_y + thickness, cen_z - len / 2 * root_3_value };
	points[5] = { cen_x - len, cen_y + thickness, cen_z };

	//  Draw the hexagon surface
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(points[0].x, points[0].y, points[0].z);
	glTexCoord2f(0.0, tex_size); glVertex3f(points[1].x, points[1].y, points[1].z);
	glTexCoord2f(0.5, tex_size); glVertex3f(points[2].x, points[2].y, points[2].z);
	glTexCoord2f(1.0, tex_size); glVertex3f(points[3].x, points[3].y, points[3].z);
	glTexCoord2f(1.0, 0.0); glVertex3f(points[4].x, points[4].y, points[4].z);
	glTexCoord2f(0.5, 0.0); glVertex3f(points[5].x, points[5].y, points[5].z);
	glEnd();

	// Draw the side surfaces of the hexagon
	glBegin(GL_QUADS);
	for (int i = 0; i < 6; i++) {
		glTexCoord2f(0.0, 0.0); glVertex3f(points[i].x, points[i].y, points[i].z);
		glTexCoord2f(0.0, 1.0); glVertex3f(points[(i + 1) % 6].x, points[(i + 1) % 6].y, points[(i + 1) % 6].z);
		glTexCoord2f(1.0, 1.0); glVertex3f(points[(i + 1) % 6].x, points[(i + 1) % 6].y - 2 * thickness, points[(i + 1) % 6].z);
		glTexCoord2f(1.0, 0.0); glVertex3f(points[i].x, points[i].y - 2 * thickness, points[i].z);
	}
	glEnd();
}
//
void Cuboid(GLfloat cen_x, GLfloat cen_y, GLfloat cen_z, GLfloat len_x, GLfloat len_y, GLfloat len_z) {
	GLfloat x_start = cen_x - len_x;
	GLfloat x_end = cen_x + len_x;
	GLfloat y_start = cen_y - len_y;
	GLfloat y_end = cen_y + len_y;
	GLfloat z_start = cen_z - len_z;
	GLfloat z_end = cen_z + len_z;
	//Define an array of vertex information and texture coordinate information for each surface
	Vertex frontFace[] = {
		{ {x_start, y_start, z_end}, {0.0, 0.0} },
		{ {x_start, y_start, z_start}, {0.0, 1.0} },
		{ {x_end, y_start, z_start}, {1.0, 1.0} },
		{ {x_end, y_start, z_end}, {1.0, 0.0} }
	};
	Vertex backFace[] = {
		{ {x_start, y_end, z_end}, {0.0, 0.0} },
		{ {x_start, y_end, z_start}, {0.0, 1.0} },
		{ {x_end, y_end, z_start}, {1.0, 1.0} },
		{ {x_end, y_end, z_end}, {1.0, 0.0} }
	};
	Vertex leftFace[] = {
		{ {x_start, y_start, z_end}, {0.0, 0.0} },
		{ {x_start, y_start, z_start}, {0.0, 1.0} },
		{ {x_start, y_end, z_start}, {1.0, 1.0} },
		{ {x_start, y_end, z_end}, {1.0, 0.0} }
	};
	Vertex rightFace[] = {
		{ {x_end, y_start, z_end}, {0.0, 0.0} },
		{ {x_end, y_start, z_start}, {0.0, 1.0} },
		{ {x_end, y_end, z_start}, {1.0, 1.0} },
		{ {x_end, y_end, z_end}, {1.0, 0.0} }
	};
	Vertex topFace[] = {
		{ {x_start, y_start, z_end}, {0.0, 0.0} },
		{ {x_start, y_end, z_end}, {0.0, 1.0} },
		{ {x_end, y_end, z_end}, {1.0, 1.0} },
		{ {x_end, y_start, z_end}, {1.0, 0.0} }
	};
	Vertex bottomFace[] = {
		{ {x_start, y_start, z_start}, {0.0, 0.0} },
		{ {x_start, y_end, z_start}, {0.0, 1.0} },
		{ {x_end, y_end, z_start}, {1.0, 1.0} },
		{ {x_end, y_start, z_start}, {1.0, 0.0} }
	};
	glBegin(GL_QUADS);
	// front
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(frontFace[i].texCoord[0], frontFace[i].texCoord[1]);
		glVertex3f(frontFace[i].position[0], frontFace[i].position[1], frontFace[i].position[2]);
	}
	// back
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(backFace[i].texCoord[0], backFace[i].texCoord[1]);
		glVertex3f(backFace[i].position[0], backFace[i].position[1], backFace[i].position[2]);
	}
	// left
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(leftFace[i].texCoord[0], leftFace[i].texCoord[1]);
		glVertex3f(leftFace[i].position[0], leftFace[i].position[1], leftFace[i].position[2]);
	}
	// right
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(rightFace[i].texCoord[0], rightFace[i].texCoord[1]);
		glVertex3f(rightFace[i].position[0], rightFace[i].position[1], rightFace[i].position[2]);
	}
	// top
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(topFace[i].texCoord[0], topFace[i].texCoord[1]);
		glVertex3f(topFace[i].position[0], topFace[i].position[1], topFace[i].position[2]);
	}

	// bottom
	for (int i = 0; i < 4; ++i) {
		glTexCoord2f(bottomFace[i].texCoord[0], bottomFace[i].texCoord[1]);
		glVertex3f(bottomFace[i].position[0], bottomFace[i].position[1], bottomFace[i].position[2]);
	}

	glEnd();
}

// Draw a cylinder where the top and bottom have the same diameter
void Cylinder(GLfloat xPos, GLfloat yPos, GLfloat zPos, GLfloat dia1, GLfloat dia2, GLfloat height, GLfloat rotateAng, bool withSpheres) {
	GLUquadricObj* objCylinder = gluNewQuadric();
	gluQuadricTexture(objCylinder, GL_TRUE); // Bind texture to the cylinder

	glPushMatrix();
	glTranslatef(xPos, yPos, zPos); // Set the position of the cylinder

	glPushMatrix();
	glRotatef(rotateAng, 1.0f, 0.0f, 0.0f);// Rotate the cylinder around the x-axis

	if (dia2 == 0) {
		gluCylinder(objCylinder, dia1, dia1, height, 32, 5);

		if (withSpheres) { // If needed, draw two spheres as the top and bottom of the cylinder
			glutSolidSphere(dia1, 20, 20);
			glPushMatrix();
			glTranslatef(0, 0, height);
			glutSolidSphere(dia1, 20, 20);
			glPopMatrix();
		}
	}
	else {
		gluCylinder(objCylinder, dia1, dia2, height, 32, 5);
	}

	glPopMatrix();
	glPopMatrix();
}

void WoodGuard(GLfloat cen_x, GLfloat cen_y, GLfloat cen_z, GLfloat len_x, GLfloat len_y, GLfloat len_z) {
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	// beginning pillor
	Cylinder(cen_x, cen_y, cen_z, len_x, 0, len_y, 90, true);
	// end pillor
	Cylinder(cen_x, cen_y, cen_z + len_z, len_x, 0, len_y, 90, true);

	// Intermediate horizontal strut
	GLfloat middleHorizontalHeights[] = { cen_y - len_y + 12, cen_y - 38, cen_y - 18 };
	GLfloat middleHorizontalLenXs[] = { len_x - 2, len_x - 2, len_x };
	GLfloat middleHorizontalLenZs[] = { len_z - 5, len_z - 5, len_z - 5 };
	for (int i = 0; i < 3; ++i) {
		Cylinder(cen_x, middleHorizontalHeights[i], cen_z, middleHorizontalLenXs[i], 0, middleHorizontalLenZs[i], 0, true);
	}

	// Intermediate vertical strut
	for (int i = 1; i < 7; i++) {
		Cylinder(cen_x, cen_y - 38, cen_z + len_z / 7 * i, len_x - 2, 0, len_y - 50, 90, true);
	}
}

// Determine the positions of the bridge and the guards
void bridge() {
	// using wood texture
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	// draw the bridge floor
	for (int i = 0; i < 10; i++)
		Cuboid(0, 10, 664 + 88 * i, 70, 10, 42);
	// draw holders of the bridge
	Cylinder(60, 10, 1100, 10, 0, 300, 90, false);
	Cylinder(-60, 10, 1000, 10, 0, 300, 90, false);

	// guard bars on the bridge
	GLfloat length = 100;
	for (int i = 0; i < 9; i++) {
		WoodGuard(-65, 120, 1400 - length * i, 5, 100, length);
		WoodGuard(65, 120, 1400 - length * i, 5, 100, length);
	}
	// guard bars around the pavilion
	// normal side
	for (int j = 1; j < 6; j++) {
		glPushMatrix();
		glRotatef(90 + j * 60, 0, 1, 0);
		for (int i = 0; i < 7; i++)
			WoodGuard(-350 * root_3, 120, 250 - length * i, 5, 100, length);
		glPopMatrix();
	}
	// side connect to the bridge (have a gap)
	GLfloat length2 = 95;
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	for (int i = 0; i < 3; i++)
		WoodGuard(-350 * root_3, 120, 250 - length2 * i, 5, 100, length2);
	for (int i = 4; i < 7; i++)
		WoodGuard(-350 * root_3, 120, 250 - length2 * i - 20, 5, 100, length2);
	glPopMatrix();
}

void drawLamp(LampProperties lampProps) {
	glDisable(GL_TEXTURE_2D);
	GLfloat emmission[] = { 0.7, 0.5, 0.3 };
	GLfloat no_emmission[] = { 0.0, 0.0, 0.0 };

	lampProps.day_state == 2 ? glMaterialfv(GL_FRONT, GL_EMISSION, emmission) : glMaterialfv(GL_FRONT, GL_EMISSION, no_emmission);

	glPushMatrix();
	glTranslatef(lampProps.posi_x, lampProps.y_loc, lampProps.posi_z);
	glColor3f(105 / 255.0, 77 / 255.0, 36 / 255.0);
	Cylinder(0, 0, 0, 15, 0, 40, 90, true);  // The lamp's body capable of giving off light.
	glMaterialfv(GL_FRONT, GL_EMISSION, no_emmission);  // other part can't shine
	// Tassels beneath the lamp.
	Cylinder(1, -54, 0, 1, 0, 22, 90, false);
	Cylinder(0, -54, 0, 1, 0, 20, 90, false);
	Cylinder(1, -54, 1, 1, 0, 17, 90, false);
	// top and bottom frame of the lamp
	glColor3f(0, 0, 0);
	Cylinder(0, 14, 0, 10, 0, 4, 90, false);
	Cylinder(0, 100, 0, 1, 0, 90, 90, false);


	glColor3f(1, 1, 1);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

//  drawCeilingFloor
void drawCeilingFloor(CeilingLayerParams layerParams, int numIterations, GLfloat stick_num, GLfloat cylinderRadius, GLfloat angle) {
	for (int i = 0; i < numIterations; i++) {
		Cylinder(19 * i, layerParams.height - stick_num * i, stick_num * i, cylinderRadius, 0, layerParams.len - stick_num / sin_45 * i, angle, false);
		Cylinder(-19 * i, layerParams.height - stick_num * i, stick_num * i, cylinderRadius, 0, layerParams.len - stick_num / sin_45 * i, angle, false);
	}
}

//  drawCeilingAngledFrame
void drawCeilingAngledFrame(CeilingLayerParams layerParams, GLfloat frameRadius, GLfloat frameLength, GLfloat angle) {
	glPushMatrix();
	glRotatef(30, 0, 1, 0);
	Cylinder(0, layerParams.height, 0, frameRadius, 0, layerParams.len * 1.12, angle, true);
	Cylinder(0, layerParams.height, 0, frameRadius, 0, layerParams.len2 * 1.12, angle, true);
	glPopMatrix();
}

void drawCeiling() {
	CeilingLayerParams layer1 = { 1100, 600, 270 };
	CeilingLayerParams layer2 = { 900, 700, 280 };
	GLfloat stick_num = 32;
	GLfloat cylinderRadius = 10;

	// wood1 texture
	glBindTexture(GL_TEXTURE_2D, textures[18]);
	for (int j = 0; j < 6; j++) {
		glPushMatrix();
		glRotatef(60 * j, 0, 1, 0);
		// the first floor
		drawCeilingFloor(layer1, 13, stick_num, cylinderRadius, 45);
		// the second floor
		drawCeilingFloor(layer2, 15, stick_num, cylinderRadius, 45);
		glPopMatrix();
	}

	// wood2 texture
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	for (int j = 0; j < 6; j++) {
		glPushMatrix();
		glRotatef(60 * j, 0, 1, 0);
		// AngledFrame
		drawCeilingAngledFrame(layer1, 15, layer1.len * 1.12, 40);
		drawCeilingAngledFrame(layer2, 15, layer2.len * 1.12, 40);
		glPopMatrix();
	}

	Cylinder(0, layer1.height + 100, 0, 35, 0, 120, 90, true);

	// stone texture
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	Hexagon(0, layer1.height - 460, 0, 350, 100, 3);
	Hexagon(0, 23, 0, 520, 3, 3);
}



// drawBase
void drawBase(Shape base) {
	Hexagon(base.x, base.y, base.z, base.sizeX, base.sizeY, base.sizeZ);
}

// SeatsSurface
void drawSeats(Shape seat, int count) {
	for (int i = 0; i < count; i++) {
		glPushMatrix();
		glRotatef(60 * i, 0, 1, 0);
		for (int j = 0; j < 6; j++) {
			seat.z = 450 / 2 * sqrt(3) - 15 + j * 10;
			Cuboid(seat.x, seat.y, seat.z, seat.sizeX, seat.sizeY, seat.sizeZ);
		}
		glPopMatrix();
	}
}


// StoneTablets
void drawStoneTablets() {
	Cuboid(0, 30, 0, 60, 10, 25);
	Cuboid(0, 170, 0, 40, 130, 10);
}

// drawSeatBase
void drawSeatBases(Shape base, int count) {
	for (int i = 0; i < count; i++) {
		glPushMatrix();
		glRotatef(60 * i, 0, 1, 0);
		Cuboid(base.x, base.y, base.z, base.sizeX, base.sizeY, base.sizeZ);
		for (int j = 1; j < 4; j++) {
			Cuboid(-300 + j * 600 / 4, 32, base.z, 10, 32, 20);
		}
		glPopMatrix();
	}
}

// drawPanel
void drawPanel() {
	Cuboid(0, 675, 175 * sqrt(3), 150, 55, 5);
}
void Pavilion() {
	drawCeiling(); // Ceiling
//	//Place lamps on each of its corners.
	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glRotatef(i * 120, 0, 1, 0);

		LampProperties lamp1 = { 270, 315, 270 * sqrt(3) };
		LampProperties lamp2 = { -270, 315, 270 * sqrt(3) };
		LampProperties lamp3 = { 190, 355, 245 * sqrt(3) };
		LampProperties lamp4 = { -190, 355, 245 * sqrt(3) };

		drawLamp(lamp1);
		drawLamp(lamp2);
		drawLamp(lamp3);
		drawLamp(lamp4);

		glPopMatrix();
	}

	glBindTexture(GL_TEXTURE_2D, textures[0]); // wood1

	// base
	Shape base = { 0, 10, 0, 710, 10, 6 };
	drawBase(base);
	base.y = -126;
	drawBase(base);

	// seat
	Shape seat = { 0, 57.5, 450 / 2 * sqrt(3) - 15, 140, 2.5, 3.5 };
	drawSeats(seat, 6);

	glBindTexture(GL_TEXTURE_2D, textures[1]); // wood2

	// pillor
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glColor3f(0.5, 0.5, 0.5);
	for (int i = 0; i < 6; i++) {
		glPushMatrix();
		glRotatef(60 * i, 0, 1, 0);
		Cylinder(450, 500, 0, 20, 0, 494, 90, false);
		glPopMatrix();
	}

	// beam
	for (int i = 0; i < 6; i++) {
		glPushMatrix();
		glRotatef(60 * i, 0, 1, 0);
		Cuboid(0, 410, 450 / 2 * sqrt(3), 220, 15, 15);
		glPopMatrix();
	}
	//stone tablets
	for (int i = 0; i < 6; i++) {
		glPushMatrix();
		glRotatef(60 * i, 0, 1, 0);
		Cylinder(150, 120, 0, 5, 0, 120, 90, false);
		Cuboid(0, 100, 75 * sqrt(3), 75, 3, 3);
		Cuboid(0, 30, 75 * sqrt(3), 75, 3, 3);
		Cuboid(0, 80, 75 * sqrt(3), 50, 3, 3);
		Cuboid(0, 50, 75 * sqrt(3), 50, 3, 3);
		Cuboid(60, 65, 75 * sqrt(3), 10, 3, 3);
		Cuboid(-60, 65, 75 * sqrt(3), 10, 3, 3);
		Cuboid(50, 65, 75 * sqrt(3), 3, 10, 3);
		Cuboid(-50, 65, 75 * sqrt(3), 3, 10, 3);
		Cuboid(20, 90, 75 * sqrt(3), 3, 10, 3);
		Cuboid(-20, 90, 75 * sqrt(3), 3, 10, 3);
		Cuboid(20, 40, 75 * sqrt(3), 3, 10, 3);
		Cuboid(-20, 40, 75 * sqrt(3), 3, 10, 3);
		glPopMatrix();
	}

	glBindTexture(GL_TEXTURE_2D, textures[2]); // stone1 tex
	//StoneTablets
	drawStoneTablets();

	glBindTexture(GL_TEXTURE_2D, textures[3]); // stone2 tex
	// seat base
	Shape seatBase = { 0, 53, 450 / 2 * sqrt(3), 154, 2, 20 };
	drawSeatBases(seatBase, 6);

	// board
	drawPanel();
	// words on tbe board
	glBindTexture(GL_TEXTURE_2D, textures[7]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.73), glVertex3f(-138, 715, 175 * sqrt(3) + 6);
	glTexCoord2f(0.0, 0.41), glVertex3f(-138, 635, 175 * sqrt(3) + 6);
	glTexCoord2f(1.0, 0.41), glVertex3f(138, 635, 175 * sqrt(3) + 6);
	glTexCoord2f(1.0, 0.73), glVertex3f(138, 715, 175 * sqrt(3) + 6);
	glEnd();
}


// Render the road on the lawn
void drawRoad(GLfloat x, GLfloat degree) {
	RoadElement roadElements[] = {
	{ -30, 20, -47, 13, 1, 18 },
	{ -30, 20, -16, 13, 1, 10 },
	{ -30, 20, 31, 13, 1, 34 },
	{ 0, 20, -51, 13, 1, 14 },
	{ 0, 20, 15.5, 13, 1, 49.5 },
	{ 30, 20, -35, 13, 1, 30 },
	{ 30, 20, 18, 13, 1, 20 },
	{ 30, 20, 53, 13, 1, 12 }
	};

	glPushMatrix();
	glRotated(degree, 0, 1, 0);

	for (size_t i = 0; i < sizeof(roadElements) / sizeof(roadElements[0]); ++i) {

		Cuboid(roadElements[i].x + x, roadElements[i].y, roadElements[i].z,
			roadElements[i].width, roadElements[i].height, roadElements[i].length);
	}
	glPopMatrix();
	glPopMatrix();
}

// render chair
void drawChair(GLfloat posi_x, GLfloat posi_z) {

	ChairPart chairParts[] = {
		// The seat part of the chair
		{0, 66, -15, 75, 1, 3.5, 0},
		{0, 66, -5, 75, 1, 3.5, 0},
		{0, 66, 5, 75, 1, 3.5, 0},
		{0, 66, 15, 75, 1, 3.5, 0},
		// The backrest part of the chair.
		{0, 81, 0, 75, 4, 1, 0},
		{0, 91, 0, 75, 4, 1, 0},
		{0, 101, 0, 75, 4, 1, 0},
		{0, 111, 0, 75, 4, 1, 0},
		// The base part of the chair.
		{0, 20, 0, 100, 1, 25, 2},
		// The leg part of the chair.
		{52, 43, -16, 5, 22, 1, 3},
		{52, 64, 0, 5, 1, 17, 3},
		{-52, 43, 16, 5, 22, 1, 3},
		{52, 43, 18, 3.5, 22, 1, 3},
		{-52, 43, -16, 5, 22, 1, 3},
		{-52, 64, 0, 5, 1, 17, 3},
		{52, 43, -18, 3.5, 22, 1, 3},
		{-52, 43, 18, 3.5, 22, 1, 3},
		// The connecting part of the chair backrest.
		{52, 90, 2, 3.5, 27, 1, 3},
		{-52, 90, 2, 3.5, 27, 1, 3}
	};
	glPushMatrix();
	glTranslatef(posi_x, 0, posi_z);

	for (size_t i = 0; i < sizeof(chairParts) / sizeof(chairParts[0]); ++i) {
		// Bind the corresponding texture according to the texture index in the structure.
		glBindTexture(GL_TEXTURE_2D, textures[chairParts[i].textureIndex]);
		// Use the data in the structure to draw cuboids.
		Cuboid(posi_x + chairParts[i].x, chairParts[i].y, chairParts[i].z,
			chairParts[i].width, chairParts[i].height, chairParts[i].length);
	}

	glPopMatrix();
}


void drawLeafLevel(LeafLevel leafLevel) {
	glColor3f(leafLevel.colorR, leafLevel.colorG, leafLevel.colorB);
	glPushMatrix();
	glRotatef(leafLevel.rotateAngle, 0, 1, 0);
	for (int j = -8; j < 8; j++) {
		GLfloat y = leafLevel.cuboidYFunctionCoefficient * j * j;
		Cuboid(leafLevel.cuboidXOffset + j * 6, leafLevel.cuboidZ - y, 0, leafLevel.cuboidWidth, leafLevel.cuboidHeight, leafLevel.cuboidDepth);
	}
	glPopMatrix();
}

void drawTree(GLfloat posi_x, GLfloat posi_z) {
	//grass texture
	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glPushMatrix();
	glTranslatef(posi_x, 0, posi_z);

	LeafLevel leafLevels[] = {
		{43 / 255.0, 80 / 255.0, 9 / 255.0, tree_wave + 90, 47, 1.7, 240, 3, 75, 1},
		{106 / 255.0, 150 / 255.0, 36 / 255.0, tree_wave + 110, 37, 1.5, 274, 3, 75, 0.5},
		{171 / 255.0, 211 / 255.0, 60 / 255.0, tree_wave + 130, 20, 1.5, 315, 3, 75, 0.5}
	};
	int numLeafLevels = sizeof(leafLevels) / sizeof(leafLevels[0]);

	for (int i = 0; i < 6; i++) {
		for (int k = 0; k < numLeafLevels; k++) {
			LeafLevel currentLevel = leafLevels[k];
			currentLevel.rotateAngle += i * 60;
			drawLeafLevel(currentLevel);
		}
	}

	glPopMatrix();

	// wood texture
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glPushMatrix();
	glTranslatef(posi_x, 0, posi_z);
	glColor3f(0.65, 0.65, 0.65);
	Cylinder(0, 230, 0, 9, 13, 210, 90, false);
	Cylinder(0, 380, 0, 2, 9, 170, 90, false);
	glColor3f(1, 1, 1);
	glPopMatrix();
}

void RoadLight(GLfloat posi_x, GLfloat posi_z) {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0, 0, 0);
	glPushMatrix();
	glTranslatef(posi_x, 0, posi_z);
	// Array of component information for the main frame parts of the street light.
	StreetLightPart bodyFrameParts[] = {
		{0, 42, 0, 8, 22, 8, false},
		{0, 67, 0, 6, 3, 6, false}
	};

	// Array of component information for the horizontal frame parts of the street light.
	StreetLightPart horizontalFrameParts[] = {
		{0, 86, 7, 8, 1, 1, false},
		{0, 86, -7, 8, 1, 1, false},
		{0, 70, 7, 8, 1, 1, false},
		{0, 70, -7, 8, 1, 1, false},
		{7, 86, 0, 1, 1, 8, false},
		{-7, 86, 0, 1, 1, 8, false},
		{7, 70, 0, 1, 1, 8, false},
		{-7, 70, 0, 1, 1, 8, false}
	};

	// Array of component information for the vertical frame parts of the street light.
	StreetLightPart verticalFrameParts[] = {
		{7, 78, -7, 1, 8, 1, false},
		{-7, 78, -7, 1, 8, 1, false},
		{7, 78, 7, 1, 1, 8, false},
		{-7, 78, 7, 1, 1, 8, false}
	};

	// Component information of the luminous part of the street light.
	StreetLightPart lightPart = { 0, 78, 0, 6, 6, 6, false };


	// bodyFrame
	for (size_t i = 0; i < sizeof(bodyFrameParts) / sizeof(bodyFrameParts[0]); ++i) {
		Cuboid(bodyFrameParts[i].x, bodyFrameParts[i].y, bodyFrameParts[i].z,
			bodyFrameParts[i].width, bodyFrameParts[i].height, bodyFrameParts[i].length);
	}
	//horizontalFrame
	for (size_t i = 0; i < sizeof(horizontalFrameParts) / sizeof(horizontalFrameParts[0]); ++i) {
		Cuboid(horizontalFrameParts[i].x, horizontalFrameParts[i].y, horizontalFrameParts[i].z,
			horizontalFrameParts[i].width, horizontalFrameParts[i].height, horizontalFrameParts[i].length);
	}
	//verticalFrame
	for (size_t i = 0; i < sizeof(verticalFrameParts) / sizeof(verticalFrameParts[0]); ++i) {
		Cuboid(verticalFrameParts[i].x, verticalFrameParts[i].y, verticalFrameParts[i].z,
			verticalFrameParts[i].width, verticalFrameParts[i].height, verticalFrameParts[i].length);
	}

	GLfloat emmission[] = { 0.7, 0.4, 0.4 };
	GLfloat no_emmission[] = { 0, 0, 0 };
	if (day_state == 2) {  // At night (when day_state is 2).
		lightPart.isEmissionOn = true;
		glMaterialfv(GL_FRONT, GL_EMISSION, emmission);
	}
	else {
		lightPart.isEmissionOn = false;
		glMaterialfv(GL_FRONT, GL_EMISSION, no_emmission);
	}
	glColor3f(1, 1, 1);
	if (lightPart.isEmissionOn) {
		Cuboid(lightPart.x, lightPart.y, lightPart.z,
			lightPart.width, lightPart.height, lightPart.length);
	}

	glMaterialfv(GL_FRONT, GL_EMISSION, no_emmission);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

//// draw the lawn
void Lawn() {
	GLfloat x_start = -2000.0, x_len = 4000;	// set the size of the lawn
	GLfloat z_start = 1740, z_len = 2000;
	GLfloat tex_len = 200;	// size of piece of the grass
	GLfloat lawn_z = 1825;

	// using grass texture
	glBindTexture(GL_TEXTURE_2D, textures[19]);
	for (int i = 0; i < x_len / tex_len; i++) {
		for (int j = 0; j < z_len / tex_len; j++) {
			glBegin(GL_QUADS);	// bind texture
			glTexCoord2f(0.0, 0.0), glVertex3f(x_start + tex_len * i, 19, z_start + 200 * (j + 1));
			glTexCoord2f(0.0, 1.0), glVertex3f(x_start + tex_len * i, 19, z_start + 200 * j);
			glTexCoord2f(1.0, 1.0), glVertex3f(x_start + tex_len * (i + 1), 19, z_start + 200 * j);
			glTexCoord2f(1.0, 0.0), glVertex3f(x_start + tex_len * (i + 1), 19, z_start + 200 * (j + 1));
			glEnd();
		}
	}

	// draw roads
	glBindTexture(GL_TEXTURE_2D, textures[2]);	// using stone texture
	for (int i = -1; i <= 1; i += 2)
	{
		glPushMatrix();
		glTranslatef(i * 741.5, 0, lawn_z + 22);
		for (int i = 1; i < 20; i++)
			drawRoad(i * 90, 270);
		glPopMatrix();
	}

	// draw chairs
	drawChair(250, lawn_z);
	drawChair(-250, lawn_z);

	int treeGroup1XRanges[][2] = { {-1, 1}, {0, 0}, {1, 1} };
	int treeGroup1ZOffset = 0;
	for (int k = 0; k < sizeof(treeGroup1XRanges) / sizeof(treeGroup1XRanges[0]); ++k) {
		for (int i = treeGroup1XRanges[k][0]; i < treeGroup1XRanges[k][1]; ++i) {
			drawTree(635 * i, 1825 + treeGroup1ZOffset);
			drawTree(351 * i, 1825 + treeGroup1ZOffset);
			drawTree(117 * i, 1825 + treeGroup1ZOffset);
		}
	}

	int treeGroup2XRanges[][2] = { {0, 5}, {-1, -5} };
	int treeGroup2ZOffset = 110;
	for (int k = 0; k < sizeof(treeGroup2XRanges) / sizeof(treeGroup2XRanges[0]); ++k) {
		for (int i = treeGroup2XRanges[k][0]; i < treeGroup2XRanges[k][1]; ++i) {
			drawTree(840 + 280 * i, 1825 + treeGroup2ZOffset);
			drawTree(-840 - 280 * i, 1825 + treeGroup2ZOffset);
		}
	}

	int treeGroup3XRanges[][2] = { {-3, 4} };
	int treeGroup3ZOffset = 240;
	for (int k = 0; k < sizeof(treeGroup3XRanges) / sizeof(treeGroup3XRanges[0]); ++k) {
		for (int i = treeGroup3XRanges[k][0]; i < treeGroup3XRanges[k][1]; ++i) {
			drawTree(211 * i, 1825 + treeGroup3ZOffset);
		}
	}
	int treeGroup4XRanges[][2] = { {-6, 7} };
	int treeGroup4ZOffsets[] = { 640, 1040, 1440 };
	for (int zOffset : treeGroup4ZOffsets) {
		for (int k = 0; k < sizeof(treeGroup4XRanges) / sizeof(treeGroup4XRanges[0]); ++k) {
			for (int i = treeGroup4XRanges[k][0]; i < treeGroup4XRanges[k][1]; ++i) {
				drawTree(280 * i, 1825 + zOffset);
			}
		}
	}
	// streetLight
	int streetLightXPositions[] = { 0, 234, -234, 980, -980, -527.5 };
	int streetLightZPositions[] = { lawn_z, lawn_z + 110, lawn_z + 240 };
	for (int posX : streetLightXPositions) {
		for (int posZ : streetLightZPositions) {
			RoadLight(posX, posZ);
		}
	}
}


// drawQuad
void drawQuadWithTexCoords(GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z) {
	glTexCoord2f(s, t);
	glVertex3f(x, y, z);
}

//  drawTriangle
void drawTriangleWithTexCoords(GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z) {
	glTexCoord2f(s, t);
	glVertex3f(x, y, z);
}

// renderBankBase
void renderBankBase(const ObjectData& data) {
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(data.texCoords[i][0], data.texCoords[i][1]);
		glVertex3f(data.vertices[i][0], data.vertices[i][1], data.vertices[i][2]);
	}
	glEnd();
}
void Bank() {
	// Define constants
	constexpr int numPillars = 10;
	constexpr float pillarSpacing = 2 * 47;
	constexpr float pillarWidth = 7;
	constexpr float pillarHeight = 40;
	constexpr float pillarDepth = 7;
	constexpr float stairLen = 20;
	constexpr float stairHeight = 110 / 8;

	// Render bank base
	ObjectData bankBaseData = {
		{
			{1010, -252, 1500},
			{1010, 10, 1500},
			{-1030, 10, 1500},
			{-1030, -252, 1500}
		},
		{
			{0.0, 0.0},
			{0.0, 1.0},
			{10.0, 1.0},
			{10.0, 0.0}
		}
	};
	renderBankBase(bankBaseData);

	// Render pillars
	for (int i = 0; i <= numPillars; i++) {
		Cuboid(70 + pillarSpacing * i, 60, 1520, pillarWidth, pillarHeight, pillarDepth);
		Cuboid(-70 - pillarSpacing * i, 60, 1520, pillarWidth, pillarHeight, pillarDepth);
		Cuboid(-1010 - pillarSpacing * i, 60, 1720, pillarWidth, pillarHeight, pillarDepth);
		Cuboid(1010 + pillarSpacing * i, 60, 1720, pillarWidth, pillarHeight, pillarDepth);
	}

	// Pillars on the stairs
	for (int i = 1; i < 5; i++)
		Cuboid(1010 + stairLen * i / 4, 60 - stairHeight * i / 4, 1520, pillarWidth, pillarHeight, pillarDepth);

	// Side pillar
	for (int i = 1; i <= 2; i++)
		Cuboid(-1010, 60, 1520 + i * 100, pillarWidth, pillarHeight, pillarDepth);

	// Render stairs
	for (int i = 1; i <= 8; i++) {
		Cuboid(1010 + stairLen * (2 * i - 1), -90 - stairHeight * i, 1700, stairLen, 110 - stairHeight * i, 160);
	}

	// Render top and bottom quads
	glBegin(GL_QUADS);
	drawQuadWithTexCoords(0.0, 0.0, 1010, 20, 1500);
	drawQuadWithTexCoords(0.0, 1.0, 1010, 20, 1540);
	drawQuadWithTexCoords(1.0, 1.0, 1344, -200, 1540);
	drawQuadWithTexCoords(1.0, 0.0, 1344, -200, 1500);
	glEnd();

	// Render top and bottom triangles
	glBegin(GL_TRIANGLES);
	drawTriangleWithTexCoords(0.0, 0.0, 1010, 20, 1540);
	drawTriangleWithTexCoords(0.0, 1.0, 1010, -200, 1540);
	drawTriangleWithTexCoords(1.0, 1.0, 1344, -200, 1540);
	drawTriangleWithTexCoords(0.0, 0.0, 1010, 20, 1500);
	drawTriangleWithTexCoords(0.0, 1.0, 1010, -200, 1500);
	drawTriangleWithTexCoords(1.0, 1.0, 1344, -200, 1500);
	glEnd();

	// Render planks
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	for (int i = 0; i < 112; i++) {
		Cuboid(-1010 + i * 18, 14, 1610, 7, 4, 90);
	}
	Cuboid(540, 80, 1520, 470, 5, 5);
	Cuboid(540, 45, 1520, 470, 25, 5);
	Cuboid(-540, 80, 1520, 470, 5, 5);
	Cuboid(-540, 45, 1520, 470, 25, 5);
	Cuboid(-1510, 80, 1720, 500, 5, 5);
	Cuboid(-1510, 45, 1720, 500, 25, 5);
	Cuboid(1510, 80, 1720, 500, 5, 5);
	Cuboid(1510, 45, 1720, 500, 25, 5);

	// Render stair guard
	glBegin(GL_QUADS);
	drawQuadWithTexCoords(0.0, 0.0, 1010, 70, 1515);
	drawQuadWithTexCoords(0.0, 1.0, 1010, 70, 1525);
	drawQuadWithTexCoords(1.0, 1.0, 1344, -150, 1525);
	drawQuadWithTexCoords(1.0, 0.0, 1344, -150, 1515);
	glEnd();

	glBegin(GL_QUADS);
	drawQuadWithTexCoords(0.0, 0.0, 1010, 75, 1515);
	drawQuadWithTexCoords(0.0, 1.0, 1010, 75, 1525);
	drawQuadWithTexCoords(1.0, 1.0, 1344, -145, 1525);
	drawQuadWithTexCoords(1.0, 0.0, 1344, -145, 1515);
	glEnd();

	Cuboid(-1010, 80, 1620, 5, 5, 100);
	Cuboid(-1010, 45, 1620, 5, 25, 100);
}
void drawCloud(GLfloat pos_x, GLfloat pos_y, GLfloat pos_z, GLfloat size, GLfloat color) {
	glDisable(GL_TEXTURE_2D);

	// Set the cloud color according to day_state
	GLfloat cloudColor[3];
	if (day_state < 2) {
		cloudColor[0] = color;
		cloudColor[1] = color;
		cloudColor[2] = color;
	}
	else {
		cloudColor[0] = color - 0.3;
		cloudColor[1] = color - 0.3;
		cloudColor[2] = color - 0.3;
	}
	glColor3fv(cloudColor);

	// Define the array of spherical information that makes up the cloud
	CloudSphere spheres[] = {
		{pos_x, pos_y, pos_z, size},
		{pos_x + 50 * size, pos_y, pos_z, size},
		{pos_x + 90 * size, pos_y - 30 * size, pos_z - 30 * size, size},
		{pos_x - 60 * size, pos_y - 15 * size, pos_z + 20 * size, size},
		{pos_x - 55 * size, pos_y - 25 * size, pos_z - 45 * size, size}
	};

	// Draw each sphere to form a cloud
	for (const auto& sphere : spheres) {
		glPushMatrix();
		glTranslatef(sphere.pos_x, sphere.pos_y, sphere.pos_z);
		glutSolidSphere(70 * sphere.size, 200, 200);
		glPopMatrix();
	}

	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
}

// Render a single face of the sky.
void renderSkyFace(const SkyFaceData& faceData, GLuint textureId) {
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBegin(GL_QUADS);
	for (int i = 0; i < 8; i += 4) {
		for (int j = 0; j < 4; j++) {
			glTexCoord2f(faceData.texCoords[i + j][0], faceData.texCoords[i + j][1]);
			glVertex3f(faceData.vertices[i + j][0], faceData.vertices[i + j][1], faceData.vertices[i + j][2]);
		}
	}
	glEnd();
}

void Sky() {
	// set position
	GLfloat x_start = -2500, x_end = 2500;
	GLfloat y_start = -800, y_mid = 0.0, y_end = 3300;
	GLfloat z_start = -1500, z_end = 3500;
	GLfloat lookat_y, y_rate;
	int daytime;	// choose sky picture by day_state state

	day_state == 2 ? daytime = 13, lookat_y = 0.5, y_rate = 0.4 : daytime = 8,
		lookat_y = 0.5, y_rate = 0.13;

	//  front
	SkyFaceData frontFaceData = {
		{
			{0.0, 1.0},
			{0.0, lookat_y},
			{1.0, lookat_y},
			{1.0, 1.0},
			{0.0, lookat_y},
			{0.0, y_rate},
			{1.0, y_rate},
			{1.0, lookat_y}
		},
		{
			{x_end, y_end, z_end},
			{x_end, y_mid, z_end},
			{x_start, y_mid, z_end},
			{x_start, y_end, z_end},
			{x_end, y_mid, z_end},
			{x_end, y_start, z_end},
			{x_start, y_start, z_end},
			{x_start, y_mid, z_end}
		}
	};
	// renderFront
	renderSkyFace(frontFaceData, textures[daytime]);

	//  right
	SkyFaceData rightFaceData = {
		{
			{0.0, 1.0},
			{0.0, lookat_y},
			{1.0, lookat_y},
			{1.0, 1.0},
			{0.0, lookat_y},
			{0.0, y_rate},
			{1.0, y_rate},
			{1.0, lookat_y}
		},
		{
			{x_end, y_end, z_start},
			{x_end, y_mid, z_start},
			{x_end, y_mid, z_end},
			{x_end, y_end, z_end},
			{x_end, y_mid, z_start},
			{x_end, y_start, z_start},
			{x_end, y_start, z_end},
			{x_end, y_mid, z_end}
		}
	};

	// render right
	renderSkyFace(rightFaceData, textures[daytime + 1]);

	// back
	SkyFaceData backFaceData = {
		{
			{0.0, 1.0},
			{0.0, lookat_y},
			{1.0, lookat_y},
			{1.0, 1.0},
			{0.0, lookat_y},
			{0.0, y_rate},
			{1.0, y_rate},
			{1.0, lookat_y}
		},
		{
			{x_start, y_end, z_start},
			{x_start, y_mid, z_start},
			{x_end, y_mid, z_start},
			{x_end, y_end, z_start},
			{x_start, y_mid, z_start},
			{x_start, y_start, z_start},
			{x_end, y_start, z_start},
			{x_end, y_mid, z_start}
		}
	};
	// render back
	renderSkyFace(backFaceData, textures[daytime + 2]);

	// left
	SkyFaceData leftFaceData = {
		{
			{0.0, 1.0},
			{0.0, lookat_y},
			{1.0, lookat_y},
			{1.0, 1.0},
			{0.0, lookat_y},
			{0.0, y_rate},
			{1.0, y_rate},
			{1.0, lookat_y}
		},
		{
			{x_start, y_end, z_end},
			{x_start, y_mid, z_end},
			{x_start, y_mid, z_start},
			{x_start, y_end, z_start},
			{x_start, y_mid, z_end},
			{x_start, y_start, z_end},
			{x_start, y_start, z_start},
			{x_start, y_mid, z_start}
		}
	};
	// render left
	renderSkyFace(leftFaceData, textures[daytime + 3]);

	// top
	SkyFaceData topFaceData = {
		{
			{0.0, 0.0},
			{1.0, 0.0},
			{1.0, 1.0},
			{0.0, 1.0}
		},
		{
			{x_start, y_end, z_end},
			{x_start, y_end, z_start},
			{x_end, y_end, z_start},
			{x_end, y_end, z_end}
		}
	};
	// render top
	renderSkyFace(topFaceData, textures[daytime + 4]);

	// cloud
	for (int i = 0; i < 10; i++)
		drawCloud(cloud_configure[i][0], cloud_configure[i][1], cloud_configure[i][2], cloud_configure[i][3], cloud_configure[i][5]);
}



void setupRenderingState(bool disableTexture) {
	if (disableTexture) {
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glEnable(GL_TEXTURE_2D);
	}
}
void restoreRenderingState(bool disableTexture) {
	if (disableTexture) {
		glEnable(GL_TEXTURE_2D);
	}
}

void drawSpindrift() {
	setupRenderingState(true);
	GLfloat ctrlPoints[3][3][3] = {
		{{ 1500.0f, -3.0f, -40.0f }, { 1500.0f, 13.0f, 0.0f }, { 1500.0f, 10.0f, 16.0f }},
		{{ 0.0f, -3.0f, -40.0f }, { 0.0f, 13.0f, 0.0f }, { 0.0f, 10.0f, 16.0f }},
		{{ -1500.0f, -3.0f, -40.0f }, { -1500.0f, 13.0f, 0.0f }, { -1500.0f, 10.0f, 16.0f }}
	};	// control points
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(1000, -245, (i * 500 + i * 500 + sea_wave) % 5000 - 1500);	// set position
		glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
		glMap2f(GL_MAP2_VERTEX_3,	// set map
			0.0f, 10.0f, 3, 3,
			0.0f, 10.0f, 9, 3,
			&ctrlPoints[0][0][0]);
		glEnable(GL_MAP2_VERTEX_3);
		glMapGrid2f(30, 0.0f, 20.0f, 10, 0.0f, 20.0f);	//  Set up the grid
		glEvalMesh2(GL_LINE, 0, 10, 0, 10);	//Compute the grid
		glPopMatrix();
	}
	glColor3f(1, 1, 1);
	restoreRenderingState(true);
}
// Calculate the sea color based on the current location coordinate and the daytime state
void calculateSeaColor(GLfloat loc, GLfloat& r, GLfloat& g, GLfloat& b) {
	if (day_state <= 1) {
		r = std::max(80 / 255.0, (163 - loc * 83 / 50.0) / 255.0);
		g = std::max(100 / 255.0, (213 - loc * 113 / 50.0) / 255.0);
		b = std::max(131 / 255.0, (231 - loc * 100 / 50.0) / 255.0);
	}
	else {
		r = std::max(0 / 255.0, (163 - loc * 163 / 50.0) / 255.0);
		g = std::max(0 / 255.0, (213 - loc * 213 / 50.0) / 255.0);
		b = std::max(10 / 255.0, (231 - loc * 231 / 50.0) / 255.0);
	}
}

// Render a single sea texture quad using the provided quad data
void renderSeaQuad(const SeaQuadData& quadData) {
	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(quadData.texCoords[i][0], quadData.texCoords[i][1]);
		glVertex3f(quadData.vertices[i][0], quadData.vertices[i][1], quadData.vertices[i][2]);
	}
	glEnd();
}
// Function to render the sea surface
void Ocean() {
	// Set the specular property of the material
	GLfloat specular[] = { 0.5, 0.5, 0.5, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Bind the specific texture for the sea
	glBindTexture(GL_TEXTURE_2D, textures[4]);

	GLfloat sea_z_start = 2250, sea_length = 5000;
	GLfloat sea_tex_size = 50;

	// Loop to generate and render the texture quads of the sea surface
	for (int j = -95; j < 5; j++) {
		for (int i = -50; i < 50; i++) {
			// Calculate the value that makes the sea color darker from the center to the edge
			GLfloat loc = std::max(0.0, std::sqrt(i * i + j * j) - 8);
			GLfloat r, g, b;
			calculateSeaColor(loc, r, g, b);
			glColor3f(r, g, b);

			// Construct the data of the current texture quad
			SeaQuadData quadData = {
				{
					{sea_tex_size * i, -252.0, sea_z_start + sea_tex_size * (j + 1)},
					{sea_tex_size * i, -252.0, sea_z_start + sea_tex_size * j},
					{sea_tex_size * (i + 1), -252.0, sea_z_start + sea_tex_size * j},
					{sea_tex_size * (i + 1), -252.0, sea_z_start + sea_tex_size * (j + 1)}
				},
				{
					{0.0, 0.0},
					{0.0, 1.0},
					{1.0, 1.0},
					{1.0, 0.0}
				}
			};
			renderSeaQuad(quadData);
		}
	}

	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);

	drawSpindrift();
}

// Manage lights
void set_lights() {
	// Global ambient light settings
	GLfloat dayLight[] = { 0.8, 0.9, 0.3, 1.0 };
	GLfloat nightLight[] = { 0.2, 0.2, 0.3, 1.0 };
	day_state == 2 ? glLightModelfv(GL_LIGHT_MODEL_AMBIENT, nightLight) : glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dayLight);
	// Generate a parallel light
	parallelingLights();
	// Create a spotlight
	point_Light(1400, 1650, -1600);
	spotingLight(1400, 1650, -1600);

}

// display the scene
void displayfunc(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	lookat_x = 220 * sin(degree / 200) + camera_x;
	lookat_z = 220 * cos(degree / 200) + camera_z;
	gluLookAt(camera_x, camera_y, camera_z, lookat_x, lookat_y, lookat_z, Vx, Vy, Vz);	// set camera position, look at point and direction of coordinate system 
	// Configure lights
	set_lights();
	// draw
	Sky(); Ocean();
	bridge(); Pavilion();
	Bank(); Lawn();

	glutSwapBuffers();
}


void load_tex() {
	std::string path = "texturesyu/";
	auto loadTexture = [path](const std::string& filename) {
		return loadImg(path + filename);
	};

	wood1 = loadTexture("wood1.bmp");
	wood2 = loadTexture("wood2.bmp");
	wood3 = loadTexture("wood3.bmp");
	stone1 = loadTexture("stone1.bmp");
	stone2 = loadTexture("stone2.bmp");
	sea = loadTexture("sea.bmp");
	grass = loadTexture("grass.bmp");
	grass2 = loadTexture("grass2.bmp");
	words = loadTexture("word.bmp");

	std::string sky = "texturesyu/sky_";
	for (int i = 0; i <= 4; ++i) {
		std::string filename = "sky_" + std::to_string(i) + ".bmp";
		sky_data[i] = loadTexture(filename);
	}

	std::string night = "texturesyu/night_";
	for (int i = 0; i <= 4; ++i) {
		std::string filename = "night_" + std::to_string(i) + ".bmp";
		night_data[i] = loadTexture(filename);
	}
}

void texturePara_repeat_Set() {
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
void texturePara_CLAMP_Set() {
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	// set texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void texture_config() {

	// bind and save textures
	glShadeModel(GL_FLAT);		// select flat or smooth shading
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		// set texture environment parameters
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Choose the most correct or highest quality option
	glGenTextures(20, textures);	// get texture's id and bind textures

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 200, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, wood1);
	texturePara_repeat_Set();

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, wood2);
	texturePara_CLAMP_Set();

	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, stone1);
	texturePara_repeat_Set();

	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, stone2);
	texturePara_CLAMP_Set();

	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, sea);
	texturePara_CLAMP_Set();

	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, grass);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, textures[7]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, words);
	texturePara_CLAMP_Set();

	for (int i = 0; i < 5; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[8 + i]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, sky_data[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	for (int i = 0; i < 5; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[13 + i]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, night_data[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D, textures[18]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 200, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, wood3);
	texturePara_CLAMP_Set();

	glBindTexture(GL_TEXTURE_2D, textures[19]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, grass2);
	texturePara_CLAMP_Set();

}

// initiating
void initialization(void)
{
	glClearColor(0, 0, 0, 0.0);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	gluPerspective(fltFOV, 1, 100, 6000);	// set the frustum

	glEnable(GL_LIGHTING);	// enable lighting
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	//load_tex
	load_tex();

	//texture_config
	texture_config();

	// set clouds randomly
	for (int i = 0; i < 10; i++) {
		cloud_configure[i][0] = rand() % 5000 - 2500;
		cloud_configure[i][1] = rand() % 500 + 2800;
		cloud_configure[i][2] = rand() % 5000 - 1500;
		cloud_configure[i][3] = rand() % 5 + 4;
		cloud_configure[i][4] = rand() % 5 + 10;
		cloud_configure[i][5] = rand() % 4 / 10.0 + 0.5;
	}

}

void menu(int item) // The mouse along with the keyboard can control the display in its entirety.
{
	Keyboards_input((unsigned char)item, 0, 0);
	mouseInput((unsigned char)item, GLUT_UP, 0, 0);
	mySpecialKeys((unsigned char)item, 0, 0);
}

void reshape(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	int s_width = glutGet(GLUT_SCREEN_WIDTH);
	int s_height = glutGet(GLUT_SCREEN_HEIGHT);
	// create the window
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition((s_width - winWidth) / 2, (s_height - winHeight) / 2);	// center the window
	glutCreateWindow("2253615_Huiwei_Xiao");

	glEnable(GL_MULTISAMPLE);

	initialization();
	glutDisplayFunc(displayfunc);
	glutIdleFunc(myIdleFunc);	// animation
	glutReshapeFunc(reshape);
	glutKeyboardFunc(Keyboards_input);
	glutSpecialFunc(mySpecialKeys);
	glutMouseFunc(mouseInput);

	int viewMenu = glutCreateMenu(menu);
	glutAddMenuEntry("MoveRight[RIGHT]", GLUT_KEY_RIGHT);
	glutAddMenuEntry("MoveLeft[LEFT]", GLUT_KEY_LEFT);
	glutAddMenuEntry("MoveForward[UP]", GLUT_KEY_UP);
	glutAddMenuEntry("MoveBack[DOWN]", GLUT_KEY_DOWN);
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddMenuEntry("LookRight[d]", 'd');
	glutAddMenuEntry("LookLeft[a]", 'a');
	glutAddMenuEntry("LookForward[w]", 'w');
	glutAddMenuEntry("LookBack[s]", 's');
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddMenuEntry(" Elevate Up [WHEEL_UP]", GLUT_WHEEL_UP);
	glutAddMenuEntry(" Elevate Down[WHEEL_DOWN]", GLUT_WHEEL_DOWN);

	glutCreateMenu(menu);
	glutAddMenuEntry("Control Menu", '\0');
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddSubMenu("View and Moving Menu ", viewMenu);
	glutAddMenuEntry(" ", '\0');
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddMenuEntry("Change the day_state", 't');
	glutAddMenuEntry("Speed up [m]", 'm');
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddMenuEntry("Thank you for testing my program.", '\0');
	glutAddMenuEntry(" ", '\0');
	glutAddMenuEntry("----------------------------------", '\0');
	glutAddMenuEntry("Quit [Q]", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON); // Open the menu

	glutMainLoop();
}