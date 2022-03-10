//Evan Sinasac - 1081418
//INFO6046 Media Fundamentals (Project 3)
//main.cpp description:
//			        The main purpose of project 3 is to place some sounds in a 3D scene and place objects to represent them
//                  so we can explore the scene and see how the sounds change depending on listener position.  I will have the
//                  Camera and listener keep in place depending on which of the listeners are selected (i.e. I'll keep track of
//                  which listener is selected and it's position and when we switch, move the camera and place an object where
//                  we left the other listener)

// Used the "most recent" shaders (same ones I used for the Graphics midterm), but disabled the lights and didn't include the
// lights classes

#include "GLCommon.h"	//common includes for GLAD and GLFW
#include "GLMCommon.h"	//common includes for GLM (mainly vector and matrix stuff

#include "globalThings.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>		//smart array
#include <fstream>		//C++ file I-O library

//Media Fundys
#include <FMOD/fmod.hpp>


//Globals;
float lastX = 600.f;
float lastY = 320.f;
bool firstMouse = true;
float yaw = 90.f;
float pitch = 0.f;

float deltaTime = 0.f;
float lastFrame = 0.f;

bool bWireFrame = true;

std::vector<std::string> modelLocations;


//Function signature for DrawObject()
void DrawObject(
    cMesh* pCurrentMesh,
    glm::mat4 matModel,
    GLint matModel_Location,
    GLint matModelInverseTranspose_Location,
    GLuint program,
    cVAOManager* pVAOManager);


FMOD::System* _system = NULL;
FMOD_RESULT _result = FMOD_OK;

//FMOD::DSP* _dspPan;

std::vector<std::string> songNames;
std::vector<FMOD::Sound*> _vecSounds;
std::vector<FMOD::Channel*> _vecChannels;

// Speaker positions
FMOD_VECTOR _listenerPosition1 = { ::cameraEye.x, ::cameraEye.y, ::cameraEye.z };
FMOD_VECTOR _listenerPosition2 = { ::bunnyListenerPos.x, bunnyListenerPos.y, bunnyListenerPos.z };

// Tell FMOD where is fwd, up and velocity (Forward gets updated with cameraTarget for camera listener)
FMOD_VECTOR _forward1 = { 0.0f, 0.0f, 1.0f };
FMOD_VECTOR _up1 = { 0.0f, 1.0f, 0.0f };
FMOD_VECTOR _velocity1{ 0.0f, 0.0f, 0.0f };
// These won't change, but the position will
FMOD_VECTOR _forward2 = { 0.0f, 0.0f, 1.0f };
FMOD_VECTOR _up2 = { 0.0f, 1.0f, 0.0f };
FMOD_VECTOR _velocity2{ 0.0f, 0.0f, 0.0f };


//callbacks
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);


GLFWwindow* _pWindow;
//Function signatures
bool init();
bool initGLFW();
bool initGL();
bool initFMOD();
void shutDown();

bool loadWorldFile();
bool loadSounds();

bool playSounds();


