// Include glad and GLFW in correct order
#include "globalOpenGL_GLFW.h"

#include <iostream>						// C++ cin, cout, etc.
#include <glm/vec3.hpp>					// glm::vec3
#include <glm/vec4.hpp>					// glm::vec4
#include <glm/mat4x4.hpp>				// glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>			// glm::value_ptr
#include <glm/gtc/quaternion.hpp>

//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/vector_angle.hpp>

#include <stdlib.h>
#include <stdio.h>
// Add the file stuff library (file stream>
#include <fstream>
#include <sstream>						// "String stream"f
#include <istream>
#include <string>
#include <vector>						// smart array, "array" in most languages
#include <random>
#include <chrono>

#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h"

#include "Physics.h"

#include "cLightManager.h"

// Include all the things that are accessed in other files
#include "globalGameStuff.h"

#include "cMouseCamera.h"

// ADDING STUFF FOR PHYSICS LIBRARY
#include <Windows.h>

#include <eShapeType.h>
#include <iPhysicsFactory.h>
#include <iPhysicsWorld.h>
#include <iRigidBody.h>
#include <iShape.h>
#include <sRigidBodyDesc.h>
#include <iSoftBody.h>
#include <sSoftBodyDesc.h>

HINSTANCE hGetProckDll;
typedef nPhysics::iPhysicsFactory*( *f_CreateFactory )( );

f_CreateFactory CreateFactory = NULL;

nPhysics::iPhysicsWorld* g_pThePhysicsWorld;
nPhysics::iPhysicsFactory* g_pThePhysicsFactory;

nPhysics::iPhysicsWorld* g_pBulletPhysicsWorld;
nPhysics::iPhysicsFactory* g_pBulletPhysicsFactory;

std::string libraryFile = "PhysicsLibrary.dll";
std::string bulletLibraryFile = "PhysicsLibraryBullet.dll";

bool g_bUseBulletPhysics = false;
// END OF STUFF FOR PHYSICS LIBRARY


//int g_GameObjNumber = 0;				// game object vector position number 
//int g_LightObjNumber = 0;				// light object vector position

int g_selectedSphere = -1;
glm::vec3 prevPosition = glm::vec3( 0.0f );

int g_NUMBER_OF_LIGHTS = 1;

bool bIsWireframe;

// Remember to #include <vector>...
std::vector< cGameObject* > g_vecGameObjects;

cMouseCamera* g_pTheMouseCamera = NULL;
float g_lastX = 0.0f;
float g_lastY = 0.0f;
bool g_firstMouse = true;

cVAOMeshManager*	g_pVAOManager = 0;		// or NULL or nullptr

cShaderManager*		g_pShaderManager;		// Heap, new (and delete)
cLightManager*		g_pLightManager;

cBasicTextureManager*	g_pTextureManager = 0;

cDebugRenderer*			g_pDebugRenderer = 0;

cMesh g_theClothMesh;
cGameObject* g_pTheClothGO = NULL;

// This contains the AABB grid for the terrain...
//cAABBBroadPhase* g_terrainAABBBroadPhase = 0;

// Other uniforms:
GLint uniLoc_materialDiffuse = -1;
GLint uniLoc_materialAmbient = -1;
GLint uniLoc_ambientToDiffuseRatio = -1; 	// Maybe	// 0.2 or 0.3
GLint uniLoc_materialSpecular = -1;  // rgb = colour of HIGHLIGHT only
									 // w = shininess of the 
GLint uniLoc_bIsDebugWireFrameObject = -1;

GLint uniLoc_eyePosition = -1;	// Camera position
GLint uniLoc_mModel = -1;
GLint uniLoc_mView = -1;
GLint uniLoc_mProjection = -1;

struct sWindowConfig
{
public:
	int height = 480;
	int width = 640;
	std::string title = "TO BE REPLACED...";
};

struct sGOparameters		// for the Game Objects' input file
{
	std::string meshname;
	float x, y, z, scale, mass;
};

struct sMeshparameters		// for the Meshes' input file
{
	std::string meshname;
	std::string meshFilename;
};


//std::vector< glm::vec3 > g_vecPoints;
//std::vector< pointTriangles > g_vecPoints;

// Forward declare the Functions
void loadConfigFile( std::string fileName, sWindowConfig& wConfig );
sGOparameters parseObjLine( std::ifstream &source );
void loadObjectsFile( std::string fileName );
sMeshparameters parseMeshLine( std::ifstream &source );
void loadMeshesFile( std::string fileName, GLint ShaderID );
void DrawObject( cGameObject* pTheGO );
float generateRandomNumber( float min, float max );
void mouse_callback( GLFWwindow* window, double xpos, double ypos );
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );
void ProcessCameraInput( GLFWwindow *window, double deltaTime );


