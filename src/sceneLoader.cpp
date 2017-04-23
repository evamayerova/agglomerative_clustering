#include "sceneLoader.h"
/**
* Stores the data from the .obj file to the arrays of triangles, bounding boxes and materials.
*/
sceneLoader::sceneLoader(const char *fileName)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(std::string("data/") + fileName + "/" + fileName + ".obj",
		aiProcess_PreTransformVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_ValidateDataStructure |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_FindInvalidData);

	if (!scene)
	{
		printf("The file wasn't successfuly opened\n");
		return;
	}

	for (unsigned i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		for (unsigned j = 0; j < mesh->mNumFaces; j++) {

			triangle *t = new triangle;
			unsigned v = mesh->mFaces[j].mIndices[0];
			t->v[0].vert = vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
			t->v[0].normal = mesh->HasNormals() ? vec3(mesh->mNormals[v].x) : vec3();
			vec2 uv = mesh->HasTextureCoords(0) ? vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y) : vec2();
			t->v[0].u = uv.x;
			t->v[0].v = uv.y;

			v = mesh->mFaces[j].mIndices[1];
			t->v[1].vert = vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
			t->v[1].normal = mesh->HasNormals() ? vec3(mesh->mNormals[v].x) : vec3();
			uv = mesh->HasTextureCoords(0) ? vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y) : vec2();
			t->v[1].u = uv.x;
			t->v[1].v = uv.y;

			v = mesh->mFaces[j].mIndices[2];
			t->v[2].vert = vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
			t->v[2].normal = mesh->HasNormals() ? vec3(mesh->mNormals[v].x) : vec3();
			uv = mesh->HasTextureCoords(0) ? vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y) : vec2();
			t->v[2].u = uv.x;
			t->v[2].v = uv.y;

			triangle *t2 = new triangle(vec3(t->v[0].vert), vec3(t->v[1].vert), vec3(t->v[2].vert));

			triangleMaterials.push_back(mesh->mMaterialIndex);
			t2->index = triangles.size();
			triangles.push_back(t2);
			t2->box->boxIndex = boxes.size();
			boxes.push_back(t2->box);
		}
	}

	for (unsigned i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial *mat = scene->mMaterials[i];
		material m;
		aiColor3D color;
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		m.diffuse = vec3(color.r, color.g, color.b);
		mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		m.specular = vec3(color.r, color.g, color.b);
		mat->Get(AI_MATKEY_SHININESS, m.shininess);
		mat->Get(AI_MATKEY_OPACITY, m.opacity);
		mat->Get(AI_MATKEY_REFRACTI, m.refractIndex);
		materials.push_back(&m);
	}
}
sceneLoader::~sceneLoader()
{
	for (std::vector<triangle*>::iterator it = triangles.begin(); it != triangles.end(); ++it)
	{
		delete (*it);
	}
	triangles.clear();
	for (std::vector<BoundingBox*>::iterator it = boxes.begin(); it != boxes.end(); ++it)
	{
		delete (*it);
	}
	boxes.clear();
	materials.clear();
	//for (unsigned i = 0; i < materials.size(); i++)
	//	delete materials[i];
	//materials.clear();
	//for (unsigned i = 0; i < triangles.size(); i++)
	//	delete triangles[i];
	//triangles.clear();
	//for (unsigned i = 0; i < boxes.size(); i++)
	//	delete boxes[i];
	//boxes.clear();
}
/**
* Loads the information from .view files.
*/
void sceneLoader::loadCamera(const char *fileName, scene * s)
{
	std::ifstream in(std::string("data/") + fileName + "/" + fileName + ".view");
	std::string line;

	s->cam.near = 0.1f;
	s->cam.far = 100.f;
	s->cam.resolution = vec2(512.f, 512.f);

	while (std::getline(in, line)) {
		std::istringstream iss(line);
		std::string a;
		iss >> a;
		if (strcmp(a.c_str(), "#from") == 0) {
			float x, y, z;
			iss >> x >> y >> z;
			s->cam.pos = vec3(x, y, z);
		}
		else if (strcmp(a.c_str(), "#at") == 0) {
			float x, y, z;
			iss >> x >> y >> z;
			s->cam.lookAt = vec3(x, y, z);
		}
		else if (strcmp(a.c_str(), "#up") == 0) {
			float x, y, z;
			iss >> x >> y >> z;
			s->cam.up = vec3(x, y, z);
		}
		else if (strcmp(a.c_str(), "fieldOfView") == 0) {
			float x;
			iss >> x;
			s->cam.fovy = x;
		}
		else if (strcmp(a.c_str(), "#resolution") == 0) {
			float x, y;
			iss >> x >> y;
			s->cam.resolution = vec2(x, y);
			s->cam.ar = x / y;
		}
	}
	in.close();
	
	if (strcmp(fileName, "conference") == 0 || strcmp(fileName, "city2") == 0)
		return;

	// new viewpoints
	std::ifstream in2(std::string("data/") + fileName + "/" + fileName + "_B.view");
	std::getline(in2, line);
	std::istringstream iss(line);
	std::string str;
	
	float x, y, z;
	// cam position
	iss >> str;
	iss >> x >> y >> z;
	s->cam.pos = vec3(x, y, z);
	// look at vector
	iss >> str;
	iss >> x >> y >> z;
	s->cam.lookAt = glm::normalize(vec3(x, y, z) - s->cam.pos);
	// up vector
	iss >> str;
	iss >> x >> y >> z;
	s->cam.up = vec3(x, y, z);
	// angle in radians
	iss >> str;
	iss >> x;
	s->cam.fovy = x;

	//s->cam.pos = s->cam.pos + 100.f * (s->cam.lookAt - s->cam.pos);
	in2.close();
	
}
/**
* Returns set of loaded materials.
*/
const std::vector<material*>& sceneLoader::getMaterials()
{
	return materials;
}
/**
* Returns set of triangle indices of the materials.
*/
const std::vector<int>& sceneLoader::getTriMaterials()
{
	return triangleMaterials;
}
/**
* Returns set of loaded triangles.
*/
const std::vector<triangle*>& sceneLoader::getTriangles()
{
	return triangles;
}
/**
* Returns set of created bounding boxes.
*/
const std::vector<BoundingBox*>& sceneLoader::getBoxes()
{
	return boxes;
}
