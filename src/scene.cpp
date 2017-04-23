
#include "scene.h"

/**
* The constructor of bounding box using three points - one triangle.
*/
BoundingBox::BoundingBox(const std::vector<glm::vec3> &v)
{
	minDim = vec3(std::numeric_limits<float>::infinity());
	maxDim = vec3(-std::numeric_limits<float>::infinity());

	assert(v.size() == 3);
	for (int i = 0; i < 3; i++)
	{
		if (v[i].x < minDim.x)
			minDim.x = v[i].x;
		if (v[i].x > maxDim.x)
			maxDim.x = v[i].x;

		if (v[i].y < minDim.y)
			minDim.y = v[i].y;
		if (v[i].y > maxDim.y)
			maxDim.y = v[i].y;

		if (v[i].z < minDim.z)
			minDim.z = v[i].z;
		if (v[i].z > maxDim.z)
			maxDim.z = v[i].z;
	}

	dimensions.x = std::abs(maxDim.x - minDim.x);
	dimensions.y = std::abs(maxDim.y - minDim.y);
	dimensions.z = std::abs(maxDim.z - minDim.z);

	center.x = maxDim.x - (dimensions.x) / 2.f;
	center.y = maxDim.y - (dimensions.y) / 2.f;
	center.z = maxDim.z - (dimensions.z) / 2.f;
}
/**
* Method that creates bounding box from the set of another bounding boxes.
*/
void BoundingBox::create(const std::vector<BoundingBox*> &b)
{
	minDim = vec3(std::numeric_limits<float>::infinity());
	maxDim = vec3(-std::numeric_limits<float>::infinity());

	for (unsigned i = 0; i < b.size(); i++)
	{
		for (int j = 0; j < DIMENSIONS; j++) {
			if (b[i]->minDim[j] < minDim[j])
				minDim[j] = b[i]->minDim[j];
			if (b[i]->maxDim[j] > maxDim[j])
				maxDim[j] = b[i]->maxDim[j];
		}
	}
	dimensions = glm::abs(maxDim - minDim);
	
	center.x = maxDim.x - (dimensions.x) / 2.f;
	center.y = maxDim.y - (dimensions.y) / 2.f;
	center.z = maxDim.z - (dimensions.z) / 2.f;
}
/**
* The scene constructor that loads the object file, stores the geometry and constructs the BVH.
* @param sceneName name of the scene
* @triCount maximal number of triangles stored in the leaf
*/
scene::scene(const char *sceneName, unsigned triCount)
{
	backgroundColor = vec3(1.f, 1.f, 0.f);
	TRIANGLE_LEAVES_NR = triCount;
	loader = new sceneLoader(sceneName);
	loader->loadCamera(sceneName, this);
	mvpInv = getMvpInv();
	triangles = loader->getTriangles();
	materials = loader->getMaterials();
	triangleMaterials = loader->getTriMaterials();
	boxes = loader->getBoxes();
	bvh = new BVH(boxes, TRIANGLE_LEAVES_NR);
}
/**
* Method that returns the inversion matrix of the model-view-projectoin matrix.
*/
mat4 scene::getMvpInv()
{
	mat4 view = glm::lookAt(cam.pos, cam.lookAt, cam.up);
	mat4 model = mat4();
	mat4 projection = glm::perspective(cam.fovy, cam.ar, cam.near, cam.far);

	mat4 mvp = projection * view * model;
	return glm::inverse(mvp);
}
/**
* Method that calculates the intersection between the ray and the triangle. It sets the distance of the ray origin, triangle normal and the material to the ray.
* @param r ray
* @param t triangle
*/
const bool scene::rayTriangle(ray & r, triangle * t)
{
	const glm::vec3 v0 = t->v[0].vert;
	const glm::vec3 v1 = t->v[1].vert;
	const glm::vec3 v2 = t->v[2].vert;
	
	glm::vec3 e1 = v1 - v0;
	glm::vec3 e2 = v2 - v0;
	glm::vec3 pVec = glm::cross(r.dir, e2);
	float det = dot(e1, pVec);
	if (det == 0)
		return false;
	float invDet = 1 / det;
	glm::vec3 tVec = r.orig - v0;
	float u = glm::dot(tVec, pVec) * invDet;
	if (u < 0 || u > 1)
		return false;
	glm::vec3 qVec = glm::cross(tVec, e1);
	float v = glm::dot(r.dir, qVec) * invDet;
	if (v < 0 || u + v > 1)
		return false;
	float tNew = glm::dot(e2, qVec) * invDet;

	if (tNew < r.t)
	{
		r.t = tNew;
		r.norm = normalize(cross(e1, e2));
		r.mat = materials[triangleMaterials[t->index]];
		return true;
	}
	return false;
}
/**
* Traversing the BVH. Returns true if the ray intersected the scene.
* @param node the current testing node, pointer to KDNode
* @param r casted ray
*/
const bool scene::castRay (KDNode *node, ray & r)
{
	if (node->intersect(r)) {
		if (!node->data.size()) {
			bool left = castRay(node->left, r);
			bool right = castRay(node->right, r);
			return left || right;
		}
		else {
			for (unsigned i = 0; i < node->data.size(); i++) {
				rayTriangle(r, node->data[i]->t);
			}
		}
	}
	return r.t < std::numeric_limits<float>::infinity();
}
/**
* Returns true if the object is in the shadow - between it and the light source is another object.
* @param p 3D point in the scene
* @param l the light source
*/
const bool scene::shadow (const glm::vec3 & p, const glm::vec3 & l)
{
	bool result = true;
	glm::vec3 pp = p;

	while(1) {
		glm::vec3 lp = normalize(l - p);
		ray r(pp + lp * 0.01f, lp);
		if (castRay(bvh->root, ray(r)))
		{
			bool ok = r.t < glm::distance(l, pp);
			if (!ok && glm::dot(r.norm, lp) < 0)
			{
				pp = pp + lp * (r.norm + 0.01f);
				continue;
			}
			result = ok;
		}
		break;
	}
	return result;
}
/**
* Returns color in the given point of the scene.
* @param cam camera
* @param p 3D point in the scene
* @param n normal of the surface in the point p
* @param mat material in the poin p
* @param onlyPrim bool value that indicates if only primary rays have to be casted (no shadow computing)
*/
const glm::vec3 scene::getColor(const glm::vec3 &cam, const glm::vec3 &p, const glm::vec3 &n, const material *mat, bool onlyPrim)
{
	glm::vec3 color = glm::vec3(0, 0, 0);
	glm::vec3 C = glm::normalize(cam - p);
	glm::vec3 L, R, diffuse;

	for (std::vector<light>::iterator it = lights.begin(); it != lights.end(); it ++)
	{
		light &l = *it;
		L = glm::normalize(l.position - p);
		R = glm::normalize(2 * dot(L, n) * n - L);

		if (onlyPrim || !shadow(p, l.position))
		{	
			// diffuse
			color += it->color * std::max<float>(0, dot(n, L));
		}
	}
	return color;
}
/**
* Recursive method that casts the ray and indicates the backface-frontface intersection.
* @param cam camera
* @param orig the ray origin
* @param dir the ray direction
* @param depth current recursion depth
* @param onlyPrim bool value that indicates if only primary rays have to be casted (no shadow computing)
*/
const glm::vec3 scene::getColorR (const glm::vec3 &cam, const glm::vec3 &orig, const glm::vec3 &dir, const int depth, bool onlyPrim)
{
	if (depth >= 8)
		return backgroundColor;

	ray r(orig, dir);
	if (!castRay(bvh->root, r))
	{
		return backgroundColor;
	}
	float nDotDir = glm::dot(r.norm, dir);
	glm::vec3 pt = orig + dir * r.t;
	glm::vec3 color = glm::vec3(0.f, 1.f, 0.f);

	if (nDotDir > 0)
	{
		// back face
		return getColorR(cam, pt + dir * 0.02f, dir, depth + 1, onlyPrim);
	}
	else 
	{
		// front face
		color = nDotDir < 0 ? getColor(orig, pt, r.norm, r.mat, onlyPrim) : glm::vec3(1.f, 0.f, 0.f);
	}

	return color;
}
