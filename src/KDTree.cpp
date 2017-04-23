
#include "KDTree.h"

bool in(KDNode *n, KDNode *current);

KDNode::KDNode()
{
	left = NULL;
	right = NULL;
	parrent = NULL;
}

/**
* Creates the KDNode with A and B as its children
*/
KDNode::KDNode(KDNode *A, KDNode *B)
{
	box = new BoundingBox();
	std::vector<BoundingBox*> boxes;
	boxes.push_back(A->box);
	boxes.push_back(B->box);
	box->create(boxes);
	left = NULL;
	right = NULL;
	parrent = NULL;
	axis = 0;
	for (int i = 1; i < DIMENSIONS; i++) {
		if (box->dimensions[i] > box->dimensions[axis] && A->box->center[i] != B->box->center[axis])
			axis = i;
	}

	if (A->box->center[axis] <= box->center[axis]) {
		left = A;
		right = B;
	}
	else {
		left = B;
		right = A;
	}
	A->parrent = this;
	B->parrent = this;
	leaf = false;
}

KDTree::KDTree(std::vector<BoundingBox*> b, unsigned triCount)
{
	TRIANGLE_LEAVES_NR = triCount;
	root = build(b, 0);
}
/**
* Recursive method used for construction of the KDTree. The data are stored only in the leaves. Midpoint is chosen by arithmetic mean of bounding boxes in this subtree.
* Splitting axis is chosen by the most large dimension of the common bounding box. If the splitting causes all boxes on one side, it tries another axis. If all of 
* the possibilities cause the same issue, the node becames a leaf.
* @boxes the set of bounding boxes located in this subtree
* @depth current depth of the tree
*/
KDNode * KDTree::build(std::vector<BoundingBox*> &boxes, int depth)
{
	KDNode *node = new KDNode();
	
	node->left = NULL;
	node->right = NULL;
	node->parrent = NULL;
	
	node->leaf = false;
	
	if (boxes.size() == 0) {
		node->leaf = 1;
		return node;
	}

	node->box = new BoundingBox();
	node->box->create(boxes);

	if (boxes.size() <= TRIANGLE_LEAVES_NR)
	{
		node->data = boxes;
		node->leaf = 1;
		return node;
	}
	
	// determine the splitting axis
	float maxSize = -numeric_limits<float>::infinity();
	node->axis = 0;
	for (int i = 0; i < 3; i++)
	{
		if (node->box->dimensions[i] > maxSize) {
			maxSize = node->box->dimensions[i];
			node->axis = i;
		}
	}
	std::vector<BoundingBox*> leftSub;
	std::vector<BoundingBox*> rightSub;

	// find the midpoint
	for (int k = 0; k < 3; k++) 
	{
		float midPoint = 0.f;
		for (unsigned i = 0; i < boxes.size(); i++)
		{
			midPoint += (boxes[i]->center[(node->axis + k)%3] / boxes.size());
		}

		leftSub.clear();
		rightSub.clear();
		for (unsigned i = 0; i < boxes.size(); i++)
		{
			if (boxes[i]->center[(node->axis + k) % 3] <= midPoint)
				leftSub.push_back(boxes[i]);
			else
				rightSub.push_back(boxes[i]);
		}

		// if one of the two boxes is empty, find another splitting axis
		if (leftSub.size() && rightSub.size())
			break;
	}

	// if any box still remains empty, all of the boxes are exactly at the same position - return
	if (leftSub.size() == 0 || rightSub.size() == 0) {
		node->data = boxes;
		node->leaf = 1;
		return node;
	}

	node->left = build(leftSub, depth + 1);
	node->right = build(rightSub, depth + 1);
	node->left->parrent = node;
	node->right->parrent = node;

	return node;
}
/** Returns true if the node A is by all its volume located in B */
bool in(KDNode *A, KDNode *B) {

	for (int i = 0; i < DIMENSIONS; i++)
	{
		if (A->box->maxDim[i] > B->box->maxDim[i] || A->box->minDim[i] < B->box->minDim[i])
			return false;
	}
	return true;
}