void DrawRenderStuff( glm::mat4 view, glm::mat4 projection )
{
	for( int index = 0; index != ::g_vecGameObjects.size(); index++ )
	{
		cGameObject* pTheGO = ::g_vecGameObjects[index];
		if( pTheGO->meshName == "ball" )	// HACK
		{
			// Draw radius for the spheres...
			glm::vec3 color = glm::vec3( 1.0f, 1.0f, 0.0f );
		
			
			nPhysics::iRigidBody* tempBody;
			if( g_bUseBulletPhysics )
				tempBody = pTheGO->btRigidBody;
			else
				tempBody = pTheGO->rigidBody;

			glm::vec3 center;
			tempBody->GetPosition( center );

			nPhysics::iShape* theShape = tempBody->GetShape();
			
			float radius;
			theShape->GetSphereRadius( radius );

			glm::vec3 radiusEnd = center + glm::vec3( radius, 0.0f, 0.0f );
			::g_pDebugRenderer->addLine( center, radiusEnd, color, false );

			radiusEnd = center + glm::vec3( 0.0f, radius, 0.0f );
			::g_pDebugRenderer->addLine( center, radiusEnd, color, false );

			radiusEnd = center + glm::vec3( 0.0f, 0.0f, radius );
			::g_pDebugRenderer->addLine( center, radiusEnd, color, false );

			// Draw velocity vector for the spheres...
			glm::vec3 velocityEnd;
			tempBody->GetVelocity( velocityEnd );

			velocityEnd += center;
			color = glm::vec3( 1.0f, 0.0f, 0.0f );
			::g_pDebugRenderer->addLine( center, velocityEnd, color, false );
		}
	}
	::g_pDebugRenderer->RenderDebugObjects( view, projection );
}


static void error_callback( int error, const char* description )
{
	fprintf( stderr, "Error: %s\n", description );
}

void updateClothMesh( GLint sexyShaderID )
{
	for( int v = 0; v != ::g_theClothMesh.numberOfVertices; v++ )
	{
		glm::vec3 position = glm::vec3( 0.0f );
		::g_pTheClothGO->softBody->GetNodePosition( v, position );
		::g_theClothMesh.pVertices[v].x = position.x;
		::g_theClothMesh.pVertices[v].y = position.y;
		::g_theClothMesh.pVertices[v].z = position.z;
		std::cout << "Node (" << v << ") pos: "
			<< position.x << ", "
			<< position.y << ", "
			<< position.z << std::endl;
	}
	::g_pVAOManager->loadMeshIntoVAO( ::g_theClothMesh, sexyShaderID, false );
}


