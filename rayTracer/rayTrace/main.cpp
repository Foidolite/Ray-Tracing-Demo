#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <gl/freeglut.h>
#include <cmath>
#include <stdio.h>
#include <soil.h>

#include "shader.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

//macros
#define MAX_OBJ 64

//shaders
Shader rayShader;

//state variables
int width = 1000, height = 500;
float FOV = 90; //degrees
Vector<float> eyePos = { 0,0,5 };
Matrix<float> viewRotMat = { {1,0,0} , {0,1,0} , {0,0,1} };
float heightRatio = tan(M_PI*(0.5*FOV) / 180.0)/height;

int targetFPS = 24;
double lastTime;
double dt;

bool up = false, down = false, left = false, right = false;
float moveSpeed = 3.0;

//callback declarations
void display(void);
void update(void);
void reshape(int w, int h);
void onMouseMoved(int x, int y);
void OnKeyboardDown(unsigned char key, int x, int y);
void OnKeyboardUp(unsigned char key, int x, int y);

//hitable object struct
struct Hitable {
	//commented members are positioned to facillitate readability. Their actual declarations are grouped for padding purposes.
	//int type; //-1 = none; 0 = sphere; 1 = plane; 2 = triangle; 100 = BVH box
	//sphere properties
	float center[4];
	//float radius;
	//plane properties
	float normal[4];
	float point[4];
	//triangle properties
	float A[4];
	float B[4];
	float C[4];

	//material properties
	//int matType; //-1 = none; 1 = diffuse; 2 = reflective; 3 = dieletric
	float color[4];
	//reflective properties
	//float fuzz;
	//refractive properties
	//float refIdx;

	//texture properties
	//float uvA[2];
	//float uvB[2];
	//float uvC[2];
	//int texId;
	
	//group above commented members for padding purposes
	int type; //-1 = none; 0 = sphere; 1 = plane; 2 = triangle
	float radius;
	int matType; //-1 = none; 1 = diffuse; 2 = reflective; 3 = diffuse&reflective; 4 = dieletric
	float fuzz;
	float refIdx;
	int texId;
	float uvA[2];
	float uvB[2];
	float uvC[2];

	Hitable() { type = -1; texId = -1; }
};

//hitable constructors
template <typename component>
Hitable Sphere(Vector<component> c, float r) {
	Hitable sphere;
	sphere.type = 0;
	sphere.texId = -1;
	sphere.radius = r;
	sphere.matType = 1;
	for (int i = 0; i < 3; ++i) {
		sphere.center[i] = c[i];
		sphere.color[i] = 0.5;
	}
	
	return sphere;
}

template <typename component>
Hitable Plane(Vector<component> p, Vector<component> n) {
	Hitable plane;
	plane.type = 1;
	plane.texId = -1;
	plane.matType = 1;
	for (int i = 0; i < 3; ++i) {
		plane.point[i] = p[i];
		plane.normal[i] = n[i];
		plane.color[i] = 0.5;
	}

	return plane;
}

template <typename component>
Hitable Triangle(Vector<component> a, Vector<component> b, Vector<component> c) {
	Hitable tri;
	tri.type = 2;
	tri.texId = -1;
	tri.matType = 1;
	for (int i = 0; i < 3; ++i) {
		tri.A[i] = a[i];
		tri.B[i] = b[i];
		tri.C[i] = c[i];
		tri.color[i] = 0.5;
	}

	return tri;
}

