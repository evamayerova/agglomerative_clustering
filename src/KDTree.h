
#ifndef KDTREE_H
#define KDTREE_H

#include <glm\glm.hpp>
#include <vector>
#include <stack>
#include <algorithm>
#include "scene.h"

struct triangle;
struct scene;
struct ray;
struct BoundingBox;

struct KDNode
{
	/** Bounding box of all triangles in this subtree */
	BoundingBox *box;
	/** Splitting axis */
	int axis;
	KDNode *left, *right, *parrent;
	/** If the node is actualy leaf, it contains the data as the set of bounding boxes */
	std::vector<BoundingBox*> data;
	/** Auxiliary variable used only for the construction of BVH. Indicates if the node is already merged with some else. */
	bool leaf;
	
	KDNode();
	KDNode(KDNode *A, KDNode *B);
	const bool intersect (ray & r) const;
	const unsigned getTotalNodeCount() const;
};

struct KDTree
{
	KDNode *root;
	/** Maximal number of triangles in single leaf */
	unsigned TRIANGLE_LEAVES_NR;

	KDTree(std::vector<BoundingBox*> b, unsigned triCount);
	void add(KDNode *n);
	void remove(KDNode *n);
	KDNode *getAnyElement() const;
	KDNode *findBestMatch(KDNode *n) const;

private:
	KDNode *build(std::vector<BoundingBox*> &boxes, int depth);
};

#endif