int main( void )
{
	//------------------------------------------------------------------------------ Adding Physics Library
	//Load the Physics library
	hGetProckDll = LoadLibraryA( libraryFile.c_str() );
	if( !hGetProckDll )
	{
		std::cout << "Fail to load Physics Library File!" << std::endl;
		system( "pause" );
		return 1;
	}

	// Creating the Physics Factory from the Library
	std::string createFactoryName = "CreateFactory";

	CreateFactory = ( f_CreateFactory )GetProcAddress( hGetProckDll, createFactoryName.c_str() );
	if( !CreateFactory )
	{
		std::cout << "Where's the CreateFactory?" << std::endl;
		system( "pause" );
		return 1;
	}
	::g_pThePhysicsFactory = CreateFactory();
	::g_pThePhysicsWorld = ::g_pThePhysicsFactory->CreateWorld();	
	//------------------------------------------------------------------------------ End of Physics Library stuff

	//------------------------------------------------------------------------------ Adding Bullet Physics Library
	//Load the Physics library
	hGetProckDll = LoadLibraryA( bulletLibraryFile.c_str() );
	if( !hGetProckDll )
	{
		std::cout << "Fail to load Physics Library File!" << std::endl;
		system( "pause" );
		return 1;
	}

	// Creating the Physics Factory from the Library
	//std::string createFactoryName = "CreateFactory";

	CreateFactory = ( f_CreateFactory )GetProcAddress( hGetProckDll, createFactoryName.c_str() );
	if( !CreateFactory )
	{
		std::cout << "Where's the CreateFactory?" << std::endl;
		system( "pause" );
		return 1;
	}
	::g_pBulletPhysicsFactory = CreateFactory();
	::g_pBulletPhysicsWorld = ::g_pBulletPhysicsFactory->CreateWorld();
	//------------------------------------------------------------------------------ End of Bullet Physics Library stuff

	GLFWwindow* window;
	GLint mvp_location; //vpos_location, vcol_location;
	glfwSetErrorCallback( error_callback );

	if( !glfwInit() )
		exit( EXIT_FAILURE );

	sWindowConfig wConfig;

	loadConfigFile( "config.txt", wConfig );
	
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	#ifdef NDEBUG				// nondebug
		window = glfwCreateWindow( wConfig.width, wConfig.height,
			wConfig.title.c_str(),
			glfwGetPrimaryMonitor(),	// Runs in full screen
			NULL );
	#else						// debug code
		window = glfwCreateWindow( wConfig.width, wConfig.height,
			wConfig.title.c_str(),
			NULL, 
			NULL );
	#endif

	if( !window )
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwSetKeyCallback( window, key_callback );
	
	glfwMakeContextCurrent( window );
	gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
	glfwSwapInterval( 1 );
	glfwSetCursorPosCallback( window, mouse_callback );
	glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	glfwSetScrollCallback( window, scroll_callback );

	std::cout << glGetString( GL_VENDOR ) << " "
		<< glGetString( GL_RENDERER ) << ", "
		<< glGetString( GL_VERSION ) << std::endl;
	std::cout << "Shader language version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

	::g_pShaderManager = new cShaderManager();

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	vertShader.fileName = "simpleVert.glsl";
	fragShader.fileName = "simpleFrag.glsl";

	// General error string, used for a number of items during start up
	std::string error;

	::g_pShaderManager->setBasePath( "assets//shaders//" );

	// Shader objects are passed by reference so that
	//	we can look at the results if we wanted to. 
	if( !::g_pShaderManager->createProgramFromFile(
		"mySexyShader", vertShader, fragShader ) )
	{
		std::cout << "Oh no! All is lost!!! Blame Loki!!!" << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		// Should we exit?? 
		return -1;
		//		exit(
	}
	std::cout << "The shaders compiled and linked OK" << std::endl;

	::g_pDebugRenderer = new cDebugRenderer();
	if( !::g_pDebugRenderer->initialize( error ) )
	{
		std::cout << "Warning: couldn't init the debug renderer." << std::endl;
	}

	//Load models
	::g_pVAOManager = new cVAOMeshManager();

	GLint sexyShaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	loadMeshesFile( "meshlist.txt", sexyShaderID );
	loadObjectsFile( "objects.txt" );
	//newMapObject( sexyShaderID );

	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	// Get the uniform locations for this shader
	mvp_location = glGetUniformLocation( currentProgID, "MVP" );		// program, "MVP");
	uniLoc_materialDiffuse = glGetUniformLocation( currentProgID, "materialDiffuse" );
	uniLoc_materialAmbient = glGetUniformLocation( currentProgID, "materialAmbient" );
	uniLoc_ambientToDiffuseRatio = glGetUniformLocation( currentProgID, "ambientToDiffuseRatio" );
	uniLoc_materialSpecular = glGetUniformLocation( currentProgID, "materialSpecular" );
	uniLoc_eyePosition = glGetUniformLocation( currentProgID, "eyePosition" );

	uniLoc_mModel = glGetUniformLocation( currentProgID, "mModel" );
	uniLoc_mView = glGetUniformLocation( currentProgID, "mView" );
	uniLoc_mProjection = glGetUniformLocation( currentProgID, "mProjection" );

	::g_pLightManager = new cLightManager();

	::g_pLightManager->CreateLights( g_NUMBER_OF_LIGHTS );	// There are 10 lights in the shader
	// Change ZERO (the SUN) light position
	::g_pLightManager->vecLights[0].position = glm::vec3( 0.0f, 0.0f, 8.0f );
	::g_pLightManager->vecLights[0].attenuation.y = 0.6f;		// Change the linear attenuation
	::g_pLightManager->vecLights[0].attenuation.z = 0.005f;		

	::g_pLightManager->LoadShaderUniformLocations( currentProgID );


	// Texture 
	::g_pTextureManager = new cBasicTextureManager();
	::g_pTextureManager->SetBasePath( "assets/textures" );
	if( !::g_pTextureManager->Create2DTextureFromBMPFile( "Rough_rock_014_COLOR.bmp", true ) )
	{
		std::cout << "Didn't load the texture. Oh no!" << std::endl;
	}
	else
	{
		std::cout << "Texture is loaded! Hazzah!" << std::endl;
	}
	::g_pTextureManager->Create2DTextureFromBMPFile( "Rough_rock_015_COLOR.bmp", true );
	::g_pTextureManager->Create2DTextureFromBMPFile( "Red_Marble_001_COLOR.bmp", true );
	::g_pTextureManager->Create2DTextureFromBMPFile("square_texture.bmp", true);	
	
	cMesh terrainMesh;
	
	::g_pTheMouseCamera = new cMouseCamera( glm::vec3( 12.0f, 12.0f, 12.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ), -135.f, -32.f );
	//::g_pTheMouseCamera = new cMouseCamera( glm::vec3( 12.0f, 12.0f, 12.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ), 0.0f, -32.f );

	//std::cout << "Camera Pos: "
	//	<< ::g_pTheMouseCamera->Position.x << ", "
	//	<< ::g_pTheMouseCamera->Position.y << ", "
	//	<< ::g_pTheMouseCamera->Position.z
	//	<< " | WorldUp: "
	//	<< ::g_pTheMouseCamera->WorldUp.x << ", "
	//	<< ::g_pTheMouseCamera->WorldUp.y << ", "
	//	<< ::g_pTheMouseCamera->WorldUp.z
	//	<< " | Yaw: " << ::g_pTheMouseCamera->Yaw
	//	<< " | Pitch: " << ::g_pTheMouseCamera->Pitch
	//	<< std::endl;

	::g_lastX = wConfig.width / 2.0f;
	::g_lastY = wConfig.height / 2.0f;

	glEnable( GL_DEPTH );

	// Select the first sphere
	if( ::g_vecGameObjects.size() > 0 )
	{
		for( int i = 0; i != ::g_vecGameObjects.size(); i++ )
		{
			if( ::g_vecGameObjects[i]->meshName == "ball" )
			{
				::g_selectedSphere = i;
				::g_vecGameObjects[i]->textureBlend[0] = 0.0f;
				::g_vecGameObjects[i]->textureBlend[1] = 1.0f;

				if( g_bUseBulletPhysics )
					::g_vecGameObjects[i]->btRigidBody->GetPosition( ::prevPosition );
				else
					::g_vecGameObjects[i]->rigidBody->GetPosition( ::prevPosition );
				break;
			}
		}
	}

	// Gets the "current" time "tick" or "step"
	double lastTimeStep = glfwGetTime();

	// Main game or application loop
	while( !glfwWindowShouldClose( window ) )
	{
		updateClothMesh( sexyShaderID );

		float ratio;
		int width, height;
		glm::mat4x4 matProjection;			// was "p"

		glfwGetFramebufferSize( window, &width, &height );
		ratio = width / ( float )height;
		glViewport( 0, 0, width, height );

		// Clear colour AND depth buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		::g_pShaderManager->useShaderProgram( "mySexyShader" );
		GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

		// Update all the light uniforms...
		// (for the whole scene)
		::g_pLightManager->CopyLightInformationToCurrentShader();

		// Changing the projection to include zoom 
		// Projection and view don't change per scene (maybe)
		matProjection = glm::perspective( glm::radians( ::g_pTheMouseCamera->Zoom ),
						ratio,
						1.0f,			// Near (as big as possible)
						100000.0f );	// Far (as small as possible)

		// View or "camera" matrix
		glm::mat4 matView = glm::mat4( 1.0f );	// was "v"

		// Now the view matrix is taken right from the camera class
		matView = ::g_pTheMouseCamera->GetViewMatrix();

		glUniformMatrix4fv( uniLoc_mView, 1, GL_FALSE,
			( const GLfloat* )glm::value_ptr( matView ) );
		glUniformMatrix4fv( uniLoc_mProjection, 1, GL_FALSE,
			( const GLfloat* )glm::value_ptr( matProjection ) );

		// Set ALL texture units and binding for ENTIRE SCENE (is faster)
		{
			// 0 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,
				::g_pTextureManager->getTextureIDFromName("Rough_rock_014_COLOR.bmp"));				
			// 1
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,
				::g_pTextureManager->getTextureIDFromName("Rough_rock_015_COLOR.bmp"));
			// 2..  and so on... 
			glActiveTexture( GL_TEXTURE2 );
			glBindTexture( GL_TEXTURE_2D,
				::g_pTextureManager->getTextureIDFromName( "Red_Marble_001_COLOR.bmp" ) );
			
			// Set sampler in the shader
			// NOTE: You shouldn't be doing this during the draw call...
			GLint curShaderID = ::g_pShaderManager->getIDFromFriendlyName("mySexyShader");
			GLint textSampler00_ID = glGetUniformLocation(curShaderID, "myAmazingTexture00");
			GLint textSampler01_ID = glGetUniformLocation(curShaderID, "myAmazingTexture01");
			// And so on (up to 10, or whatever number of textures)... 

			GLint textBlend00_ID = glGetUniformLocation(curShaderID, "textureBlend00");
			GLint textBlend01_ID = glGetUniformLocation(curShaderID, "textureBlend01");

			// This connects the texture sampler to the texture units... 
			glUniform1i(textSampler00_ID, 0);
			glUniform1i(textSampler01_ID, 1);
		}


		// Enable blend ("alpha") transparency for the scene
		// NOTE: You "should" turn this OFF, then draw all NON-Transparent objects
		//       Then turn ON, sort objects from far to near ACCORDING TO THE CAMERA
		//       and draw them
		glEnable( GL_BLEND );		// Enables "blending"
									//glDisable( GL_BLEND );
									// Source == already on framebuffer
									// Dest == what you're about to draw
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );



		// Draw the scene
		unsigned int sizeOfVector = ( unsigned int )::g_vecGameObjects.size();	//*****//
		for( int index = 0; index != sizeOfVector; index++ )
		{
			cGameObject* pTheGO = ::g_vecGameObjects[index];

			DrawObject( pTheGO );

		}//for ( int index = 0...

		if( bIsWireframe )
		{
			DrawRenderStuff( matView, matProjection );
		}

		std::map< unsigned long long, cAABBv2* >::iterator itAABB;


		std::stringstream ssTitle;
		ssTitle << "Physics 2: Project 1"
			<< "Camera Position (xyz): "
			<< ::g_pTheMouseCamera->Position.x << ", "
			<< ::g_pTheMouseCamera->Position.y << ", "
			<< ::g_pTheMouseCamera->Position.z
			<< " - Yaw: "
			<< ::g_pTheMouseCamera->Yaw
			<< " - Pitch: "
			<< ::g_pTheMouseCamera->Pitch;
		
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

		glfwSwapBuffers( window );
		glfwPollEvents();

		// Essentially the "frame time"
		// Now many seconds that have elapsed since we last checked
		double curTime = glfwGetTime();
		double deltaTime = curTime - lastTimeStep;

		// Update camera from input
		ProcessCameraInput( window, deltaTime );
		
		// Physics Calculation
		if( g_bUseBulletPhysics )		
			::g_pBulletPhysicsWorld->TimeStep( ( float )deltaTime );
		else
			::g_pThePhysicsWorld->TimeStep( ( float )deltaTime );
		
		lastTimeStep = curTime;

	}// while ( ! glfwWindowShouldClose(window) )


	glfwDestroyWindow( window );
	glfwTerminate();

	// 
	delete ::g_pShaderManager;
	delete ::g_pVAOManager;

	//    exit(EXIT_SUCCESS);
	return 0;
}

