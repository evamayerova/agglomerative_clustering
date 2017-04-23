#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "scene.h"
#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\cimport.h>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <fstream>
#include <sstream>

struct scene;
struct triangle;
struct material;
struct BoundingBox;

struct mesh {
	mesh();
};

class sceneLoader {

	std::vector<material*> materials;
	std::vector<triangle*> triangles;
	std::vector<BoundingBox*> boxes;
	std::vector<int> triangleMaterials;
	unsigned int loadTexture(const char *fileName);

public:
	sceneLoader(const char *fileName);
	~sceneLoader();
	void loadCamera(const char *fileName, scene * s);
	const std::vector<mesh*>& getMeshes();
	const std::vector<material*>& getMaterials();
	const std::vector<triangle*>& getTriangles();
	const std::vector<int>& getTriMaterials();
	const std::vector<BoundingBox*>& getBoxes();
};

#endif
