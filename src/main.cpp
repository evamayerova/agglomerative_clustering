
#include <stdio.h>
#include <stdlib.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "scene.h"
#include "shaderloader.h"

//#define TESTING
//#define BUILDING
#define DRAWING

using namespace glm;
using namespace std;

GLuint textureID;
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint texID;
GLuint programID;

GLFWwindow *window;
scene *sc;
std::string sceneName;
unsigned triangleCount;
unsigned resolution[2];
bool onlyPrimary;

const short scenesNr = 1;
const char* scenes[] = { /*"A10", "armadillo", "city", "city2", "conference", "fforest", "park", "sibenik", "teapots",*/ "yoda" };
const int tests[] = { 4, 8, 16, 4, 8, 16 };
float buildTimes[3][10];
float drawingTimes[6][10];

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

static const GLfloat g_uv_buffer_data[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f
};

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int init()
{
	if (!glfwInit()) {
		throw "cannot initialize GLFW";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwSetErrorCallback(error_callback);
	window = glfwCreateWindow(resolution[0], resolution[1], "My Title", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		throw "cannot create the window";
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		return 1;
	}

	return 0;
}

float * getImage(const int & width, const int & height)
{
	float * img = new float[width * height * 4];

	/*sc->lights.clear();
	sc->lights.push_back(light(sc->cam.pos + sc->cam.up * 2.f, vec3(1.f, 1.f, 1.f)));
	sc->lights.push_back(light(vec3(100000.f, 50000.f, -100000.f), vec3(0.f, 1.f, 0.f)));
	sc->lights.push_back(light(vec3(0.f, 100000.f, -100000.f), vec3(1.f, 0.f, 0.f)));
	sc->lights.push_back(light(vec3(-100000.f, 50000.f, -100000.f), vec3(0.f, 0.f, 1.f)));*/

	float dx = 2.f / width;
	float dy = 2.f / height;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float NDCx = dx * (x + 0.5f) - 1;
			float NDCy = dy * (y + 0.5f) - 1;

			vec4 nearPoint = sc->mvpInv * vec4(NDCx, NDCy, -1, 1);
			vec4 farPoint = sc->mvpInv * vec4(NDCx, NDCy, 1, 1);

			vec3 near = vec3(nearPoint.x, nearPoint.y, nearPoint.z) * (1.f / nearPoint.w);
			vec3 far = vec3(farPoint.x, farPoint.y, farPoint.z) * (1.f / farPoint.w);

			vec3 rayOrig = near;
			vec3 rayDir = normalize(far - near);

			vec3 color = sc->getColorR(rayOrig, rayOrig, rayDir, 0, onlyPrimary);

			img[4 * (y * width + x)] = color.r;
			img[4 * (y * width + x) + 1] = color.g;
			img[4 * (y * width + x) + 2] = color.b;
			img[4 * (y * width + x) + 3] = 1.f;
		}
	}

	return img;
}

#include <Windows.h>
void display(int sceneIdx, int test)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	if (sc)
		delete sc;

	sc = new scene(scenes[sceneIdx], 8 * tests[test]);
	sc->lights.clear();
	sc->lights.push_back(light(sc->cam.pos + sc->cam.up * 2.f, vec3(1.f, 1.f, 1.f)));
	onlyPrimary = test >= 3 ? false : true;
	if (!sc->bvh->root)
		return;

	LARGE_INTEGER frequency, t1, t2;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);

	float * img = getImage(width, height);

	QueryPerformanceCounter(&t2);
	double elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
#ifdef DRAWING
	cout << " \\fi " << elapsedTime / 1000.f;
#endif
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// clear the screen to dark blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	texID = glGetUniformLocation(programID, "texSampler");
	glUseProgram(programID);

	glViewport(0, 0, width, height);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	delete [] img;
}

void display()
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	float * img = getImage(width, height);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// clear the screen to dark blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	texID = glGetUniformLocation(programID, "texSampler");
	glUseProgram(programID);

	glViewport(0, 0, width, height);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	delete img;
}

void loadParameters()
{
	std::ifstream in("parameters.txt");
	std::string line;

	while (std::getline(in, line)) {
		std::istringstream iss(line);
		std::string a;
		iss >> a;
		if (strcmp(a.c_str(), "sceneName") == 0) {
			iss >> a;
			sceneName = a;
		}
		else if (strcmp(a.c_str(), "triangleCount") == 0) {
			unsigned x;
			iss >> x;
			triangleCount = x;
		}
		else if (strcmp(a.c_str(), "resolution") == 0) {
			unsigned x, y;
			iss >> x >> y;
			resolution[0] = x;
			resolution[1] = y;
		}
		else if (strcmp(a.c_str(), "onlyPrimaryRays") == 0) {
			bool x;
			iss >> x;
			onlyPrimary = x;
		}
	}
}