//Load Config.txt
void loadConfigFile( std::string fileName, sWindowConfig& wConfig )
{
	// TODO change this config formating
	std::ifstream infoFile( fileName );
	if( !infoFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so read it... 
		std::string a;

		infoFile >> a;	// "Game"	//std::cin >> a;
		infoFile >> a;	// "Config"
		infoFile >> a;	// "width"
		infoFile >> wConfig.width;	// 1080
		infoFile >> a;	// "height"
		infoFile >> wConfig.height;	// 768
		infoFile >> a;		// Title_Start

		std::stringstream ssTitle;		// Inside "sstream"
		bool bKeepReading = true;
		do
		{
			infoFile >> a;		// Title_End??
			if( a != "Title_End" )
			{
				ssTitle << a << " ";
			}
			else
			{	// it IS the end! 
				bKeepReading = false;
				wConfig.title = ssTitle.str();
			}
		} while( bKeepReading );
	}
}

// Generate real random numbers
float generateRandomNumber( float min, float max )
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator( seed );
	std::uniform_real_distribution<float> distribution( min, max );

	float randomNumber = 0.0;

	randomNumber = distribution( generator );
	return randomNumber;

}

bool CheckDistance( glm::vec3 position, glm::vec3 fromPoint, float minDistance, float maxDistance )
{
	float thisDistance = 0.0f;
	thisDistance = glm::distance( position, fromPoint );

	if( thisDistance >= minDistance || thisDistance <= maxDistance ) return true;

	return false;
}

