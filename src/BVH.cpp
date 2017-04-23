
#include "BVH.h"
#include <windows.h>

void printTree(KDNode *root, int depth)
{
	if (!root)
		return;

	for (int i = 0; i < depth; i++)
		printf("-");
	printf("C [%f, %f, %f], D [%f, %f, %f]\n", root->box->center[0], root->box->center[1], root->box->center[2], root->box->dimensions[0],
		root->box->dimensions[1], root->box->dimensions[2]);
	printf("total nodes: %u\n", root->getTotalNodeCount());
	//printTree(root->left, depth + 1);
	//printTree(root->right, depth + 1);
}
/**
* Construction of bounding volume hierarchy using the "local ordering" method described in the article "Fast agglomerative clustering for rendering".
* @param b set of the bounding boxes
* @param triCount maximal number of triangles in the single leaf
*/
BVH::BVH(const std::vector<BoundingBox*> &b, unsigned triCount)
{
	LARGE_INTEGER frequency, t1, t2;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);

	KDTree *tree = new KDTree(b, triCount);
	KDNode *A = tree->getAnyElement();
	KDNode *B = tree->findBestMatch(A);
	
	while (!tree->root->leaf) {
		KDNode *C = tree->findBestMatch(B);
		if (A == C || d(A,B) <= d(B,C)) {
			assert(A != B);
			tree->remove(A);
			tree->remove(B);
			A = new KDNode(A, B);
			tree->add(A);
			B = tree->findBestMatch(A);
		}
		else {
			A = B;
			B = C;
		}
	}
	root = tree->root;

	QueryPerformanceCounter(&t2);
	double elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("%f", elapsedTime / 1000.f);
}
void BVH::del(KDNode *n)
{
	if (!n)
		return;
	del(n->left);
	del(n->right);
	delete n;
}
/**
* Computes the squared distance between the centers of the bounding boxes A and B.
*/
float BVH::d(KDNode *A, KDNode *B) const
{
	float dist = 0.f;
	for (int i = 0; i < 3; i++) {
		float a = glm::abs(A->box->center[i] - B->box->center[i]);
		dist += (a * a);
	}
	return dist;
}