int main(int argc, char** argv)
{
    //if (!glfwInit())
    //{
    //    return -1;
    //}

    if (!init())
    {
        fprintf(stderr, "Unable to initalize app");
        exit(EXIT_FAILURE);
    }

    GLuint program = 0;     //0 means "no shader program"

    GLint mvp_location = -1;
    std::stringstream ss;

    glm::vec3 camRight(1.0f);
    glm::vec3 camForward(1.0f);
    glm::vec3 camUp(0.0f, 1.0f, 0.0f);

    glfwSetErrorCallback(GLFW_error_callback);

    

    //Moved into init()->initGLFW()
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    pWindow = glfwCreateWindow(1200, 640, "Media Project 3", NULL, NULL);
    if (!pWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(pWindow, GLFW_key_callback);

    glfwSetCursorEnterCallback(pWindow, GLFW_cursor_enter_callback);
    glfwSetCursorPosCallback(pWindow, GLFW_cursor_position_callback);
    glfwSetScrollCallback(pWindow, GLFW_scroll_callback);
    glfwSetMouseButtonCallback(pWindow, GLFW_mouse_button_callback);
    glfwSetWindowSizeCallback(pWindow, GLFW_window_size_callback);


    glfwSetCursorPosCallback(pWindow, mouse_callback);

    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(pWindow);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);*/


    GLint max_uniform_location = 0;
    GLint* p_max_uniform_location = NULL;
    p_max_uniform_location = &max_uniform_location;
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, p_max_uniform_location);

    std::cout << "GL_MAX_UNIFORM_LOCATIONS: " << max_uniform_location << std::endl;

    // Create global things
    ::g_StartUp(_pWindow);



    cShaderManager::cShader vertShader;
    ss << SOLUTION_DIR << "common/assets/shaders/vertShader_01.glsl";
    vertShader.fileName = ss.str().c_str();
    ss.str("");

    cShaderManager::cShader fragShader;
    ss << SOLUTION_DIR << "common/assets/shaders/fragShader_01.glsl";
    fragShader.fileName = ss.str().c_str();
    ss.str("");


    if (::g_pShaderManager->createProgramFromFile("Shader#1", vertShader, fragShader))
    {
        std::cout << "Shader compiled OK" << std::endl;
        //Set the "program" variable to the one the Shader Manager used...
        program = ::g_pShaderManager->getIDFromFriendlyName("Shader#1");
    }
    else
    {
        std::cout << "Error making shader program: " << std::endl;
        std::cout << ::g_pShaderManager->getLastError() << std::endl;
    }

    // Select the shader program we want to use
    // (Note we only have one shader program at this point)
    glUseProgram(program);


    //GLint mvp_location = -1;
    mvp_location = glGetUniformLocation(program, "MVP");

    // Get "uniform locations" (aka the registers these are in)
    GLint matModel_Location = glGetUniformLocation(program, "matModel");
    GLint matView_Location = glGetUniformLocation(program, "matView");
    GLint matProjection_Location = glGetUniformLocation(program, "matProjection");
    GLint matModelInverseTranspose_Location = glGetUniformLocation(program, "matModelInverseTranspose");

    //Will remove colour from models, will be assigned by World file
    GLint bUseVertexColour_Location = glGetUniformLocation(program, "bUseVertexColour");
    GLint vertexColourOverride_Location = glGetUniformLocation(program, "vertexColourOverride");

    //Lights stuff would go here but we're not using it for this program
    

    //Old load files
    /*if (loadModelsFromFile(program))
    {
        std::cout << "loadModelsFromFile finished ok" << std::endl;
    }
    else
    {
        std::cout << "loadModelsFromFile did not finish ok, aborting" << std::endl;
        return -1;
    }*/

    ss.str("");
    ss << SOLUTION_DIR << "common\\assets\\models\\";

    ::g_pVAOManager->setFilePath(ss.str());

    modelLocations.push_back("bun_zipper_res2_XYZ_N.ply");
    //modelLocations.push_back("Biodome_02.ply");
    //modelLocations.push_back("dfk_arch_0.ply");
    modelLocations.push_back("dfk_bookshelf.ply");
    modelLocations.push_back("dfk_chairs.ply");
    //modelLocations.push_back("dfk_crystal_02.ply");
    modelLocations.push_back("dfk_pillar_small_02.ply");
    modelLocations.push_back("Floor_01_B.ply");
    modelLocations.push_back("Turret.ply");

    unsigned int totalVerticesLoaded = 0;
    unsigned int totalTrianglesLoaded = 0;
    for (std::vector<std::string>::iterator itModel = modelLocations.begin(); itModel != modelLocations.end(); itModel++)
    {
        sModelDrawInfo theModel;
        std::string modelName = *itModel;
        std::cout << "Loading " << modelName << "...";
        if (!::g_pVAOManager->LoadModelIntoVAO(modelName, theModel, program))
        {
            std::cout << "didn't work because: " << std::endl;
            std::cout << ::g_pVAOManager->getLastError(true) << std::endl;
        }
        else
        {
            std::cout << "OK." << std::endl;
            std::cout << "\t" << theModel.numberOfVertices << " vertices and " << theModel.numberOfTriangles << " triangles loaded." << std::endl;
            totalTrianglesLoaded += theModel.numberOfTriangles;
            totalVerticesLoaded += theModel.numberOfVertices;
        }
    } //end of for (std::vector<std::string>::iterator itModel

    std::cout << "Done loading models." << std::endl;
    std::cout << "Total vertices loaded = " << totalVerticesLoaded << std::endl;
    std::cout << "Total triangles loaded = " << totalTrianglesLoaded << std::endl;

    //World file stuff here
    if (loadWorldFile())
    {
        std::cout << "loadWorldFile finished OK" << std::endl;
    }
    else
    {
        std::cout << "loadWorldFile did not finish OK, aborting" << std::endl;
        return -1;
    }

    ::bunnyListenerPos = ::g_vec_pMeshes[g_vec_pMeshes.size() - 1]->positionXYZ;

    // playSounds needs to run after loadWorldFile so that the sounds are placed at the same positions as the meshes 
    //(rather than hard coding positions, I can just change the mesh position in the worldFile.txt and the sound will move)
    if (!playSounds())
    {
        fprintf(stderr, "Play sounds failed!");
        return -1;
    }

    //cameraTarget is (0,0,1) by default.    
    if (::cameraEye.x > 0 && ::cameraEye.z > 0)
    {
        yaw = 180.f + (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
    }
    else if (::cameraEye.x > 0 && ::cameraEye.z < 0)
    {
        yaw = 90.f - (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
    }
    else if (::cameraEye.x < 0 && ::cameraEye.z > 0)
    {
        yaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
    }
    else if (::cameraEye.x < 0 && ::cameraEye.z < 0)
    {
        yaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
    }
    else if (::cameraEye.x == 0.f)
    {
        if (::cameraEye.z >= 0.f)
        {
            yaw = 270.f;
        }
        else
        {
            yaw = 90.f;
        }
    }
    else if (::cameraEye.z == 0.f)
    {
        if (::cameraEye.x <= 0)
        {
            yaw = 0.f;
        }
        else
        {
            yaw = 180.f;
        }
    }
    
    ::cameraTarget = glm::vec3(-1.f * ::cameraEye.x, 0, -1.f * ::cameraEye.z);
    glm::normalize(::cameraTarget);                                           //and normalize it so when we initially add the target to the position it doesn't shift

    const double MAX_DELTA_TIME = 0.1;	//100 ms
    double previousTime = glfwGetTime();

    // From FMOD API:  If the number of listeners is set to more than 1, then panning and doppler are turned off. 
    // All sound effects will be mono. FMOD uses a 'closest sound to the listener' method to determine what should be 
    // heard in this case.
    //_system->set3DNumListeners(2);          
    
    while (!glfwWindowShouldClose(_pWindow))
    {
        float ratio;
        int width, height;
        glm::mat4 matModel;				// used to be "m"; Sometimes it's called "world"
        glm::mat4 matProjection;        // used to be "p";
        glm::mat4 matView;              // used to be "v";

        double currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
        previousTime = currentTime;

        glfwGetFramebufferSize(_pWindow, &width, &height);
        ratio = width / (float)height;

        //Turn on the depth buffer
        glEnable(GL_DEPTH);         //Turns on the depth buffer
        glEnable(GL_DEPTH_TEST);    //Check if the pixel is already closer

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //*********************************************************************
        //Screen is cleared and we're ready to draw
        //*********************************************************************

        //Update the title text
        glfwSetWindowTitle(_pWindow, ::g_TitleText.c_str());


        matProjection = glm::perspective(0.6f,
            ratio,
            0.1f,
            1000.0f);

        matView = glm::mat4(1.0f);

        matView = glm::lookAt(::cameraEye,
            ::cameraEye + ::cameraTarget,
            upVector);

        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(matView));
        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(matProjection));

        ::g_vec_pMeshes[::g_vec_pMeshes.size() - 1]->positionXYZ = ::bunnyListenerPos;

        // **********************************************************************
        // Draw the "scene" of all objects.
        // i.e. go through the vector and draw each one...
        // **********************************************************************
        for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
        {
            cMesh* pCurrentMesh = ::g_vec_pMeshes[index];

            //         mat4x4_identity(m);
            matModel = glm::mat4(1.0f);     //"Identity" ("do nothing", like x1)
           
            DrawObject(pCurrentMesh,
                matModel,
                matModel_Location,
                matModelInverseTranspose_Location,
                program,
                ::g_pVAOManager);

        } //end of for
        // Scene is drawn
        // **********************************************************************

        
        // From FMOD Documentation:
        //              listener:   Listener ID in a multi-listener environment. Specify 0 if there is only 1 listener.
        //              pos:        The position of the listener in world space, measured in distance units. You can specify 0 or NULL to not update the position.
        //              vel:        The velocity of the listener measured in distance units per second. You can specify 0 or NULL to not update the velocity of the listener.
        //              forward:    The forwards orientation of the listener. This vector must be of unit length and 
        //                          perpendicular to the up vector. You can specify 0 or NULL to not update the forwards 
        //                          orientation of the listener.
        //              up:         The upwards orientation of the listener. This vector must be of unit length and 
        //                          perpendicular to the forwards vector. You can specify 0 or NULL to not update the upwards 
        //                          orientation of the listener.
        // Amazing!  When I read documentation and follow the instructions, I stop getting errors! lmao
        
        if (!::bunOrCamListener)    // listening to camera
        {
            _listenerPosition1 = { ::cameraEye.x, ::cameraEye.y, ::cameraEye.z };           
            camForward = glm::vec3(::cameraTarget.x, 0.0f, ::cameraTarget.z);
            camForward = glm::normalize(camForward);
            _forward1 = { camForward.x, camForward.y, camForward.z };
            camRight = glm::cross(camForward, ::upVector);
            camUp = glm::cross(camForward, camRight);
            camUp = glm::normalize(camUp);
            _up1 = { camUp.x, camUp.y, camUp.z };
            _velocity1 = { 0.0f, 0.0f, 0.0f };       //Not sure what to do with velocity while moving so just leaving it at 0 for simplicity
            
            _result = _system->set3DListenerAttributes(0, &_listenerPosition1, &_velocity1, &_forward1, &_up1);
            if (FMOD_OK != _result) {
                fprintf(stderr, "Unable to set 3d listener attributes.\n");
            }
        }
        else    // listening to bunny
        {
            _listenerPosition2 = { ::bunnyListenerPos.x, bunnyListenerPos.y, bunnyListenerPos.z };
            _result = _system->set3DListenerAttributes(1, &_listenerPosition2, &_velocity2, &_forward2, &_up2);
            //_result = _system->set3DListenerAttributes(0, &_listenerPosition2, &_velocity2, &_forward2, &_up2);
            if (FMOD_OK != _result) {
                fprintf(stderr, "Unable to set 3d listener attributes.\n");
            }
        }
        
        _result = _system->update();
        if (FMOD_OK != _result) {
            fprintf(stderr, "Unable to update FMOD system.\n");
        }

        // "Present" what we've drawn.
        glfwSwapBuffers(_pWindow);        // Show what we've drawn

        // Process any events that have happened
        glfwPollEvents();

        // Handle OUR keyboard, mouse stuff
        handleAsyncKeyboard(_pWindow, deltaTime);
        handleAsyncMouse(_pWindow, deltaTime);

    }//while (!glfwWindowShouldClose(window))

	// All done, so delete things...
    ::g_ShutDown(_pWindow);
    shutDown();

    glfwDestroyWindow(_pWindow);

    glfwTerminate();
    exit(EXIT_SUCCESS);
} //end of main