glm::vec3 GetRandomPosition()
{
	glm::vec3 center = glm::vec3( 0.0f );
	glm::vec3 newPosition = glm::vec3( 0.0f );
	bool validPosition = false;

	while( !validPosition )
	{
		newPosition = glm::vec3( generateRandomNumber( -200, 200 ),
			generateRandomNumber( -200, 200 ),
			generateRandomNumber( -200, 200 ) );
		validPosition = CheckDistance( newPosition, center, 30.0f, 100.0f );
	}

	return newPosition;
}

//Load objects.txt
void loadObjectsFile( std::string fileName )
{
	//sGOparameters sGOpar;
	std::vector <sGOparameters> allObjects;

	std::ifstream objectsFile( fileName );
	if( !objectsFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so loop through the file and pushback to structure
		while( !objectsFile.eof() && objectsFile.is_open() ) {
			allObjects.push_back( parseObjLine( objectsFile ) );
		}
		objectsFile.close();  //Closing "costfile.txt"
	}

	for( int index = 0; index != allObjects.size(); index++ )
	{
		// Create a new GO
		cGameObject* pTempGO = new cGameObject();

		// Set the GO Mesh
		pTempGO->meshName = allObjects[index].meshname;
		pTempGO->scale = allObjects[index].scale;

		// Create a new RigidBody
		nPhysics::iRigidBody* newBody = NULL;
		nPhysics::iRigidBody* newBulletBody = NULL;

		// Create a RigidBody Description
		nPhysics::sRigidBodyDesc theDesc;			

		theDesc.Position.x = allObjects[index].x;
		theDesc.Position.y = allObjects[index].y;
		theDesc.Position.z = allObjects[index].z;		
		theDesc.Mass	   = allObjects[index].mass;

		if( theDesc.Mass == 0.0f )
			theDesc.invMass = theDesc.Mass;
		else
			theDesc.invMass = 1 / theDesc.Mass;

		theDesc.PrevPosition = theDesc.Position;

		if( pTempGO->meshName == "ball" )
		{
			pTempGO->textureBlend[0] = 1.0f;
			pTempGO->textureNames[0] = "Rough_rock_015_COLOR.bmp";
			pTempGO->textureBlend[1] = 0.0f;
			pTempGO->textureNames[1] = "Red_Marble_001_COLOR.bmp";

			cMesh tempMesh;
			::g_pVAOManager->lookupMeshFromName( "ball", tempMesh );
			float radius = tempMesh.maxExtent / 2 * pTempGO->scale;
			
			newBody = ::g_pThePhysicsFactory->CreateRigidBody( theDesc, ::g_pThePhysicsFactory->CreateSphere( radius ) );
			newBulletBody = ::g_pBulletPhysicsFactory->CreateRigidBody( theDesc, ::g_pThePhysicsFactory->CreateSphere( radius ) );

			//::g_pThePhysicsWorld->AddRigidBody(newBody);			
			//::g_pBulletPhysicsWorld->AddRigidBody(newBulletBody);
			::g_pThePhysicsWorld->AddBody( newBody );

			pTempGO->rigidBody = newBody;
			pTempGO->btRigidBody = newBulletBody;
		}
		else if(pTempGO->meshName == "cloth")
		{
			// Save this mesh into a global variable
			::g_pVAOManager->lookupMeshFromName( "cloth", ::g_theClothMesh );

			// Create a SoftBody Description
			nPhysics::sSoftBodyDesc theDesc;

			float minZ, maxZ, minX, maxX;		
			// This will add all the vertices to the description
			// Also I'm calculating the corners' position
			for( int v = 0; v != ::g_theClothMesh.numberOfVertices; v++ )
			{
				theDesc.Vertices.push_back( glm::vec3( ::g_theClothMesh.pVertices[v].x,
													   ::g_theClothMesh.pVertices[v].y,
													   ::g_theClothMesh.pVertices[v].z ) );

				if( v == 0 )
				{
					minX = ::g_theClothMesh.pVertices[v].x;
					maxX = ::g_theClothMesh.pVertices[v].x;
					minZ = ::g_theClothMesh.pVertices[v].z;
					maxZ = ::g_theClothMesh.pVertices[v].z;
				}
				else
				{
					if( ::g_theClothMesh.pVertices[v].x < minX ) minX = ::g_theClothMesh.pVertices[v].x;
					if( ::g_theClothMesh.pVertices[v].x > maxX ) maxX = ::g_theClothMesh.pVertices[v].x;
					if( ::g_theClothMesh.pVertices[v].z < minZ ) minZ = ::g_theClothMesh.pVertices[v].z;
					if( ::g_theClothMesh.pVertices[v].z > maxZ ) maxZ = ::g_theClothMesh.pVertices[v].z;
				}
			}

			glm::vec3 topLeft, topRight, bottomLeft, bottomRight;
			topLeft = glm::vec3( minX, 0.0f, maxZ );
			topRight = glm::vec3( maxX, 0.0f, maxZ );
			bottomLeft = glm::vec3( minX, 0.0f, minZ );
			bottomRight = glm::vec3( maxX, 0.0f, minZ );
			
			// Bending Distances
			float bendEdgeDist = glm::distance( topLeft, topRight );
			float bendDiagDist = glm::distance( topLeft, bottomRight );

			// Structural (edge) and Sher (diagonal) distances
			float edgeDist = 0.0f;
			float diagDist = 0.0f;

			// Use the 1st triangle to measure the distances
			{
				nPhysics::sTriangle* theTriangle = new nPhysics::sTriangle();
				theTriangle->nodeID_0 = ::g_theClothMesh.pTriangles[0].vertex_ID_0;
				theTriangle->nodeID_1 = ::g_theClothMesh.pTriangles[0].vertex_ID_1;
				theTriangle->nodeID_2 = ::g_theClothMesh.pTriangles[0].vertex_ID_2;

				//theDesc.TriangulatedIndices.push_back( theTriangle );

				// Calculate the triangles distance
				glm::vec3 pos0, pos1, pos2;

				pos0 = glm::vec3( ::g_theClothMesh.pVertices[theTriangle->nodeID_0].x,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_0].y,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_0].z );
															 
				pos1 = glm::vec3( ::g_theClothMesh.pVertices[theTriangle->nodeID_1].x,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_1].y,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_1].z );
															 
				pos2 = glm::vec3( ::g_theClothMesh.pVertices[theTriangle->nodeID_2].x,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_2].y,
								  ::g_theClothMesh.pVertices[theTriangle->nodeID_2].z );

				// Calculating the distances
				float d01, d02, d12;
				d01 = glm::distance( pos0, pos1 );
				d02 = glm::distance( pos0, pos2 );
				d12 = glm::distance( pos1, pos2 );

				// Comparing the distances
				if( d01 == d02 )		// d12 must be larger
				{
					diagDist = d12;
					edgeDist = d01;					
				}
				else if( d01 == d12 )	// d02 must be larger
				{
					diagDist = d02;
					edgeDist = d01;
				}
				else					// d01 must be the largest
				{
					diagDist = d01;
					edgeDist = d02;
				}
			}

			// Now that I got all the distances I can create the Springs description
			for( int i = 0; i != ::g_theClothMesh.numberOfVertices -1; i++ )
			{
				for( int j = i+1; j != ::g_theClothMesh.numberOfVertices; j++ )
				{
					nPhysics::sConstrain* theConstrain = new nPhysics::sConstrain();
					theConstrain->nodeID_0 = i;
					theConstrain->nodeID_1 = j;

					glm::vec3 posVertI = ( glm::vec3( ::g_theClothMesh.pVertices[i].x,
													  ::g_theClothMesh.pVertices[i].y,
													  ::g_theClothMesh.pVertices[i].z ) );

					glm::vec3 posVertJ = ( glm::vec3( ::g_theClothMesh.pVertices[j].x,
													  ::g_theClothMesh.pVertices[j].y,
													  ::g_theClothMesh.pVertices[j].z ) );

					float distance = glm::distance( posVertI, posVertJ );

					if( distance == edgeDist )			// It's a Structural constrain
					{
						theConstrain->type = nPhysics::eConstrainType::STRUCTURAL;
						theDesc.ConstrainIndices.push_back( theConstrain );
					}

					else if( distance == diagDist )		// It's a Sher constrain
					{
						theConstrain->type = nPhysics::eConstrainType::SHEAR;
						theDesc.ConstrainIndices.push_back( theConstrain );
					}

					else if( distance == bendEdgeDist )	// It's a Bending constrain
					{
						theConstrain->type = nPhysics::eConstrainType::BEND;
						theDesc.ConstrainIndices.push_back( theConstrain );
					}

					else if( distance == bendDiagDist )	// It's a Bending constrain
					{
						theConstrain->type = nPhysics::eConstrainType::BEND;
						theDesc.ConstrainIndices.push_back( theConstrain );
					}						

					// Find the Top Left and Top Right Nodes and add them to static list
					if( posVertI == topLeft || posVertI == topRight ) theDesc.StaticIndices.push_back( i );
				}
			}

			// Set the radius of the nodes based on the Structural Constrain distance
			theDesc.radius = edgeDist / 2;
						
			pTempGO->textureBlend[0] = 1.0f;
			pTempGO->textureNames[0] = "square_texture.bmp";
			pTempGO->isSoftBody = true;
			pTempGO->position = glm::vec3(	allObjects[index].x,
											allObjects[index].y,
											allObjects[index].z );
			
			nPhysics::iSoftBody* newBody = ::g_pThePhysicsFactory->CreateSoftBody( theDesc );
			::g_pThePhysicsWorld->AddBody( newBody );

			::g_pTheClothGO = pTempGO;
			pTempGO->softBody = newBody;
		}
		else
		{
			pTempGO->textureBlend[0] = 1.0f;
			pTempGO->textureNames[0] = "Rough_rock_014_COLOR.bmp";

			cMesh tempMesh;
			::g_pVAOManager->lookupMeshFromName( pTempGO->meshName, tempMesh );

			glm::vec3 planeNormal = glm::vec3( tempMesh.pVertices[tempMesh.pTriangles[0].vertex_ID_0].nx,
												tempMesh.pVertices[tempMesh.pTriangles[0].vertex_ID_0].ny,
												tempMesh.pVertices[tempMesh.pTriangles[0].vertex_ID_0].nz );

			planeNormal = glm::normalize( planeNormal );
			planeNormal *= glm::vec3( -1.0, -1.0, -1.0 );

			float planeConst = glm::dot( theDesc.Position, planeNormal );

			newBody = ::g_pThePhysicsFactory->CreateRigidBody( theDesc, g_pThePhysicsFactory->CreatePlane( planeNormal, planeConst ) );
			newBulletBody = ::g_pBulletPhysicsFactory->CreateRigidBody( theDesc, g_pThePhysicsFactory->CreatePlane( planeNormal, planeConst ) );

			//::g_pThePhysicsWorld->AddRigidBody(newBody);
			//::g_pBulletPhysicsWorld->AddRigidBody(newBulletBody);
			::g_pThePhysicsWorld->AddBody( newBody );


			pTempGO->rigidBody = newBody;
			pTempGO->btRigidBody = newBulletBody;
		}



		::g_vecGameObjects.push_back( pTempGO );
	}
}