int main(int argc, char * argv)
{

#ifndef TESTING
	loadParameters();
	sc = new scene(sceneName.c_str(), triangleCount);
	sc->lights.clear();
	sc->lights.push_back(light(sc->cam.pos + sc->cam.up * 2.f, vec3(1.f, 1.f, 1.f)));
#endif
	/*
	triangle *t1 = new triangle(vec3(8.f, 2.f, 0.f), vec3(8.1f, 2.f, 0.f), vec3(8.f, 2.1f, 0.f));
	triangle *t2 = new triangle(vec3(5.f, 1.f, 0.f), vec3(5.1f, 1.f, 0.f), vec3(5.f, 1.1f, 0.f));
	triangle *t3 = new triangle(vec3(4.f, 1.f, 0.f), vec3(4.1f, 1.f, 0.f), vec3(4.f, 1.1f, 0.f));
	triangle *t4 = new triangle(vec3(7.f, 4.f, 0.f), vec3(7.1f, 4.f, 0.f), vec3(7.f, 4.1f, 0.f));
	triangle *t5 = new triangle(vec3(2.f, 2.f, 0.f), vec3(2.1f, 2.f, 0.f), vec3(2.f, 2.1f, 0.f));
	triangle *t6 = new triangle(vec3(1.f, 5.f, 0.f), vec3(1.1f, 5.f, 0.f), vec3(1.f, 5.1f, 0.f));
	triangle *t7 = new triangle(vec3(4.f, 0.f, 0.f), vec3(4.1f, 0.f, 0.f), vec3(4.f, 0.1f, 0.f));
	triangle *t8 = new triangle(vec3(4.f, 3.f, 0.f), vec3(4.1f, 3.f, 0.f), vec3(4.f, 3.1f, 0.f));
	triangle *t9 = new triangle(vec3(4.f, 7.f, 0.f), vec3(4.1f, 7.f, 0.f), vec3(4.f, 7.1f, 0.f));
	*/
	/*
	triangle *t1 = new triangle(vec3(0.f, 0.f, 0.f), vec3(0.1f, 0.f, 0.f), vec3(0.f, 0.1f, 0.f));
	triangle *t2 = new triangle(vec3(1.f, 1.f, 0.f), vec3(1.1f, 1.f, 0.f), vec3(1.f, 1.1f, 0.f));
	triangle *t3 = new triangle(vec3(2.f, 2.f, 0.f), vec3(2.1f, 2.f, 0.f), vec3(2.f, 2.1f, 0.f));
	triangle *t4 = new triangle(vec3(1.f, 3.f, 0.f), vec3(1.1f, 3.f, 0.f), vec3(1.f, 3.1f, 0.f));
	triangle *t5 = new triangle(vec3(2.f, 4.f, 0.f), vec3(2.1f, 4.f, 0.f), vec3(2.f, 4.1f, 0.f));
	triangle *t6 = new triangle(vec3(3.f, 5.f, 0.f), vec3(3.1f, 5.f, 0.f), vec3(3.f, 5.1f, 0.f));
	triangle *t7 = new triangle(vec3(4.f, 1.f, 0.f), vec3(4.1f, 1.f, 0.f), vec3(4.f, 1.1f, 0.f));
	triangle *t8 = new triangle(vec3(4.f, 3.f, 0.f), vec3(4.1f, 3.f, 0.f), vec3(4.f, 3.1f, 0.f));
	triangle *t9 = new triangle(vec3(4.f, 6.f, 0.f), vec3(4.1f, 6.f, 0.f), vec3(4.f, 6.1f, 0.f));
	triangle *t10 = new triangle(vec3(5.f, 5.f, 0.f), vec3(5.1f, 5.f, 0.f), vec3(5.f, 5.1f, 0.f));
	triangle *t11 = new triangle(vec3(6.f, 4.f, 0.f), vec3(6.1f, 4.f, 0.f), vec3(6.f, 4.1f, 0.f));
	triangle *t12 = new triangle(vec3(6.f, 2.f, 0.f), vec3(6.1f, 2.f, 0.f), vec3(6.f, 2.1f, 0.f));
	triangle *t13 = new triangle(vec3(7.f, 3.f, 0.f), vec3(7.1f, 3.f, 0.f), vec3(7.f, 3.1f, 0.f));
	triangle *t14 = new triangle(vec3(7.f, 1.f, 0.f), vec3(7.1f, 1.f, 0.f), vec3(7.f, 1.1f, 0.f));
	triangle *t15 = new triangle(vec3(8.f, 0.f, 0.f), vec3(8.1f, 0.f, 0.f), vec3(8.f, 0.1f, 0.f));
	triangle *t16 = new triangle(vec3(8.1f, 0.f, 0.f), vec3(8.1f, 0.1f, 0.f), vec3(8.f, 0.1f, 0.f));
	triangle *t17 = new triangle(vec3(8.f, 1.f, 0.f), vec3(8.1f, 1.f, 0.f), vec3(8.f, 1.1f, 0.f));
	triangle *t18 = new triangle(vec3(8.f, 2.f, 0.f), vec3(8.1f, 2.f, 0.f), vec3(8.f, 2.1f, 0.f));

	sc->boxes.clear();
	sc->boxes.push_back(t1->box);
	sc->boxes.push_back(t2->box);
	sc->boxes.push_back(t3->box);
	sc->boxes.push_back(t4->box);
	sc->boxes.push_back(t5->box);
	sc->boxes.push_back(t6->box);
	sc->boxes.push_back(t7->box);
	sc->boxes.push_back(t8->box);
	sc->boxes.push_back(t9->box);

	sc->boxes.push_back(t10->box);
	sc->boxes.push_back(t11->box);
	sc->boxes.push_back(t12->box);
	sc->boxes.push_back(t13->box);
	sc->boxes.push_back(t14->box);
	sc->boxes.push_back(t15->box);
	sc->boxes.push_back(t16->box);
	sc->boxes.push_back(t17->box);
	sc->boxes.push_back(t18->box);

	sc->bvh = new BVH(sc->boxes);
	sc->cam.pos = normalize(vec3(4.f, 0.f, 20.f));
	sc->cam.lookAt = normalize(vec3(4.f, 0.f, 0.f));
	sc->cam.up = vec3(0.f, 1.f, 0.f);
	sc->mvpInv = sc->getMvpInv();
	*/
	//sc->lights.push_back(light(vec3(4.f, 1.f, 5.f), vec3(1.f, 1.f, 1.f)));
	//sc->lights.push_back(light(vec3(100000.f, 50000.f, -100000.f), vec3(0.f, 1.f, 0.f)));
	//sc->lights.push_back(light(vec3(0.f, 100000.f, -100000.f), vec3(1.f, 0.f, 0.f)));
	//sc->lights.push_back(light(vec3(-100000.f, 50000.f, -100000.f), vec3(0.f, 0.f, 1.f)));

#ifdef TESTING
	resolution[0] = 512.f;
	resolution[1] = 512.f;
#endif

	if (init() == 1)
		return 1;

	programID = LoadShaders("vs.vert", "fs.frag");
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);