//model I/O
void loadObj(const char filename[], Hitable* objArray, int at, Hitable refObj) {
	const unsigned int BUFFER_SIZE = 1024;

	FILE* p_obj;
	char str[BUFFER_SIZE];

	fopen_s(&p_obj, filename, "r");
	if (p_obj == nullptr)
		return;

	//get a count of all objects
	int verticeCount = 0;
	int uvCount = 0;
	int faceCount = 0;
	while (fgets(str, BUFFER_SIZE, p_obj) != NULL) {
		if (str[0] == 'v') {
			if (str[1] == ' ')
				++verticeCount;
			else if (str[1] == 't')
				++uvCount;
		}
		else if (str[0] == 'f') {
			++faceCount;
		}
	}

	//fill index buffers & build Triangles
	float* vertices = new float[3 * verticeCount]{ 0 };
	float* uvs = new float[2 * uvCount]{ 0 };
	Hitable* faces = objArray + at;

	rewind(p_obj);
	verticeCount = 0;
	uvCount = 0;
	faceCount = 0;
	while (fgets(str, BUFFER_SIZE, p_obj) != NULL) {
		if (str[0] == 'v') {
			if (str[1] == ' ') {
				int cursor = 2;
				for (int i = 0; i < 3; ++i) {
					char value[16];
					int start = cursor;
					while (str[cursor] != ' ' && str[cursor] != '\n') {
						value[cursor - start] = str[cursor];
						++cursor;
					}
					value[cursor - start] = '\0';
					sscanf_s(value, "%f", &vertices[verticeCount]);
					++verticeCount;
					++cursor;
				}
			}
			else if (str[1] == 't') {
				int cursor = 3;
				for (int i = 0; i < 2; ++i) {
					char value[10];
					int start = cursor;
					while (str[cursor] != ' ' && str[cursor] != '\n') {
						value[cursor - start] = str[cursor];
						++cursor;
					}
					value[cursor - start] = '\0';
					sscanf_s(value, "%f", &uvs[uvCount]);
					++uvCount;
					++cursor;
				}
			}
		}
		else if (str[0] == 'f') {
			int cursor = 2;
			*faces = Hitable();
			faces->type = 2;
			faces->matType = refObj.matType;
			faces->texId = refObj.texId;
			faces->fuzz = refObj.fuzz;
			faces->refIdx = refObj.refIdx;
			for (int i = 0; i < 3; ++i) {
				char value[16];
				int start = cursor;
				while (str[cursor] != ' ' && str[cursor] != '\n') {
					value[cursor - start] = str[cursor];
					++cursor;
				}
				value[cursor - start] = '\0';
				int indices[2];
				sscanf_s(value, "%d %*c %d", &indices[0], &indices[1]);

				if (i == 0) {
					memcpy(faces->A, vertices + ((indices[0] - 1) * 3), 3 * sizeof(float));
					if (indices[1] != 0)
						memcpy(faces->uvA, uvs + ((indices[1] - 1) * 2), 2 * sizeof(float));
				}
				else if (i == 1) {
					memcpy(faces->B, vertices + ((indices[0] - 1) * 3), 3 * sizeof(float));
					if (indices[1] != 0)
						memcpy(faces->uvB, uvs + ((indices[1] - 1) * 2), 2 * sizeof(float));
				}
				else {
					memcpy(faces->C, vertices + ((indices[0] - 1) * 3), 3 * sizeof(float));
					if (indices[1] != 0)
						memcpy(faces->uvC, uvs + ((indices[1] - 1) * 2), 2 * sizeof(float));
				}

				faces->color[i] = refObj.color[i];

				++cursor;
			}
			faces++;
			if (faces > objArray+MAX_OBJ)
				return;
		}
	}

	fclose(p_obj);

	//free
	delete[] vertices;
	delete[] uvs;
}

//prepare the world to be rendered
Hitable world[MAX_OBJ];
GLuint uboObjs;

//main entry / initialize
int main(int argc, char* argv[]) {
	//initialize window
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Main");

	//glutFullScreen();

	//initialize GLEW
	glewInit();

	//initialize shader
	rayShader.init("None", "rayShader.frag");

	//set up callbacks
	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutReshapeFunc(reshape);
	glutMotionFunc(onMouseMoved);
	glutPassiveMotionFunc(onMouseMoved);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutKeyboardFunc(OnKeyboardDown);
	glutKeyboardUpFunc(OnKeyboardUp);

	//initialize world and bind to UBO
	world[0] = Plane(Vector<float>({ 0,-1,0 }), Vector<float>({ 0,1,0 }));
	world[1] = Sphere(Vector<float>({ 6,4,-2 }), 5);
	world[1].matType = 2; world[1].fuzz = 0; world[1].color[0] = 1; world[1].color[1] = 216./255; world[1].color[2] = 228./255;
	for (int i = 2; i < 64; ++i) {
		float y = (std::rand() % 100 - 50) / 40.0f;
		world[i] = Sphere(Vector<float>({ (std::rand() % 100) / 100.0f + (i-50)/3.0f, y ,(std::rand() % 100 - 50) / 2.5f }), y+1);
		world[i].matType = std::rand() % 3 + 1; 
		world[i].fuzz = 0;
		world[i].refIdx = 1.5;
		world[i].color[0] = (std::rand() % 100)/100.0f; 
		world[i].color[1] = (std::rand() % 100) / 100.0f; 
		world[i].color[2] = (std::rand() % 100) / 100.0f;
	}
	//model I/O, uncomment and make room in world array to use
	/*Hitable refObj;
	refObj.matType = 2;
	refObj.refIdx = 1.5;
	refObj.fuzz = 0;
	refObj.color[0] = 0.5; refObj.color[1] = 0.5; refObj.color[2] = 0.5; 
	refObj.texId = 0;
	loadObj("cube.obj", world, 10, refObj);
	refObj.matType = 3;
	refObj.texId = 1;
	loadObj("alignedCube.obj", world, 22, refObj);*/

	int binding_index = 1;
	rayShader.bind();
	int block_index = glGetUniformBlockIndex(rayShader.id(), "worldBlock");
	glUniformBlockBinding(rayShader.id(), block_index, binding_index);
	rayShader.unbind();

	glGenBuffers(1, &uboObjs);
	glBindBuffer(GL_UNIFORM_BUFFER, uboObjs);
	glBindBufferRange(GL_UNIFORM_BUFFER, binding_index, uboObjs, 0, sizeof(world));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(world), world, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(world), world);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//load and bind any textures
	int twidth, theight;
	unsigned char* img1 = SOIL_load_image("squareTex.png", &twidth, &theight, 0, SOIL_LOAD_RGBA);
	unsigned char* img2 = SOIL_load_image("refCubeTex2.png", &twidth, &theight, 0, SOIL_LOAD_RGBA);

	GLuint texArray;
	glGenTextures(1, &texArray);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, twidth, theight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, twidth, theight, 1, GL_RGBA, GL_UNSIGNED_BYTE, img1);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, twidth, theight, 1, GL_RGBA, GL_UNSIGNED_BYTE, img2);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//check errors
	printf("glGetError returned %d\n", glGetError());
	lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	//enter main loop
	glutMainLoop();

	return 0;
}