// Parse the file line to fit into the structure
sGOparameters parseObjLine( std::ifstream &source ) {

	sGOparameters sGOpar;

	//Scanning a line from the file
	source >> sGOpar.meshname 
		>> sGOpar.x 
		>> sGOpar.y 
		>> sGOpar.z 
		>> sGOpar.scale
		>> sGOpar.mass;

	return sGOpar;
}

//Load meshlist.txt
void loadMeshesFile( std::string fileName, GLint ShaderID )
{
	std::vector <sMeshparameters> allMeshes;

	std::ifstream objectsFile( fileName );
	if( !objectsFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so loop through the file and pushback to structure
		while( !objectsFile.eof() && objectsFile.is_open() ) {
			allMeshes.push_back( parseMeshLine( objectsFile ) );
		}
		objectsFile.close();  //Closing "costfile.txt"
	}

	for( int index = 0; index != allMeshes.size(); index++ )
	{
		cMesh testMesh;
		testMesh.name = allMeshes[index].meshname;
		//if( !LoadPlyFileIntoMesh( allMeshes[index].meshFilename, testMesh ) )
		if( !LoadPlyFileIntoMeshWithUV( allMeshes[index].meshFilename, testMesh ) )
		{
			std::cout << "Didn't load model" << std::endl;
			// do something??
		}
		if( testMesh.name == "ball" )
		{
			if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, ShaderID, true ) )
			{
				std::cout << "Could not load mesh into VAO" << std::endl;
			}
		}
		else
		{
			if( !::g_pVAOManager->loadMeshIntoVAO( testMesh, ShaderID, true ) )
			{
				std::cout << "Could not load mesh into VAO" << std::endl;
			}
		}		
	}
}

