
#ifndef BVH_H
#define BVH_H

#include "KDTree.h"

struct KDNode;
struct triangle;
struct ray;
struct BoundingBox;

struct BVH
{
	KDNode *root;
	BVH(const std::vector<BoundingBox*> &b, unsigned triCount);
	~BVH()
	{
		del(root);
	}

private:
	void del(KDNode *n);
	float d(KDNode *A, KDNode *B) const;
};

#endif