static bool just_warped = false;
void onMouseMoved(int x, int y) {
	if (just_warped)
	{
		just_warped = false;
		return;
	}

	int dx = x - width / 2;
	int dy = y - height / 2;

	float yaw = dx * -0.002f;
	Vector<float> yawAxis = {0,1,0};
	Quaternion<float> yawQ(cos(yaw),sin(yaw)*yawAxis);
	for (int c = 0; c < 3; ++c) {
		Quaternion<float> column = Quaternion<float>(0,viewRotMat[c]);
		column = yawQ * column * yawQ.conjugate();
		viewRotMat.setColumn(column.Im(), c);
	}

	float pitch = dy * -0.002f;
	Vector<float> pitchAxis = viewRotMat[0];
	Quaternion<float> pitchQ(cos(pitch), sin(pitch) * pitchAxis);
	for (int c = 0; c < 3; ++c) {
		Quaternion<float> column = Quaternion<float>(0, viewRotMat[c]);
		column = pitchQ * column * pitchQ.conjugate();
		viewRotMat.setColumn(column.Im(), c);
	}

	glutWarpPointer(width / 2, height / 2);
}

void OnKeyboardDown(unsigned char key, int x, int y) {
	if (key == 'w') up = true;
	else if (key == 's') down = true;
	else if (key == 'a') left = true;
	else if (key == 'd') right = true;
	else if (key == 27) glutLeaveMainLoop();
}

void OnKeyboardUp(unsigned char key, int x, int y) {
	if (key == 'w') up = false;
	else if (key == 's') down = false;
	else if (key == 'a') left = false;
	else if (key == 'd') right = false;
}

void update(void) {
	dt = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - lastTime;
	if (dt > 1.0 / targetFPS) {
		lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

		Vector<float> forward = -viewRotMat[2]; forward.setValue(0,1); forward = forward.unit();
		Vector<float> rightward = viewRotMat[0]; rightward.setValue(0, 1); rightward = rightward.unit();
		if (up) eyePos += forward * moveSpeed * dt;
		else if (down) eyePos -= forward * moveSpeed * dt;
		if (right) eyePos += rightward * moveSpeed * dt;
		else if (left) eyePos -= rightward * moveSpeed * dt;

		//used if object properties change
		//glBindBuffer(GL_UNIFORM_BUFFER, uboObjs);
		//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(world), world);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glutPostRedisplay();
	}
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	rayShader.bind();

	int resolutionLoc = glGetUniformLocation(rayShader.id(), "resolution");
	glUniform2f(resolutionLoc, width, height);
	int resInvLoc = glGetUniformLocation(rayShader.id(), "resInv");
	glUniform2f(resInvLoc, 1.0/width, 1.0/height);
	int heightRatioLoc = glGetUniformLocation(rayShader.id(), "heightRatio");
	glUniform1f(heightRatioLoc, heightRatio);
	int eyeLoc = glGetUniformLocation(rayShader.id(), "eye");
	glUniform3f(eyeLoc, eyePos[0], eyePos[1], eyePos[2]);
	int viewLoc = glGetUniformLocation(rayShader.id(), "viewRot");
	float* viewRotArr = new float[9];
	viewRotMat.toColArray(viewRotArr);
	glUniformMatrix3fv(viewLoc, 1, GL_FALSE, viewRotArr);
	delete[] viewRotArr;

	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(width, 0);
	glVertex2f(width, height);
	glVertex2f(0, height);
	glEnd();

	rayShader.unbind();

	glutSwapBuffers();
}

void reshape(int w, int h) {
	width = w; height = h;
	float heightRatio = tan(M_PI*(0.5*FOV) / 180.0) / (0.5*height);

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}