// Parse the file line to fit into the structure
sMeshparameters parseMeshLine( std::ifstream &source ) {

	sMeshparameters sMeshpar;

	//Scanning a line from the file
	source >> sMeshpar.meshname >> sMeshpar.meshFilename;

	return sMeshpar;
}

// Draw a single object
void DrawObject( cGameObject* pTheGO )
{
	// Is there a game object? 
	if( pTheGO == 0 )	//if ( ::g_GameObjects[index] == 0 )
	{	// Nothing to draw
		return;		// Skip all for loop code and go to next
	}

	// Was near the draw call, but we need the mesh name
	std::string meshToDraw = pTheGO->meshName;		//::g_GameObjects[index]->meshName;

	sVAOInfo VAODrawInfo;
	if( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
	{	// Didn't find mesh
		std::cout << "WARNING: Didn't find mesh " << meshToDraw << " in VAO Manager" << std::endl;
		return;
	}

	// There IS something to draw
	glm::vec3 position;
	glm::quat qOrientation;
	
	if ( pTheGO->isSoftBody )
	{
		position = pTheGO->position;
		qOrientation = pTheGO->qOrientation;
	}
	else
	{
		if (g_bUseBulletPhysics)
		{
			if (pTheGO->btRigidBody != NULL)
			{
				pTheGO->btRigidBody->GetPosition(position);
				pTheGO->btRigidBody->GetRotation(qOrientation);
			}

		}
		else
		{
			if (pTheGO->rigidBody != NULL)
			{
				pTheGO->rigidBody->GetPosition(position);
				pTheGO->rigidBody->GetRotation(qOrientation);
			}
		}
	}

	// 'model' or 'world' matrix
	glm::mat4x4 mModel = glm::mat4x4( 1.0f );	//		mat4x4_identity(m);

	glm::mat4 trans = glm::mat4x4( 1.0f );
	trans = glm::translate( trans, position );
	mModel = mModel * trans;

	// Now with quaternion rotation
	// Like many things in GML, the conversion is done in the constructor
	glm::mat4 postRotQuat = glm::mat4( qOrientation ); //pTheGO->qOrientation );
	mModel = mModel * postRotQuat;

	float finalScale = pTheGO->scale;

	glm::mat4 matScale = glm::mat4x4( 1.0f );
	matScale = glm::scale( matScale,
		glm::vec3( finalScale,
			finalScale,
			finalScale ) );
	mModel = mModel * matScale;

	glUniformMatrix4fv( uniLoc_mModel, 1, GL_FALSE,
		( const GLfloat* )glm::value_ptr( mModel ) );
	
	glm::mat4 mWorldInTranpose = glm::inverse( glm::transpose( mModel ) );
	
	glUniform4f( uniLoc_materialDiffuse,
		pTheGO->diffuseColour.r,
		pTheGO->diffuseColour.g,
		pTheGO->diffuseColour.b,
		pTheGO->diffuseColour.a );
	//...and all the other object material colours

	if( pTheGO->bIsWireFrame )
	{
		glUniform1f( uniLoc_bIsDebugWireFrameObject, 1.0f );	// TRUE
	}
	else
	{
		glUniform1f( uniLoc_bIsDebugWireFrameObject, 0.0f );	// FALSE
	}

	// Set up the textures
	std::string textureName = pTheGO->textureNames[0];
	GLuint texture00Number
		= ::g_pTextureManager->getTextureIDFromName( textureName );
	// Texture binding... (i.e. set the 'active' texture
	GLuint texture00Unit = 0;							// Texture units go from 0 to 79 (at least)
	glActiveTexture( texture00Unit + GL_TEXTURE0 );		// GL_TEXTURE0 = 33984
	glBindTexture( GL_TEXTURE_2D, texture00Number );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D,
		::g_pTextureManager->getTextureIDFromName( pTheGO->textureNames[0] ) );
	
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D,
		::g_pTextureManager->getTextureIDFromName( pTheGO->textureNames[1] ) );

	// Set sampler in the shader
	GLint curShaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );
	GLint textSampler00_ID = glGetUniformLocation( curShaderID, "myAmazingTexture00" );
	GLint textSampler01_ID = glGetUniformLocation( curShaderID, "myAmazingTexture01" );
	
	GLint textBlend00_ID = glGetUniformLocation( curShaderID, "textureBlend00" );
	GLint textBlend01_ID = glGetUniformLocation( curShaderID, "textureBlend01" );

	// And the blending values
	glUniform1f( textBlend00_ID, pTheGO->textureBlend[0] );
	glUniform1f( textBlend01_ID, pTheGO->textureBlend[1] );
	
	if( bIsWireframe )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );	// Default
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	// Default
		glEnable( GL_DEPTH_TEST );		// Test for z and store in z buffer
		glEnable( GL_CULL_FACE );
	}

	glCullFace( GL_BACK );

	glBindVertexArray( VAODrawInfo.VAO_ID );

	glDrawElements( GL_TRIANGLES,
		VAODrawInfo.numberOfIndices,		// testMesh.numberOfTriangles * 3,	// How many vertex indices
		GL_UNSIGNED_INT,					// 32 bit int 
		0 );
	// Unbind that VAO
	glBindVertexArray( 0 );

	return;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback( GLFWwindow* window, double xpos, double ypos )
{
	if( ::g_firstMouse )
	{
		::g_lastX = ( float )xpos;
		::g_lastY = ( float )ypos;
		::g_firstMouse = false;
	}

	float xoffset = ::g_lastX - ( float )xpos;
	float yoffset = ::g_lastY - ( float )ypos; // reversed since y-coordinates go from bottom to top

	::g_lastX = ( float )xpos;
	::g_lastY = ( float )ypos;

	::g_pTheMouseCamera->ProcessMouseMovement( xoffset, yoffset );
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
	::g_pTheMouseCamera->ProcessMouseScroll( ( float )yoffset );
}

