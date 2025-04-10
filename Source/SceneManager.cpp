///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

 /***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// load textures from texture folder with appropriate tags
	bReturn = CreateGLTexture("./textures/lamp_glass.png", "lamp_glass");
	bReturn = CreateGLTexture("./textures/base_ground.png", "ground_1");
	bReturn = CreateGLTexture("./textures/bench_structure.png", "bench_struct");
	bReturn = CreateGLTexture("./textures/bench_body.png","bench_body");
	bReturn = CreateGLTexture("./textures/fence_support.png", "fence_support");
	bReturn = CreateGLTexture("./textures/fence_bars.png", "fence_bars");
	bReturn = CreateGLTexture("./textures/ground_2.png", "ground_2");
	bReturn = CreateGLTexture("./textures/bark_brown_02_diff_4k.jpg","wood");



	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// default plastic material
	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.diffuseColor = glm::vec3(0.8f, 0.4f, 0.8f);
	plasticMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.shininess = 2.0;
	plasticMaterial.tag = "plastic";
	m_objectMaterials.push_back(plasticMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// ambient and directional lighting
	m_pShaderManager->setVec3Value("directionalLight.direction", 0.2f, -0.2f, -0.5f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.9f, 0.9f, 0.9f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);


	m_pShaderManager->setVec3Value("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("spotLight.diffuse", 1.0f, 1.0f, 0.0f);
	m_pShaderManager->setVec3Value("spotLight.specular", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("spotLight.constant", 0.2f);
	m_pShaderManager->setFloatValue("spotLight.linear", 0.0f);
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.0f);
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(70.0f)));
	m_pShaderManager->setFloatValue("spotLight.outerCutOff",glm::cos(glm::radians(110.0f)));
	m_pShaderManager->setVec3Value("spotLight.position", glm::vec3(0.0f, 6.0f, -60.0f));
	m_pShaderManager->setVec3Value("spotLight.direction", glm::vec3(0.0f, 0.0f, -1.0f));
	m_pShaderManager->setBoolValue("spotLight.bActive", true);

}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	LoadSceneTextures();
	DefineObjectMaterials();
	SetupSceneLights();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	//m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadSphereMesh();
}

/********************************************************
* LampPost
* 
* This method draws a lamp post attached to the ground at a given postition 
* 
* this method also has an option for showing the wireframe lines
*/
void SceneManager::LampPost(glm::vec3 translation, bool use_lines) {
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); 
	glm::vec4 color2 = glm::vec4(1.0f, 0.855f, 0.725f, 0.9f);
	glm::vec4 line_color = glm::vec4(0, 1, 1, 1);
	SetShaderColor(color.x, color.y, color.z, color.a);

	//cylander for main body
	SetTransformations(
		glm::vec3(0.5f, 10.0f, 0.5f), // this size seems good
		0, 0, 0, // no rotation needed
		translation // use unaltered translation to pout the lamp bottom where the function caller decides
	);
	m_basicMeshes->DrawCylinderMesh();
	if (use_lines) { // when using lines we should switch render color for just a second
		SetShaderColor(line_color.x,line_color.y,line_color.z,line_color.a);
		m_basicMeshes->DrawCylinderMeshLines();
		SetShaderColor(color.x, color.y, color.z, color.a);
	}
	
	//cone 1 for top
	SetTransformations(
		glm::vec3(1.0f, 1.0f, 1.0f), // default size was great for this
		0, 0, 0, // no rotation needed
		translation + glm::vec3(0.0f, 12.0f, 0.0f) // this should be above the users given position because the user gives the ground and the top of the lamp post is up prety high
	);
	m_basicMeshes->DrawConeMesh(); // reusing color from the cylander
	if (use_lines) {
		SetShaderColor(line_color.x, line_color.y, line_color.z, line_color.a);
		m_basicMeshes->DrawConeMeshLines();
	}

	//cone 2 for glass
	SetTransformations(
		glm::vec3(1.0f, 4.0f, 1.0f), // elongate the cone for the glass
		180.0f, 0, 0, // flip it upside down
		translation + glm::vec3(0.0f, 12.0f, 0.0f) // move to same position as top cone
	);
	//SetShaderColor(color2.x, color2.y, color2.z, color2.a); // uses custom glass color with some transparency
	SetShaderTexture("lamp_glass");
	m_basicMeshes->DrawConeMesh();
	if (use_lines) {
		SetShaderColor(line_color.x, line_color.y, line_color.z, line_color.a);
		m_basicMeshes->DrawConeMeshLines();
	}

}