/**
* Adds the leaf in the KDTree. 
* @param n inserting node
*/
void KDTree::add(KDNode *n)
{
	n->leaf = true;
	KDNode *current = root;
	while (current && !current->leaf && in(n, current))
	{
		if (n->box->center[current->axis] <= current->box->center[current->axis])
			current = current->left;
		else
			current = current->right;
	}
	
	// root is empty
	if (!current)
	{
		n->leaf = true;
		root = n;
		return;
	}
	// current is leaf or n is not all inside the current 
	else {
		KDNode *parrent = current->parrent;
		KDNode *par = new KDNode(n, current);
		par->parrent = parrent; 
		if (parrent) {
			if (parrent->left == current)
				parrent->left = par;
			else
				parrent->right = par;
		}
		else
			root = par;
		return;
	}
}
/**
* Removes the leaf node of the tree.
* @param n removing node
*/
void KDTree::remove(KDNode *n)
{
	assert(n->leaf);
	if (!n->parrent) {
		root = NULL;
		return;
	}
	if (n->parrent->left == n) {
		n->parrent->right->parrent = n->parrent->parrent;
		if (n->parrent->parrent) {
			if (n->parrent->parrent->left == n->parrent) {
				n->parrent->parrent->left = n->parrent->right;
			}
			else {
				n->parrent->parrent->right = n->parrent->right;
			}
		}
		else
			root = n->parrent->right;
	}
	else {
		n->parrent->left->parrent = n->parrent->parrent;
		if (n->parrent->parrent) {
			if (n->parrent->parrent->left == n->parrent) {
				n->parrent->parrent->left = n->parrent->left;
			}
			else {
				n->parrent->parrent->right = n->parrent->left;
			}
		}
		else
			root = n->parrent->left;
	}
}
/**
* Recursively computes the nuber of triangles
*/
const unsigned KDNode::getTotalNodeCount() const
{
	return (left ? left->getTotalNodeCount() : 0)
		+ (right ? right->getTotalNodeCount() : 0)
		+ data.size();
}
/**
* Returns arbitrary leaf node. In this case it returns the node located the most left.
*/
KDNode *KDTree::getAnyElement() const
{
	KDNode *n = root;
	while (!n->leaf) {
		n = n->left;
	}
	return n;
}
/**
* Finds the nearest neighbor to the given node. It uses the stack instead of recursion.
* @param n query node
*/
KDNode *KDTree::findBestMatch(KDNode *n) const
{	
	KDNode *node = root;
	std::stack<KDNode*> stack;
	while (node && !node->leaf)
	{
		if (n->box->center[node->axis] <= node->box->center[node->axis]) {
			stack.push(node->right);
			node = node->left;
		}
		else {
			stack.push(node->left);
			node = node->right;
		}
		assert(node);
	}

	KDNode *best = node;
	float lowerBound = 0.f;
	float curr = numeric_limits<float>::infinity();
	//int cnt = 0;
	while (!stack.empty()) {
		//cnt++;
		KDNode *cluster = stack.top();
		stack.pop();

		if (!cluster)
			continue;

		lowerBound = 0.f;
		for (int i = 0; i < DIMENSIONS; i++) {
			float a = glm::max(0.f, glm::max(n->box->center[i] - (cluster->box->center[i] + 0.5f * cluster->box->dimensions[i]),
				(cluster->box->center[i] - 0.5f * cluster->box->dimensions[i]) - n->box->center[i]));
			lowerBound += (a * a);
		}
		
		if (lowerBound >= curr || cluster == n)
			continue;

		lowerBound = 0.f;
		for (int i = 0; i < DIMENSIONS; i++) {
			float a = glm::abs(n->box->center[i] - cluster->box->center[i]);
			lowerBound += (a * a);
		}
		
		if (cluster->leaf) {
			if (lowerBound >= curr)
				continue;

			best = cluster;
			curr = lowerBound;
		}
		else {
			if (n->box->center[cluster->axis] <= cluster->box->center[cluster->axis])
			{
				stack.push(cluster->right);
				stack.push(cluster->left);
			}
			else {
				stack.push(cluster->left);
				stack.push(cluster->right);
			}
		}
	}
	return best;
}
/**
* Intersection between the ray and the bounding box.
* @param r ray
* @return true if the ray intersects bounding box of the node
*/
const bool KDNode::intersect (ray & r) const
{
	float tNear = -std::numeric_limits<float>::infinity();
	float tFar = std::numeric_limits<float>::infinity();

	float t1, t2;
	for (int i = 0; i < 3; i++) {
		// ray is parallel to X-plane
		if (r.dir[i] == 0) {
			if (r.orig[i] < box->minDim[i] || r.orig[i] > box->maxDim[i])
				return false;
		}
		// ray is not parallel to X-plane
		else {
			t1 = (box->minDim[i] - r.orig[i]) * r.dirfrac[i];
			t2 = (box->maxDim[i] - r.orig[i]) * r.dirfrac[i];

			if (t1 > t2) {
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}
			if (t1 > tNear)
				tNear = t1;

			if (t2 < tFar)
				tFar = t2;

			if (tNear > tFar)
				return false;
			if (tFar < 0)
				return false;
		}
	}
	return true;
}