//Function Definitions
bool init()
{
    if (!initGLFW())
    {
        return false;
    }
    if (!initGL())
    {
        return false;
    }
    if (!initFMOD())
    {
        return false;
    }

    return true;
} //end of init

bool initGLFW() {

    if (!glfwInit()) {
        fprintf(stderr, "Unable to initalize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    _pWindow = glfwCreateWindow(1200, 640, "Media Project 3", NULL, NULL);
    if (!_pWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(_pWindow, GLFW_key_callback);

    glfwSetCursorEnterCallback(_pWindow, GLFW_cursor_enter_callback);
    glfwSetCursorPosCallback(_pWindow, GLFW_cursor_position_callback);
    glfwSetScrollCallback(_pWindow, GLFW_scroll_callback);
    glfwSetMouseButtonCallback(_pWindow, GLFW_mouse_button_callback);
    glfwSetWindowSizeCallback(_pWindow, GLFW_window_size_callback);


    glfwSetCursorPosCallback(_pWindow, mouse_callback);

    glfwSetInputMode(_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(_pWindow);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


    return true;
} //end of initGLFW

bool initGL() {

    if (!gladLoadGL(/*glfwGetProcAddress*/)) {  //Don't know why we don't need this now
        fprintf(stderr, "Unable to initialize glad");
        return false;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
} //end of initGL

bool initFMOD() {
    _result = FMOD::System_Create(&_system);
    if (_result != FMOD_OK) {
        fprintf(stderr, "Unable to create FMOD system");
        return false;
    }

    _result = _system->init(32, FMOD_INIT_NORMAL, NULL);
    if (_result != FMOD_OK) {
        fprintf(stderr, "Unable to initialize FMOD system");
        return false;
    }


    //std::vector<std::string> audioList = getSoundsFromFile("audioList.txt");
    // equivalent should be my songNames
    if (!loadSounds())
    {
        fprintf(stderr, "Unable to load sounds!");
        return false;
    }

    //Added to loadSounds

    //std::stringstream ss;
    //std::vector<std::string>::iterator itAudioList = songNames.begin();
    //for (itAudioList; itAudioList != songNames.end(); itAudioList++) {
    //    ss.str("");
    //    //TODO: Path - You should do better than this
    //    ss << SOLUTION_DIR << "common\\assets\\audio\\" << *itAudioList;
    //    FMOD::Sound* sound = nullptr;
    //    FMOD::Channel* channel = nullptr;
    //    _result = _system->createSound(ss.str().c_str(), FMOD_3D, 0, &sound);
    //    //TODO: Error Check - You should do better than this anywhere you check for errors
    //    if (FMOD_OK != _result) {
    //        fprintf(stderr, "Unable to create sound for audio: %s\n", ss.str().c_str());
    //        continue;
    //    }
    //    //Set min/max distance
    //    _result = sound->set3DMinMaxDistance(0.5f, 5000.0f);
    //    if (FMOD_OK != _result) {
    //        fprintf(stderr, "Unable to set min/max distance for sound: %s", ss.str().c_str());
    //        //return false;
    //    }
    //    //set loop to normal
    //    _result = sound->setMode(FMOD_LOOP_NORMAL);
    //    if (FMOD_OK != _result) {
    //        fprintf(stderr, "Unable to set loop normal for sound: %s", ss.str().c_str());
    //    }
    //    _vecSounds.push_back(sound);
    //    _vecChannels.push_back(channel);
    //}

    if (_vecSounds.size() <= 0) {
        fprintf(stderr, "Unable to create sounds");
        return false;
    }

    return true;
} //end of initFMOD


void shutDown() {

    glfwTerminate();


    //Release sounds
    std::vector<FMOD::Sound*>::iterator itSounds = _vecSounds.begin();
    for (itSounds; itSounds != _vecSounds.end(); itSounds++) {
        _result = (*itSounds)->release();
        if (_result != FMOD_OK) {
            fprintf(stderr, "Unable to release sound");
        }
    }

    _result = _system->close();
    if (_result != FMOD_OK) {
        fprintf(stderr, "Unable to close system");
    }

    _result = _system->release();
    if (_result != FMOD_OK) {
        fprintf(stderr, "Unable to release system");
    }

    /*if (_text) {
        delete _text;
        _text = nullptr;
    }*/


    exit(EXIT_SUCCESS);
} //end of shutdown

bool loadWorldFile()
{
    std::stringstream ss;
    std::stringstream sFile;

    ss << SOLUTION_DIR << "common\\assets\\worldFile.txt";

    std::ifstream theFile(ss.str());

    if (!theFile.is_open())
    {
        fprintf(stderr, "Could not open modelsToLoad.txt");
        return false;
    }
    std::string nextToken;
    ss.str("");

    //Throwaway text describing the format of the file
    theFile >> nextToken;       //ModelFileName(extension)
    theFile >> nextToken;       //Position(x,y,z)
    theFile >> nextToken;       //Orientation(x,y,z)
    theFile >> nextToken;       //Scale
    theFile >> nextToken;       //Colour(r,g,b,a)

    theFile >> nextToken;       //Camera
    if (nextToken == "Camera")
    {
        //might as well check that we're in the right spot
        theFile >> nextToken;   //Camera x position
        ::cameraEye.x = std::stof(nextToken);
        theFile >> nextToken;   //Camera y position
        ::cameraEye.y = std::stof(nextToken);
        theFile >> nextToken;   //Camera z position
        ::cameraEye.z = std::stof(nextToken);
    }
    else
    {
        std::cout << "Uhm, we're in the wrong position in the worldFile, aborting!" << std::endl;
        return false;
    }

    while (theFile >> nextToken)    //this should always be the name of the model to load or end.  Potential error check, add a check for "ply" in the mdoel name
    {
        cMesh* curMesh = new cMesh();
        if (nextToken == "end")
        {
            break;
        }
        std::cout << nextToken << std::endl;        //Printing model names to console, just making sure we're loading ok.  Can be commented out whenever
        //First is the file name of model
        //ss << SOLUTION_DIR << "common\\assets\\models\\" << nextToken;
        curMesh->meshName = nextToken;
        //Next 3 are the position of the model
        theFile >> nextToken;                                               //x position for the model
        curMesh->positionXYZ.x = std::stof(nextToken);
        theFile >> nextToken;                                               //y position for the model
        curMesh->positionXYZ.y = std::stof(nextToken);
        theFile >> nextToken;                                               //z position for the model
        curMesh->positionXYZ.z = std::stof(nextToken);
        //Next 3 are the orientation of the model
        theFile >> nextToken;                                               //x orientation value
        //curMesh.orientationXYZ.x = std::stof(nextToken);
        curMesh->orientationXYZ.x = glm::radians(std::stof(nextToken));
        theFile >> nextToken;                                               //y orientation value
        //curMesh.orientationXYZ.y = std::stof(nextToken);
        curMesh->orientationXYZ.y = glm::radians(std::stof(nextToken));
        theFile >> nextToken;                                               //z orientation value
        //curMesh.orientationXYZ.z = std::stof(nextToken);
        curMesh->orientationXYZ.z = glm::radians(std::stof(nextToken));
        //Next is the scale to multiply the model by
        theFile >> nextToken;                                               //scale multiplier
        curMesh->scale = std::stof(nextToken);
        //Next 3 are the r, g, b values for the model
        curMesh->bUseWholeObjectDiffuseColour = true;
        theFile >> nextToken;													//RGB red value
        curMesh->wholeObjectDiffuseRGBA.r = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
        theFile >> nextToken;													//RGB green value
        curMesh->wholeObjectDiffuseRGBA.g = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
        theFile >> nextToken;													//RGB blue value
        curMesh->wholeObjectDiffuseRGBA.b = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
        theFile >> nextToken;
        curMesh->wholeObjectDiffuseRGBA.a = std::stof(nextToken);
        curMesh->alphaTransparency = std::stof(nextToken);

        curMesh->bDontLight = true;


        g_vec_pMeshes.push_back(curMesh);     //push the model onto our vector of meshes
        ss.str("");                         //reset the stringstream
    } //end of while
    theFile.close();
    return true;
} //end of loadWorldFile


//Figured out the math for how to do this from https://learnopengl.com/Getting-started/Camera and http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
//Using the mouse position we calculate the direction that the camera will be facing
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    //if it's the start of the program this smooths out a potentially glitchy jump
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    //find the offset of where the mouse positions have moved
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    //multiply by sensitivity so that it's not potentially crazy fast
    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;         // The yaw is the rotation around the camera's y-axis (so we want to add the xOffset to it)
    pitch += yOffset;       // The pitch is the rotation around the camera's x-axis (so we want to add the yOffset to it)
    //This limits the pitch so that we can't just spin the camera under/over itself
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    //calculations for the new direction based on the mouse movements
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    ::cameraTarget = glm::normalize(direction);
} //fly camera


//Modification of loadSounds from project 1, 
bool loadSounds()
{
    std::stringstream ss;
    std::stringstream sFile;
    ss << SOLUTION_DIR << "common\\assets\\audio\\audioList.txt";

    FMOD::Sound* sound;
    FMOD::Channel* channel;

    std::ifstream theFile(ss.str());
    ss.str("");

    if (!theFile.is_open())
    {
        fprintf(stderr, "Could not open audioList.txt");
        return false;
    }
    std::string nextToken;
    while (theFile >> nextToken)
    {
        if (nextToken == "end")
        {
            break;
        }

        if (nextToken.find("mp3") != std::string::npos || nextToken.find("wav") != std::string::npos)
        {
            sFile << nextToken.c_str();
            ss << SOLUTION_DIR << "common\\assets\\audio\\" << sFile.str().c_str();
            songNames.push_back(sFile.str().c_str());
            // LinearRollOff makes the sound go mute when the listener has moved away from the max distance
            _result = _system->createSound(ss.str().c_str(), FMOD_3D_LINEARROLLOFF, 0, &sound);
            if (_result != FMOD_OK)
            {
                fprintf(stderr, "Unable to create a sound: %s", ss.str().c_str());
            }
            else
            {
                //Set min/max distance
                _result = sound->set3DMinMaxDistance(0.5f, 8.0f);
                if (FMOD_OK != _result) {
                    fprintf(stderr, "Unable to set min/max distance for sound: %s", ss.str().c_str());
                    //return false;
                }
                //set loop to normal
                _result = sound->setMode(FMOD_LOOP_NORMAL);
                if (FMOD_OK != _result) {
                    fprintf(stderr, "Unable to set loop normal for sound: %s", ss.str().c_str());
                }

                _vecSounds.push_back(sound);
                _vecChannels.push_back(channel);
            }
            
            ss.str("");
            sFile.str("");
        }
        else
        {
            //in case audio file name contains spaces
            sFile << nextToken.c_str() << " ";
        }
    } //end while
    theFile.close();
    return true;
} //end of loadSounds

bool playSounds()
{
    // 0 - Bunny Mesh
    // 1 - Turret Mesh
    // 2 - Chairs Mesh
    // 3 - Bookshelf mesh
    // 4 - Pillar mesh
    for (unsigned int soundIndex = 0; soundIndex < 5; soundIndex++)
    {
        _result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
        if (FMOD_OK != _result) {
            fprintf(stderr, "Unable to play sound[%d]", soundIndex);
            return false;
        }
        FMOD_VECTOR position = { g_vec_pMeshes[soundIndex]->positionXYZ.x, g_vec_pMeshes[soundIndex]->positionXYZ.y, g_vec_pMeshes[soundIndex]->positionXYZ.z };//{ -4.2f, -0.05f, -2.0f };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
        //Set channel position and velocity
        _result = _vecChannels.at(soundIndex)->set3DAttributes(&position, &velocity);
        if (FMOD_OK != _result) {
            fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
            return false;
        }
        //Unpaused the sound.
        _result = _vecChannels.at(soundIndex)->setPaused(false);
        if (FMOD_OK != _result) {
            fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
            return false;
        }
    }

    ////TODO make sounds and place them around the scene
    //unsigned int soundIndex = 0;
    ////Play sound 1
    ////Bunny Mesh
    //_result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to play sound[%d]", soundIndex);
    //    return false;
    //}
    //FMOD_VECTOR position1 = { g_vec_pMeshes[0]->positionXYZ.x, g_vec_pMeshes[0]->positionXYZ.y, g_vec_pMeshes[0]->positionXYZ.z };//{ -4.2f, -0.05f, -2.0f };
    //FMOD_VECTOR velocity1 = { 0.0f, 0.0f, 0.0f };
    ////Set channel position and velocity
    //_result = _vecChannels.at(soundIndex)->set3DAttributes(&position1, &velocity1);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
    //    return false;
    //}
    //_result = _vecChannels[soundIndex]->addDSP(0, _dspPan);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to add dsp pan for channel[%d]", soundIndex);
    //    return false;
    //}
    ////Unpaused the sound.
    //_result = _vecChannels.at(soundIndex)->setPaused(false);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
    //    return false;
    //}

    //soundIndex++;

    ////Play sound 2
    ////Turret mesh
    //_result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to play sound[%d]", soundIndex);
    //    return false;
    //}
    //FMOD_VECTOR position2 = { g_vec_pMeshes[1]->positionXYZ.x, g_vec_pMeshes[1]->positionXYZ.y, g_vec_pMeshes[1]->positionXYZ.z };
    //FMOD_VECTOR velocity2 = { 0.0f, 0.0f, 0.0f };
    ////Set channel position and velocity
    //_result = _vecChannels.at(soundIndex)->set3DAttributes(&position2, &velocity2);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
    //    return false;
    //}
    ////Unpaused the sound.
    //_result = _vecChannels.at(soundIndex)->setPaused(false);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
    //    return false;
    //}

    //soundIndex++;

    ////Play sound 3
    ////Chairs mesh
    //_result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to play sound[%d]", soundIndex);
    //    return false;
    //}
    //FMOD_VECTOR position3 = { g_vec_pMeshes[2]->positionXYZ.x, g_vec_pMeshes[2]->positionXYZ.y, g_vec_pMeshes[2]->positionXYZ.z };
    //FMOD_VECTOR velocity3 = { 0.0f, 0.0f, 0.0f };
    ////Set channel position and velocity
    //_result = _vecChannels.at(soundIndex)->set3DAttributes(&position3, &velocity3);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
    //    return false;
    //}
    ////Unpaused the sound.
    //_result = _vecChannels.at(soundIndex)->setPaused(false);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
    //    return false;
    //}

    //soundIndex++;

    ////Play sound 4
    ////Bookshelf mesh
    //_result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to play sound[%d]", soundIndex);
    //    return false;
    //}
    //FMOD_VECTOR position4 = { g_vec_pMeshes[3]->positionXYZ.x, g_vec_pMeshes[3]->positionXYZ.y, g_vec_pMeshes[3]->positionXYZ.z };
    //FMOD_VECTOR velocity4 = { 0.0f, 0.0f, 0.0f };
    ////Set channel position and velocity
    //_result = _vecChannels.at(soundIndex)->set3DAttributes(&position4, &velocity4);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
    //    return false;
    //}
    ////Unpaused the sound.
    //_result = _vecChannels.at(soundIndex)->setPaused(false);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
    //    return false;
    //}

    //soundIndex++;

    ////Play sound 5
    ////Crystal mesh
    //_result = _system->playSound(_vecSounds.at(soundIndex), 0, true, &_vecChannels.at(soundIndex));
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to play sound[%d]", soundIndex);
    //    return false;
    //}
    //FMOD_VECTOR position5 = { g_vec_pMeshes[4]->positionXYZ.x, g_vec_pMeshes[4]->positionXYZ.y, g_vec_pMeshes[4]->positionXYZ.z };
    //FMOD_VECTOR velocity5 = { 0.0f, 0.0f, 0.0f };
    ////Set channel position and velocity
    //_result = _vecChannels.at(soundIndex)->set3DAttributes(&position5, &velocity5);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to set 3d settings for channel[%d]", soundIndex);
    //    return false;
    //}
    ////Unpaused the sound.
    //_result = _vecChannels.at(soundIndex)->setPaused(false);
    //if (FMOD_OK != _result) {
    //    fprintf(stderr, "Unable to unpause channel[%d]", soundIndex);
    //    return false;
    //}

    ////soundIndex++;

    return true;
} //end of playSounds