/*********************************************************
* Bench()
* 
* this method will perform all transforms required to make a bench at a given position with a given orientation
* 
*/
void SceneManager::Bench(glm::vec3 pos,bool facing_right) {
	// TODO - fix blending issue where texture drawn first will clip later drawn tectures

	//seat
	SetShaderTexture("bench_body");
	SetTransformations(
		glm::vec3(1.0f, 1.0f, 2.0f),
		0, 0, 0,
		pos + glm::vec3(0.1f, 1.7f, 0)
	);
	m_basicMeshes->DrawPlaneMesh();
	
	//back 
	float facing = 0.0f;
	if (facing_right) {
		facing = 180.0f;
	}
	SetTransformations(
		glm::vec3(1.0f, 1.0f, 2.0f),
		0, facing , 90.0f,
		pos + glm::vec3(1.7f*(facing_right?-1:1), 3.0f, 0)
	);
	m_basicMeshes->DrawPlaneMesh();

	// structure left
	SetShaderTexture("bench_struct");
	SetTransformations(
		glm::vec3(2.0f, 2.0f, 2.0f),
		90.0f, facing, 0,
		pos + glm::vec3(0, 2.0f, -2.0f) 
	);
	m_basicMeshes->DrawPlaneMesh();
	
	// structure right
	SetTransformations(
		glm::vec3(2.0f, 2.0f, 2.0f),
		90.0f, facing, 0,
		pos + glm::vec3(0, 2.0f, 2.0f)
	);
	m_basicMeshes->DrawPlaneMesh();
}

/**********************************************************
* Fence()
* 
* this method will perform all transforms required to make a fence at a given position
* 
*/
void SceneManager::Fence(glm::vec3 pos) {
	SetShaderTexture("fence_support");
	SetTransformations(
		glm::vec3(1.0f, 2.0f, 2.0f),
		90.0f, 0.0f, 0.0f,
		pos + glm::vec3(0.0f, 2.0f, -2.0f)
	);
	m_basicMeshes->DrawPlaneMesh();

	SetShaderTexture("fence_bars");
	SetTransformations(
		glm::vec3(2.0f, 2.0f, 2.0f),
		90.0f, 90.0f, 0.0f,
		pos + glm::vec3(0.0f, 2.0f, 0.0f)
	);
	m_basicMeshes->DrawPlaneMesh();

	SetShaderTexture("fence_support");
	SetTransformations(
		glm::vec3(1.0f, 2.0f, 2.0f),
		90.0f, 0.0f, 0.0f,
		pos + glm::vec3(0.0f, 2.0f, 2.0f)
	);
	m_basicMeshes->DrawPlaneMesh();
}



//helper functions for recursive transformation math
glm::vec3 pos_from_data(glm::vec3 prev, glm::vec3 rot, float scale) {
	return glm::vec3(
		prev.x + (-sin(glm::radians(rot.z))) * scale,
		prev.y + (cos(glm::radians(rot.z))) * scale,
		prev.z);
}
glm::vec3 rot_add(glm::vec3 one, glm::vec3 two) {

	float x = one.x + two.x;
	while (x >= 360.0f){
		x -= 360.0f;
	}
	float y = one.y + two.y;
	while (y >= 360.0f) {
		y -= 360.0f;
	}
	float z = one.z + two.z;
	while (z >= 360.0f) {
		z -= 360.0f;
	}

	return glm::vec3(x, y, z);
}