//void printCameraDetails()
//{
//	std::cout << "Camera Pos: "
//		<< ::g_pTheMouseCamera->Position.x << ", "
//		<< ::g_pTheMouseCamera->Position.y << ", "
//		<< ::g_pTheMouseCamera->Position.z
//		<< " | WorldUp: "
//		<< ::g_pTheMouseCamera->WorldUp.x << ", "
//		<< ::g_pTheMouseCamera->WorldUp.y << ", "
//		<< ::g_pTheMouseCamera->WorldUp.z
//		<< " | Yaw: " << ::g_pTheMouseCamera->Yaw
//		<< " | Pitch: " << ::g_pTheMouseCamera->Pitch
//		<< std::endl;
//}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessCameraInput( GLFWwindow *window, double deltaTime )
{
	if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
		glfwSetWindowShouldClose( window, true );

	if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( FORWARD, ( float )deltaTime );
	if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( BACKWARD, ( float )deltaTime );
	if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( LEFT, ( float )deltaTime );
	if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( RIGHT, ( float )deltaTime );
	if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( UP, ( float )deltaTime );
	if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
		::g_pTheMouseCamera->ProcessKeyboard( DOWN, ( float )deltaTime );
}

void switchPhysicsEngine()
{	
	nPhysics::iRigidBody* targetBody;
	nPhysics::iRigidBody* sourceBody;

	for( int index = 0; index != ::g_vecGameObjects.size(); index++ )
	{
		cGameObject* pTheGO = ::g_vecGameObjects[index];
		if( pTheGO->rigidBody == nullptr )
			continue;

		if( g_bUseBulletPhysics )
		{
			targetBody = pTheGO->rigidBody;
			sourceBody = pTheGO->btRigidBody;
		}
		else
		{
			targetBody = pTheGO->btRigidBody;
			sourceBody = pTheGO->rigidBody;			
		}

		glm::vec3 pos;
		glm::vec3 vel;

		// Grab data from the 1st engine
		sourceBody->GetVelocity( vel );
		sourceBody->GetPosition( pos );
		
		// Set it to the engine to be used
		targetBody->SetVelocity( vel );
		targetBody->SetPosition( pos );

		glm::vec3 finalpos;
		glm::vec3 finalvel;

		targetBody->GetVelocity( finalvel );
		targetBody->GetPosition( finalpos );
	}
	
	return;
}