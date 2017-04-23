
#ifndef _SCENE_H_
#define _SCENE_H_

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <vector>
#include <string>

#include "BVH.h"
#include "KDTree.h"
#include "sceneLoader.h"

#define DIMENSIONS 3

using namespace glm;
using namespace std;

class BVH;
class sceneLoader;
struct ray;
struct KDNode;

/**
* The struct that holds the information about vertex
*/
struct vertex
{
	glm::vec3 vert;
	float u, v;
	glm::vec3 color, normal, tangent;
};

/**
* The struct that holds the information about bounding box. 
* It is used in the KD-tree and in the BVH. Initialy, each triangle 
* has its own bounding box, which is located in the leaf of the KD-tree.
*/
struct BoundingBox
{
	glm::vec3 center, dimensions, minDim, maxDim;
	int boxIndex;
	triangle *t;

	BoundingBox() {}
	BoundingBox(const std::vector<glm::vec3> &v);
	void create(const std::vector<BoundingBox*> &b);
};
/**
* The struct that holds the information about triangle
*/
struct triangle
{
	/** Three vertices of the triangle */
	vertex v[3]; 
	int index;
	/** The normal of the surface */
	glm::vec3 norm; 
	/** The axis aligned bounding box */
	BoundingBox *box; 

	triangle() {}
	triangle(const glm::vec3 & x, const glm::vec3 & y, const glm::vec3 & z)
	{
		std::vector<glm::vec3> ve;
		ve.push_back(x);
		ve.push_back(y);
		ve.push_back(z);
		box = new BoundingBox(ve);

		v[0].vert = x;
		v[1].vert = y;
		v[2].vert = z;
		glm::vec3 xy = (y - x);
		glm::vec3 yz = (z - y);
		norm = glm::cross(xy, yz);

		box->t = this;
	}
};

/**
* The struct that holds the information about material
*/
struct material 
{
	vec3 diffuse; /** Diffuse color of the material */
	vec3 specular; /** Specular color of the material */
	float diffuseCoeficient;
	float specularCoeficient;
	float shininess;
	float transparency;
	float opacity;
	float refractIndex; /** Index of refraction */
};

/**
* The struct that holds the information about light
*/
struct light 
{
	glm::vec3 position;
	glm::vec3 color;
	/** Attenuation factor */
	float attenuation; 
	light(glm::vec3 pos, glm::vec3 col) : position(pos), color(col) {}
};

/**
* The struct that holds the information about ray
*/
struct ray
{
	glm::vec3 orig, dir, norm, dirfrac;
	/** Pointer to intersected material */
	material *mat;
	/** Parameter, which indicates the distance of the intersected object */
	float t; 
	ray (const glm::vec3 & o, const glm::vec3 & d) : orig(o), dir(d), t(std::numeric_limits<float>::infinity()) 
	{
		dirfrac.x = 1.f / d.x;
		dirfrac.y = 1.f / d.y;
		dirfrac.z = 1.f / d.z;
	}
};

/**
* The struct that holds the information about the camera
*/
struct camera
{
	glm::vec3 pos, lookAt, up;
	glm::vec2 resolution;
	float fovy, ar, near, far;
};

struct scene
{	
	/** Maximal number of triangles stored in one leaf */
	unsigned TRIANGLE_LEAVES_NR; 
	std::vector<light> lights;
	std::vector<material*> materials;
	std::vector<triangle*> triangles;
	std::vector<BoundingBox*> boxes;
	std::vector<int> triangleMaterials;
	glm::vec3 backgroundColor;
	const string sceneName;
	camera cam;
	glm::mat4 mvpInv;
	sceneLoader *loader;
	BVH *bvh;

	scene() { backgroundColor = vec3(1.f, 1.f, 0.f); }
	~scene()
	{
		delete loader;
		delete bvh;
	}
	scene(const char *sceneName, unsigned triCount);
	glm::mat4 getMvpInv();
	const glm::vec3 getColorR (const glm::vec3 &cam, const glm::vec3 &orig, const glm::vec3 &dir, const int depth, bool onlyPrim);

private:
	const glm::vec3 getColor (const glm::vec3 &cam, const glm::vec3 &p, const glm::vec3 &n, const material * mat, bool onlyPrim);
	const bool shadow (const glm::vec3 & p, const glm::vec3 & l);
	const bool castRay (KDNode *node, ray & r);
	const bool rayTriangle(ray & r2, triangle * t);
};

#endif