void SceneManager::Branch(glm::vec3 base, glm::vec3 rot, int recursions_left) {
	if (recursions_left <= 0) {
		return;
	}
	glm::vec3 base_scale = glm::vec3(1.0f, 20.0f, -1.0f);
	glm::vec3 step_rot = rot_add(rot, glm::vec3(0, 0, 15.0f));
	float scaling = recursions_left*0.2;
	// draw two cylinders
	SetShaderTexture("wood");
	SetTransformations(
		base_scale * scaling,
		rot.x, rot.y, rot.z,
		base
	);
	m_basicMeshes->DrawCylinderMesh();
	SetTransformations(
		base_scale * scaling,
		step_rot.x, step_rot.y, step_rot.z,
		pos_from_data(base,rot,base_scale.y*scaling*0.95f)
	);
	m_basicMeshes->DrawCylinderMesh();
	// recurse (probably will need to use quaternions to make rotations easier)
	Branch(pos_from_data(base,rot,base_scale.y*scaling*0.5),rot_add(rot,glm::vec3(0.0f, 0, 30.0f)), recursions_left - 1);
	Branch(pos_from_data(base, rot, base_scale.y * scaling * 0.75), rot_add(rot, glm::vec3(0.0, 0, -30.0f)), recursions_left - 1);
}

/**********************************************************
* Tree()
* 
* This method will perform all transforms required to make a tree at a given position with a given orientation
* 
* 
*/
void SceneManager::Tree(glm::vec3 pos, float angle) {
	//
	Branch(pos, glm::vec3(0.0f,angle,0.0f), 4);
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(100.0f, 1.0f, 200.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("ground_1");
	SetShaderMaterial("plastic");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	SetShaderTexture("ground_2");
	SetTransformations(
		scaleXYZ*0.4f,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ + glm::vec3(0.0f, 0.01f, -70.0f)
	);
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	// here is where all the components go, be sure to give them the correct locations
	
	// hill
	SetTransformations(
		glm::vec3(30.0f, 30.0f, 30.0f),
		0, 180.0f, 90.0f,
		glm::vec3(0.0f, -20.0f, -200.0f)
	);
	SetShaderTexture("ground_1");
	m_basicMeshes->DrawSphereMesh();


	// trees
	Tree(glm::vec3(0.0f, 10.0f, -200.0f), 0);
	for (int i = 0; i < 5; i++) {
		Tree(glm::vec3(-25.0f, 0.0f, -40.0f - 40.0f * i),0);
		Tree(glm::vec3(-60.0f, 0.0f, -60.0f - 36.0f * i),0);
		Tree(glm::vec3(25.0f, 0.0f, -30.0f - 50.0f * i), 0);
		Tree(glm::vec3(60.0f, 0.0f, -50.0f - 44.0f * i), 0);
	}
	
	// right row of lamp posts
	for (int i = 0; i < 9; i++) {
		LampPost(glm::vec3(22.0f, 0.0f, -12.0f-20.0f*i)); // testing lamp in center for demo, pushed back a bit so that the top is visible
	}
	// left row of lamp posts
	for (int i = 0; i < 9; i++) {
		LampPost(glm::vec3(-22.0f, 0.0f, -16.0f - 20.0f * i));
	}

	// right and left side fences
	for (int i = 30; i >= 0; i--) {
		Fence(glm::vec3(20.0f, 0.0f, -4.0f * i));
		Fence(glm::vec3(-20.0f, 0.0f, -4.0f * i));
	}


	// right side benches
	for (int i = 5; i >= 0 ; i--) {
		for (int j = 4; j >= 0; j--) {
			Bench(glm::vec3(17.0f, 0.0f, -17.0f-4.0f*j-30.0f*i));
		}
	}

	// left side benches
	for (int i = 5; i >= 0; i--) {
		for (int j = 4; j >= 0; j--) {
			Bench(glm::vec3(-17.0f, 0.0f, -10.0f - 4.0f * j - 30.0f * i),true);
		}
	}

	// leaves
	for (int i = 0; i < 8; i++) {
		SetShaderTexture("ground_2");
		SetTransformations(
			glm::vec3(150.0f , 1.0f, 100.0f),
			-90.0f,
			0,
			180.0f,
			glm::vec3(i*10.0f*cos(i*95.0f), 100.0f, -200.0f + 30.0f * i)
		);
		m_basicMeshes->DrawPlaneMesh();
	}

}