#ifdef TESTING
#ifdef BUILDING
	//int i = 0;
	//int j = 0;
	//cout << "\\title{BVH building times (ms)}" << endl;
	cout << "\\begin{tabular}{|c||c|c|c|}" << endl <<
		"\\hline" << endl <<
		" & " << tests[0] << " & " << tests[1] << " & " << tests[2] << " \\\\" << endl << "\\hline \\hline" << endl;
	for (int i = 0; i < scenesNr; i++)
	{
		cout << scenes[i] ;
		for (int j = 0; j < 3; j++)
		{
			cout << " & ";
			display(i, j);
			glfwPollEvents();
			glfwSwapBuffers(window);
			
		}
		cout << " \\\\" << endl << "\\hline" << endl;
	}
	cout << "\\end{tabular}" << endl;


	for (int i = 0; i < scenesNr; i ++)
	{
		cout << "\\begin{table}\\centering" << endl;
		cout << "\\caption{" << scenes[i] << " drawing times}" << endl;
		cout << "\\begin{tabular}{|c||c|c|c|}" << endl;
		cout << "\\hline" << endl << "Triangles count in leaves & 4 & 8 & 16 \\\\" << endl << "\\hline \\hline" << endl;
		for (int j = 0; j < 2; j++)
		{
			cout << "Primary rays" ;
			for (int k = 0; k < 3; k ++)
			{
				cout << " & ";
				display(i,j);
				glfwPollEvents();
				glfwSwapBuffers(window);
			}
			cout << " \\\\" << "\\hline" << endl;
		}
	}
#endif
#ifdef DRAWING
	for (int i = 0; i < scenesNr; i++)
	{
		cout << "\\begin{table}\\centering" << endl;
		cout << "\\caption{" << scenes[i] << " drawing times (s)}" << endl;
		cout << "\\begin{tabular}{|c||c|c|c|}" << endl;
		cout << "\\hline" << endl << "Triangle number in leaves & 4 & 8 & 16 \\\\" << endl << "\\hline" << endl;
		for (int j = 0; j < 2; j++)
		{
			if (j == 0) cout << "Primary rays";
			else		cout << "Shadow rays";
			for (int k = 0; k < 3; k++)
			{
				cout << " & \\iffalse ";
				display(i, j*3 + k);
				glfwPollEvents();
				glfwSwapBuffers(window);
			}
			cout << " \\\\"<< endl;
		}
		cout << "\\hline" << endl << "\\end{tabular}" << endl << "\\end{table}" << endl << endl;
	}

#endif
#endif
#ifndef TESTING
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
#endif
	delete sc;
	glfwTerminate();
	return 0;
}