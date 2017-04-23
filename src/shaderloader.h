#ifndef _SHADERLOADER_H_
#define _SHADERLOADER_H_

#include <string>
#include <fstream>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <vector>
#include "assimp\ai_assert.h"
#include "assimp\postprocess.h"
#include "assimp\material.h"
#include "assimp\mesh.h"
#include "scene.h"

using namespace glm;

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
void loadScene(const string & sceneName);

#endif
