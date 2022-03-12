//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \version   3.2.0 $Rev: 1925 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;

//------------------------------------------------------------------------------
// STATES
//------------------------------------------------------------------------------
enum MouseState
{
    MOUSE_IDLE,
    MOUSE_SELECTION
};

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;


// a light source to illuminate the objects in the world
cDirectionalLight* light;

// virtual drill mesh
cMultiMesh* drill;

// virtual drill mesh
cMultiMesh* bolapalo1;
cMesh* single_bolapalo1;

// virtual drill mesh
cMultiMesh* bolapalo2;
cMesh* single_bolapalo2;

// virtual drill mesh
cMultiMesh* bolapalo3;
cMesh* single_bolapalo3;

// virtual drill mesh
cMultiMesh* palo;
cMesh* single_palo;

// virtual drill mesh
cMultiMesh* cuadradopalo;
cMesh* single_cuadradopalo;

// a virtual tooth mesh
cMultiMesh* caja;
cMesh* single_caja;

cMesh* needle;

// transparency level
double transparencyLevel = 0.3;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// side framebuffer
cFrameBufferPtr frameBuffer;
cFrameBufferPtr frameBuffer2;



// a colored background
cBackground* background;

// some objects
cMesh* mesh;
cShapeSphere* sphere;
cShapeBox* box;
cShapeCylinder* cylinder;
cShapeTorus* torus;
cShapeLine* line1;
cShapeLine* line2;

// a small sphere which displays the position of a click hit in the world
cShapeSphere* sphereSelect;

// a small line to display the surface normal at the selection point
cShapeLine* normalSelect;

// a pointer to the selected object
cGenericObject* selectedObject = NULL;

// offset between the position of the mmouse click on the object and the object reference frame location.
cVector3d selectedObjectOffset;


// side Panel that displays content of framebuffer
cViewPanel* viewPanel;
cViewPanel* viewPanel2;


// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a widget panel
cPanel* panel;

// some labels
cLabel* labelRed;
cLabel* labelGreen;
cLabel* labelBlue;
cLabel* labelOrange;
cLabel* labelGray;

// a label to explain what is happening
cLabel* labelMessage;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// root resource path
string resourceRoot;

// position of mouse click.
cVector3d selectedPoint;

// mouse position
double mouseX, mouseY;

// mouse state
MouseState mouseState = MOUSE_IDLE;






//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);

// callback to handle mouse click
void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to handle mouse motion
void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);

//==============================================================================
/*
    DEMO:    tooth.cpp

    This demonstration shows how to attach a 3D mesh file to a virtual
    tool. A second mesh (tooth) is loaded in the scene. By pressing the user
    switch of the haptic device it is possible to translate or orient
    the tooth object accordingly.

    In the main haptics loop function  "updateHaptics()" , the position
    of the haptic device is retrieved at each simulation iteration.
    The interaction forces are then computed and sent to the device.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 23-tooth" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[1, 2, 3, 4, 5, 6, 7, 8, 9] - Change needle position" << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;
    cout << "[up, down, right, left] - Move needle up, down, rith or left" << endl;
    cout << "[M] - vertical mirroring " << endl;
    cout << "[O] - increase transparency " << endl;
    cout << "[K] - decrease transparency " << endl;

    cout << endl << endl;

    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    //glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

    // set mouse position callback
    glfwSetCursorPosCallback(window, mouseMotionCallback);

    // set mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setWhite();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(2.5, 0.0, 0.6),    // camera position (eye)
        cVector3d(0.0, 0.0, 0.0),    // lookat position (target)
        cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

// set the near and far clipping planes of the camera
// anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(3.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // create a light source
    light = new cDirectionalLight(world);

    // attach light to camera
    camera->addChild(light);

    // enable light source
    light->setEnabled(true);

    // position the light source
    light->setLocalPos(0.0, 0.5, 0.0);

    // define the direction of the light beam
    light->setDir(-3.0, -0.5, 0.0);

    camera->setSphericalDeg(double(2.5),
        double(90),
        double(0.0)
    );







    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);


    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a tool (cursor) and insert into the world
    tool = new cToolCursor(world);
    world->addChild(tool);

    // connect the haptic device to the virtual tool
    tool->setHapticDevice(hapticDevice);

    // if the haptic device has a gripper, enable it as a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    // define the radius of the tool (sphere)
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // hide the device sphere. only show proxy.
    tool->setShowContactPoints(true, false);

    // create a white cursor
    tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    tool->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when
    // the tool is located inside an object for instance.
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();


    //--------------------------------------------------------------------------
    // COMPOSE OBJECTS
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // stiffness property
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "caja"
    /////////////////////////////////////////////////////////////////////////

    // create a virtual mesh
    caja = new cMultiMesh();
    single_caja = new cMesh();

    // add object to world
    //world->addChild(caja);

    // set the position and orientation of the object at the center of the world
    caja->setLocalPos(-1.05, 0.0, -0.775);
    caja->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // load an object file
    bool fileload;
    fileload = caja->loadFromFile(RESOURCE_PATH("../resources/models/caja/CAJA3.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = caja->loadFromFile("../../../bin/resources/models/caja/CAJA3.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    caja->convertToSingleMesh(single_caja);
    world->addChild(single_caja);

    // set the position and orientation of the object at the center of the world
    single_caja->setLocalPos(-1.05, 0.0, -0.775);
    single_caja->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // make the outside of the tooth rendered in semi-transparent
    caja->getMesh(0)->setUseTransparency(false);
    caja->getMesh(0)->setTransparencyLevel(transparencyLevel);

    // make the outside of the tooth rendered in semi-transparent
    single_caja->setUseTransparency(false);
    single_caja->setTransparencyLevel(transparencyLevel);

    // compute a boundary box
    caja->computeBoundaryBox(true);

    // resize tooth to screen
    caja->scale(0.01);
    single_caja->scale(0.01);


    // compute collision detection algorithm
    caja->createAABBCollisionDetector(toolRadius);


    // define a default stiffness for the object
    caja->setStiffness(0.8 * maxStiffness, true);

    single_caja->m_material->setGreenLight();


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "NEEDLE"
    /////////////////////////////////////////////////////////////////////////

    // create a new mesh.
    needle = new cMesh();

    //add needle as box child
    //box->addChild(mesh);

    //build needle using a cone primitive
    cCreateCone(needle,
        2.5,
        0.01,
        0.05,
        20,
        1,
        1,
        true,
        true,
        cVector3d(0, 0, 0),
        cMatrix3d(cVector3d(0, 1, 0), 1.571)
    );

    // set material color
    needle->m_material->setBlueCornflower();

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    needle->deleteCollisionDetector(true);

    // attach needle to tool
    tool->m_image->addChild(needle);


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "DRILL"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    drill = new cMultiMesh();

    // load a drill like mesh and attach it to the tool
    /*fileload = drill->loadFromFile(RESOURCE_PATH("../resources/models/drill/drill.3ds"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = drill->loadFromFile("../../../bin/resources/models/drill/drill.3ds");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    // resize tool mesh model
    drill->scale(0.004);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    drill->deleteCollisionDetector(true);*/

    // define a material property for the mesh
    cMaterial mat;
    mat.m_ambient.set(0.5f, 0.5f, 0.5f);
    mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
    mat.m_specular.set(1.0f, 1.0f, 1.0f);
    drill->setMaterial(mat, true);
    drill->computeAllNormals();

    // attach drill to tool
    tool->m_image->addChild(needle);

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "bolapalo1"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    bolapalo1 = new cMultiMesh();
    single_bolapalo1 = new cMesh();


    // load a drill like mesh and attach it to the tool
    fileload = bolapalo1->loadFromFile(RESOURCE_PATH("../resources/models/bolapalo1/Segmentation Bola con palo 1.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = bolapalo1->loadFromFile("../../../bin/resources/models/bolapalo1/Segmentation Bola con palo 1.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    bolapalo1->convertToSingleMesh(single_bolapalo1);
    world->addChild(single_bolapalo1);

    // set the position and orientation of the object at the center of the world
    single_bolapalo1->setLocalPos(-1.05, 0.0, -0.775);
    single_bolapalo1->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

   
    // make the outside of the tooth rendered in semi-transparent
    single_bolapalo1->setUseTransparency(false);
    single_bolapalo1->setTransparencyLevel(transparencyLevel);

    // resize tool mesh model
    bolapalo1->scale(0.01);
    single_bolapalo1->scale(0.01);

    // set the position and orientation of the object at the center of the world
    bolapalo1->setLocalPos(-1.05, 0.0, -0.775);
    bolapalo1->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    //drill->deleteCollisionDetector(true);

    // define a material property for the mesh
    mat.m_ambient.set(0.5f, 0.5f, 0.5f);
    mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
    mat.m_specular.set(1.0f, 1.0f, 1.0f);
    bolapalo1->setMaterial(mat, true);
    bolapalo1->computeAllNormals();

    // attach drill to tool
    //tool->m_image->addChild(bolapalo1);

    single_bolapalo1->m_material->setRedDark();

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "bolapalo2"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    bolapalo2 = new cMultiMesh();
    single_bolapalo2 = new cMesh();

    // load a drill like mesh and attach it to the tool
    fileload = bolapalo2->loadFromFile(RESOURCE_PATH("../resources/models/bolapalo2/Segmentation Bola con palo 2.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = bolapalo2->loadFromFile("../../../bin/resources/models/bolapalo2/Segmentation Bola con palo 2.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    bolapalo2->convertToSingleMesh(single_bolapalo2);
    world->addChild(single_bolapalo2);

    // set the position and orientation of the object at the center of the world
    single_bolapalo2->setLocalPos(-1.05, 0.0, -0.775);
    single_bolapalo2->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // make the outside of the tooth rendered in semi-transparent
    single_bolapalo2->setUseTransparency(false);
    single_bolapalo2->setTransparencyLevel(transparencyLevel);

    // resize tool mesh model
    bolapalo2->scale(0.01);
    single_bolapalo2->scale(0.01);

    // set the position and orientation of the object at the center of the world
    bolapalo2->setLocalPos(-1.05, 0.0, -0.775);
    bolapalo2->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    //drill->deleteCollisionDetector(true);

    // define a material property for the mesh
    mat.m_ambient.set(0.5f, 0.5f, 0.5f);
    mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
    mat.m_specular.set(1.0f, 1.0f, 1.0f);
    bolapalo2->setMaterial(mat, true);
    bolapalo2->computeAllNormals();

    // attach drill to tool
    //tool->m_image->addChild(bolapalo1);

    single_bolapalo2->m_material->setGray();

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "bolapalo3"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    bolapalo3 = new cMultiMesh();
    single_bolapalo3 = new cMesh();

    // load a drill like mesh and attach it to the tool
    fileload = bolapalo3->loadFromFile(RESOURCE_PATH("../resources/models/bolapalo3/Segmentation Bola con palo 3.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = bolapalo3->loadFromFile("../../../bin/resources/models/bolapalo3/Segmentation Bola con palo 3.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    bolapalo3->convertToSingleMesh(single_bolapalo3);
    world->addChild(single_bolapalo3);

    // set the position and orientation of the object at the center of the world
    single_bolapalo3->setLocalPos(-1.05, 0.0, -0.775);
    single_bolapalo3->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // make the outside of the tooth rendered in semi-transparent
    single_bolapalo3->setUseTransparency(false);
    single_bolapalo3->setTransparencyLevel(transparencyLevel);

    // resize tool mesh model
    bolapalo3->scale(0.01);
    single_bolapalo3->scale(0.01);

    // set the position and orientation of the object at the center of the world
    bolapalo3->setLocalPos(-1.05, 0.0, -0.775);
    bolapalo3->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    //drill->deleteCollisionDetector(true);

    // define a material property for the mesh
    mat.m_ambient.set(0.5f, 0.5f, 0.5f);
    mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
    mat.m_specular.set(1.0f, 1.0f, 1.0f);
    bolapalo3->setMaterial(mat, true);
    bolapalo3->computeAllNormals();

    // attach drill to tool
    //tool->m_image->addChild(bolapalo1);

    single_bolapalo3->m_material->setYellowGold();


    /////////////////////////////////////////////////////////////////////////
    // OBJECT "palo"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    palo = new cMultiMesh();
    single_palo = new cMesh();

    // load a drill like mesh and attach it to the tool
    fileload = palo->loadFromFile(RESOURCE_PATH("../resources/models/palo/Segmentation_Palo4.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = bolapalo1->loadFromFile("../../../bin/resources/models/palo/Segmentation_Palo4.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    palo->convertToSingleMesh(single_palo);
    world->addChild(single_palo);

    // set the position and orientation of the object at the center of the world
    single_palo->setLocalPos(-1.05, 0.0, -0.775);
    single_palo->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // make the outside of the tooth rendered in semi-transparent
    single_palo->setUseTransparency(false);
    single_palo->setTransparencyLevel(transparencyLevel);

    // resize tool mesh model
    palo->scale(0.01);
    single_palo->scale(0.01);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    //drill->deleteCollisionDetector(true);

    // attach drill to tool
    //tool->m_image->addChild(palo);

    single_palo->m_material->setOrange();

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "Cuadradopalo"
    /////////////////////////////////////////////////////////////////////////

    //create a new mesh
    cuadradopalo = new cMultiMesh();
    single_cuadradopalo = new cMesh();

    // load a drill like mesh and attach it to the tool
    fileload = cuadradopalo->loadFromFile(RESOURCE_PATH("../resources/models/cuadradopalo/Segmentation Cuadrado con palo.stl"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = cuadradopalo->loadFromFile("../../../bin/resources/models/cuadradopalo/Segmentation Cuadrado con palo.stl");
#endif
    }
    if (!fileload)
    {
        printf("Error - 3D Model failed to load correctly.\n");
        close();
        return (-1);
    }

    cuadradopalo->convertToSingleMesh(single_cuadradopalo);
    world->addChild(single_cuadradopalo);

    // set the position and orientation of the object at the center of the world
    single_cuadradopalo->setLocalPos(-1.05, 0.0, -0.775);
    single_cuadradopalo->rotateAboutGlobalAxisDeg(cVector3d(0.0, 0.0, 1.0), 90);

    // make the outside of the tooth rendered in semi-transparent
    single_cuadradopalo->setUseTransparency(false);
    single_cuadradopalo->setTransparencyLevel(transparencyLevel);

    // resize tool mesh model
    cuadradopalo->scale(0.01);
    single_cuadradopalo->scale(0.01);

    // remove the collision detector. we do not want to compute any
    // force feedback rendering on the object itself.
    //drill->deleteCollisionDetector(true);


    // attach drill to tool
    //tool->m_image->addChild(cuadradopalo);

    single_cuadradopalo->m_material->setBlack();


    //--------------------------------------------------------------------------
    // SIDE FRAMEBUFFER
    //--------------------------------------------------------------------------

    // create secondary camera for side view
    cCamera* cameraTool = new cCamera(world);

    // attach camera to tool
    needle->addChild(cameraTool);
    cameraTool->setLocalPos(0.0, 0.0, 0.1);

    // create framebuffer for side view
    frameBuffer = cFrameBuffer::create();
    frameBuffer->setup(cameraTool);

    // create panel to display side view
    viewPanel = new cViewPanel(frameBuffer);
    camera->m_frontLayer->addChild(viewPanel);
    viewPanel->setLocalPos(10, 10);






    double toolRadious = 0.01;
    // build collision detection tree
    single_bolapalo1->createAABBCollisionDetector(toolRadius);
    single_bolapalo2->createAABBCollisionDetector(toolRadius);
    single_bolapalo3->createAABBCollisionDetector(toolRadius);
    single_palo->createAABBCollisionDetector(toolRadius);
    single_cuadradopalo -> createAABBCollisionDetector(toolRadius);
    single_caja->createAABBCollisionDetector(toolRadius);


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();


    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setBlack();
    camera->m_frontLayer->addChild(labelRates);

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(1.00f, 1.00f, 1.00f),
        cColorf(0.95f, 0.95f, 0.95f),
        cColorf(0.85f, 0.85f, 0.85f),
        cColorf(0.80f, 0.80f, 0.80f));


    // create a small sphere to display a selection hit with the mouse
    sphereSelect = new cShapeSphere(0.005);
    world->addChild(sphereSelect);
    sphereSelect->m_material->setRedCrimson();
    sphereSelect->setShowEnabled(false);
    sphereSelect->setGhostEnabled(true);

    normalSelect = new cShapeLine();
    sphereSelect->addChild(normalSelect);
    normalSelect->m_colorPointA.setRedCrimson();
    normalSelect->m_colorPointB.setRedCrimson();
    normalSelect->setShowEnabled(false);
    normalSelect->setGhostEnabled(true);








    // SET TEXTURES FOR EVERY OBJECT

     
    



    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI22();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    camera->m_frontLayer->addChild(labelRates);

    // set font color
    labelRates->m_fontColor.setBlack();

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    background->setCornerColors(cColorf(1.0, 1.0, 1.0),
        cColorf(1.0, 1.0, 1.0),
        cColorf(0.8, 0.8, 0.8),
        cColorf(0.8, 0.8, 0.8));

    // a widget panel
    panel = new cPanel();
    camera->m_frontLayer->addChild(panel);
    panel->setSize(100, 115);
    panel->m_material->setGrayDim();
    panel->setTransparencyLevel(0.8);

    // create some labels
    labelRed = new cLabel(font);
    panel->addChild(labelRed);
    labelRed->setText("red");
    labelRed->setLocalPos(15, 10, 0.1);
    labelRed->m_fontColor.setWhite();

    labelGreen = new cLabel(font);
    panel->addChild(labelGreen);
    labelGreen->setText("green");
    labelGreen->setLocalPos(15, 30, 0.1);
    labelGreen->m_fontColor.setWhite();

    labelBlue = new cLabel(font);
    panel->addChild(labelBlue);
    labelBlue->setText("blue");
    labelBlue->setLocalPos(15, 50, 0.1);
    labelBlue->m_fontColor.setWhite();

    labelOrange = new cLabel(font);
    panel->addChild(labelOrange);
    labelOrange->setText("orange");
    labelOrange->setLocalPos(15, 70, 0.1);
    labelOrange->m_fontColor.setWhite();

    labelGray = new cLabel(font);
    panel->addChild(labelGray);
    labelGray->setText("gray");
    labelGray->setLocalPos(15, 90, 0.1);
    labelGray->m_fontColor.setWhite();

    // create a label with a small message
    labelMessage = new cLabel(font);
    camera->m_frontLayer->addChild(labelMessage);

    // set font color
    labelMessage->m_fontColor.setBlack();

    // set text message
    labelMessage->setText("use mouse to select and move objects");

    //posicionpanel
    panel->setLocalPos(20, 350, 0.0);

    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    //windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        windowSizeCallback(window, 0, 0);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    glfwGetFramebufferSize(window, &width, &height);

    // update size of framebuffer and view panel
    int side = 0.3 * cMin(width, height);
    frameBuffer->setSize(side, side);

    int radius = 0.25 * side;
    viewPanel->setSize(side, side);
    viewPanel->setCornerRadius(radius, radius, radius, radius);
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - wireframe
    else if (a_key == GLFW_KEY_P)
    {
        // only enable/disable wireframe of one part of the tooth model
        bool useWireMode = caja->getMesh(0)->getWireMode();
        caja->getMesh(0)->setWireMode(!useWireMode);
    }

    // option - decrease transparency
    else if (a_key == GLFW_KEY_K)
    {
        // decrease transparency level
        transparencyLevel = transparencyLevel - 0.1;
        if (transparencyLevel < 0.0) { transparencyLevel = 0.0; }

        // apply changes to tooth
        caja->getMesh(0)->setTransparencyLevel(transparencyLevel);
        caja->getMesh(0)->setUseTransparency(true);

        // if object is almost transparent, make it invisible
        if (transparencyLevel < 0.1)
        {
            caja->getMesh(0)->setShowEnabled(false, true);
            caja->getMesh(0)->setHapticEnabled(false, true);
        }
    }

    // option - increase transparency
    else if (a_key == GLFW_KEY_O)
    {
        // increase transparency level
        transparencyLevel = transparencyLevel + 0.1;
        if (transparencyLevel > 1.0) { transparencyLevel = 1.0; }

        // apply changes to tooth
        caja->getMesh(0)->setTransparencyLevel(transparencyLevel);

        // make object visible
        if (transparencyLevel >= 0.1)
        {
            caja->getMesh(0)->setShowEnabled(true, true);
            caja->getMesh(0)->setHapticEnabled(true, true);
        }

        // disable transparency is transparency level is set to 1.0
        if (transparencyLevel == 1.0)
        {
            caja->getMesh(0)->setUseTransparency(false);
        }
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }

    //move out
    else if (a_key == GLFW_KEY_S)
    {
        cVector3d drill_pos, new_pos;
        cVector3d movx(0.05, 0, 0);

        drill_pos = needle->getLocalPos();
        new_pos = drill_pos + movx;
        needle->setLocalPos(new_pos);
    }
    //move in
    else if (a_key == GLFW_KEY_W)
    {
        cVector3d drill_pos, new_pos;
        cVector3d movx(0.05, 0, 0);

        drill_pos = needle->getLocalPos();
        new_pos = drill_pos - movx;
        needle->setLocalPos(new_pos);
    }

    //move right
    else if (a_key == GLFW_KEY_RIGHT)
    {
        cVector3d drill_pos, new_pos;
        cVector3d movy(0, 0.05, 0);

        drill_pos = needle->getLocalPos();
        new_pos = drill_pos + movy;
        needle->setLocalPos(new_pos);
    }
    //move left
    else if (a_key == GLFW_KEY_LEFT)
    {
        cVector3d drill_pos, new_pos;
        cVector3d movy(0, 0.05, 0);

        drill_pos = needle->getLocalPos();
        new_pos = drill_pos - movy;
        needle->setLocalPos(new_pos);
    }

    //move up
    else if (a_key == GLFW_KEY_UP)
    {
        cVector3d drill_pos, new_pos;
        cVector3d movz(0, 0, 0.05);

        drill_pos = needle->getLocalPos();
        new_pos = drill_pos + movz;
        needle->setLocalPos(new_pos);
    }
    //move down
    else if (a_key == GLFW_KEY_DOWN)
    {
    cVector3d drill_pos, new_pos;
    cVector3d movz(0, 0, 0.05);

    drill_pos = needle->getLocalPos();
    new_pos = drill_pos - movz;
    needle->setLocalPos(new_pos);
    }
    //zoom
    else if (a_key == GLFW_KEY_I)
    {
    double cam_r, modified_r;

    cam_r = camera->getSphericalRadius();
    modified_r = cam_r - 0.1;
    //double rad = camera->getSphericalPolarRad();
    //double deg = camera->getSphericalPolarDeg();
    camera->setSphericalRadius(modified_r);
    //camera->setSphericalPolarRad(rad);
    //camera->setSphericalPolarDeg(deg);
    }
    else if (a_key == GLFW_KEY_O)
    {
    double cam_r, modified_r;

    cam_r = camera->getSphericalRadius();
    modified_r = cam_r + 0.1;
    camera->setSphericalRadius(modified_r);
    }

    //pos 1
    else if (a_key == GLFW_KEY_1)
    {
    cVector3d new_pos(0, -0.35, 0.35);

    needle->setLocalPos(new_pos);
    }

    //pos 2
    else if (a_key == GLFW_KEY_2)
    {
    cVector3d new_pos(0, 0, 0.35);

    needle->setLocalPos(new_pos);
    }

    //pos 3
    else if (a_key == GLFW_KEY_3)
    {
    cVector3d new_pos(0, 0.35, 0.35);

    needle->setLocalPos(new_pos);
    }

    //pos 4
    else if (a_key == GLFW_KEY_4)
    {
    cVector3d new_pos(0, -0.35, 0);

    needle->setLocalPos(new_pos);
    }

    //pos 5
    else if (a_key == GLFW_KEY_5)
    {
    cVector3d new_pos(0, 0, 0);

    needle->setLocalPos(new_pos);
    }

    //pos 6
    else if (a_key == GLFW_KEY_6)
    {
    cVector3d new_pos(0, 0.35, 0);

    needle->setLocalPos(new_pos);
    }

    //pos 7
    else if (a_key == GLFW_KEY_7)
    {
    cVector3d new_pos(0, -0.35, -0.35);

    needle->setLocalPos(new_pos);
    }

    //pos 8
    else if (a_key == GLFW_KEY_8)
    {
    cVector3d new_pos(0, 0, -0.35);

    needle->setLocalPos(new_pos);
    }

    //pos 9
    else if (a_key == GLFW_KEY_9)
    {
    cVector3d new_pos(0, 0.35, -0.35);

    needle->setLocalPos(new_pos);
    }
    
    else if (a_key == GLFW_KEY_T)
    {
        //////////////////////////////////////////////////////////////////////////
        //       TEXTURE
        /////////////////////////////////////////////////////////////////////////
            //new texture

        // create texture
        cTexture2dPtr texture = cTexture2d::create();

        // load texture file
        bool fileload2 = texture->loadFromFile(RESOURCE_PATH("../resources/images/spheremap-3.jpg"));
        if (!fileload2)
        {
    #if defined(_MSVC)
            fileload2 = texture->loadFromFile("../../../bin/resources/images/spheremap-3.jpg");
    #endif
        }
        if (!fileload2)
        {
            cout << "Error - Texture image failed to load correctly." << endl;
            close();
            //return (-1);
        }
        
        // setTexture
        // set graphic properties of sphere
        single_bolapalo1->setTexture(texture);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo1->setUseTexture(true);
        single_bolapalo1->m_material->setWhite();

        // setTexture
        // set graphic properties of sphere
        single_bolapalo2->setTexture(texture);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo2->setUseTexture(true);
        //single_bolapalo2->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_bolapalo3->setTexture(texture);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo3->setUseTexture(true);
        single_bolapalo3->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_cuadradopalo->setTexture(texture);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_cuadradopalo->setUseTexture(true);
        single_cuadradopalo->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_palo->setTexture(texture);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_palo->setUseTexture(true);
        single_palo->m_material->setWhite();



    }

        else if (a_key == GLFW_KEY_H)
        {
        //////////////////////////////////////////////////////////////////////////
        //       TEXTURE
        /////////////////////////////////////////////////////////////////////////
            //new texture

        // create texture
        cTexture2dPtr texture2 = cTexture2d::create();

        // load texture file
        bool fileload3 = texture2->loadFromFile(RESOURCE_PATH("../resources/images/whitefoam.jpg"));
        if (!fileload3)
        {
    #if defined(_MSVC)
            fileload3 = texture2->loadFromFile("../../../bin/resources/images/whitefoam.jpg");
    #endif
        }
        if (!fileload3)
        {
            cout << "Error - Texture image failed to load correctly." << endl;
            close();
            //return (-1);
        }

        // setTexture
        // set graphic properties of sphere
        single_bolapalo1->setTexture(texture2);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo1->setUseTexture(true);
        single_bolapalo1->m_material->setWhite();

        // setTexture
        // set graphic properties of sphere
        single_bolapalo2->setTexture(texture2);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo2->setUseTexture(true);
        //single_bolapalo2->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_bolapalo3->setTexture(texture2);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_bolapalo3->setUseTexture(true);
        single_bolapalo3->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_cuadradopalo->setTexture(texture2);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_cuadradopalo->setUseTexture(true);
        single_cuadradopalo->m_material->setWhite();


        // setTexture
        // set graphic properties of sphere
        single_palo->setTexture(texture2);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_palo->setUseTexture(true);
        single_palo->m_material->setWhite();



    }
        else if (a_key == GLFW_KEY_G)
        {
        //////////////////////////////////////////////////////////////////////////
        //       TEXTURE
        /////////////////////////////////////////////////////////////////////////
            //new texture

        // create texture
        cTexture2dPtr texture3 = cTexture2d::create();

        // load texture file
        bool fileload4 = texture3->loadFromFile(RESOURCE_PATH("../resources/images/brownboard.jpg"));
        if (!fileload4)
        {
#if defined(_MSVC)
            fileload4 = texture3->loadFromFile("../../../bin/resources/images/brownboard.jpg");
#endif
        }
        if (!fileload4)
        {
            cout << "Error - Texture image failed to load correctly." << endl;
            close();
            //return (-1);
        }

        // setTexture
        // set graphic properties of sphere
        single_caja->setTexture(texture3);
        //sphere->m_texture->setSpheriwecalMappingEnabled(true);
        single_caja->setUseTexture(true);
        single_caja->m_material->setWhite();
    }
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render side framebuffer
    frameBuffer->renderView();

    // render world
    camera->renderView(width, height);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // temp variable to store positions and orientations
    // of tooth and drill
    cVector3d lastPosObject;
    cMatrix3d lastRotObject;
    cVector3d lastPosDevice;
    cMatrix3d lastRotDevice;
    cVector3d lastDeviceObjectPos;
    cMatrix3d lastDeviceObjectRot;
    bool firstTime = false;

    // retrieve information about the current haptic device
    cHapticDeviceInfo info = hapticDevice->getSpecifications();

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // main haptic simulation loop
    while (simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////////
        // HAPTIC RENDERING
        /////////////////////////////////////////////////////////////////////////

        // signal frequency counter
        freqCounterHaptics.signal(1);

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();

        // send forces to haptic device
        tool->applyToDevice();



        /////////////////////////////////////////////////////////////////////////
        // MANIPULATION
        /////////////////////////////////////////////////////////////////////////

        // if the haptic device does track orientations, we automatically
        // orient the drill to remain perpendicular to the tooth
        cVector3d pos = tool->m_hapticPoint->getGlobalPosProxy();
        cMatrix3d rot = tool->getDeviceGlobalRot();

        if (info.m_sensedRotation == false)
        {
            rot.identity();

            cVector3d vx, vy, vz;
            cVector3d zUp(0, 0, 1);
            cVector3d yUp(0, 1, 0);
            vx = pos - caja->getLocalPos();
            if (vx.length() > 0.001)
            {
                vx.normalize();

                if (cAngle(vx, zUp) > 0.001)
                {
                    vy = cCross(zUp, vx);
                    vy.normalize();
                    vz = cCross(vx, vy);
                    vz.normalize();

                }
                else
                {
                    vy = cCross(yUp, vx);
                    vy.normalize();
                    vz = cCross(vx, vy);
                    vz.normalize();
                }

                rot.setCol(vx, vy, vz);
                drill->setLocalRot(rot);
            }
        }

        int button = tool->getUserSwitch(0);
        if (button == 0)
        {
            lastPosDevice = pos;
            lastRotDevice = rot;
            lastPosObject = caja->getLocalPos();
            lastRotObject = caja->getLocalRot();
            lastDeviceObjectPos = cTranspose(lastRotDevice) * ((lastPosObject - lastPosDevice) + 0.01 * cNormalize(lastPosObject - lastPosDevice));
            lastDeviceObjectRot = cMul(cTranspose(lastRotDevice), lastRotObject);
            drill->setTransparencyLevel(1.0, true, true);
            firstTime = true;
        }
        else if (firstTime)
        {
            drill->setTransparencyLevel(0.3, true, true);
            cMatrix3d newRot = cMul(rot, lastDeviceObjectRot);
            cVector3d newPos = cAdd(pos, cMul(rot, lastDeviceObjectPos));
            caja->setLocalPos(newPos);
            caja->setLocalRot(newRot);
            world->computeGlobalPositions(true);
        }
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------

void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{


    if (a_button == GLFW_MOUSE_BUTTON_RIGHT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);



        mouseState = MOUSE_SELECTION;
    }

    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // variable for storing collision information
        cCollisionRecorder recorder;
        cCollisionSettings settings;

        // detect for any collision between mouse and front layer widgets
        bool hit = camera->selectFrontLayer(mouseX, (height - mouseY), width, height, recorder, settings);
        if (hit)
        {
            // reset all label font colors to white
            labelRed->m_fontColor.setWhite();
            labelGreen->m_fontColor.setWhite();
            labelBlue->m_fontColor.setWhite();
            labelOrange->m_fontColor.setWhite();
            labelGray->m_fontColor.setWhite();

            // check mouse selection
            if (recorder.m_nearestCollision.m_object == labelRed)
            {
                labelRed->m_fontColor.setBlack();
                if (selectedObject != NULL)
                    selectedObject->m_material->setRedCrimson();
            }
            else if (recorder.m_nearestCollision.m_object == labelGreen)
            {
                labelGreen->m_fontColor.setBlack();
                if (selectedObject != NULL)
                    selectedObject->m_material->setGreenLightSea();
            }
            else if (recorder.m_nearestCollision.m_object == labelBlue)
            {
                labelBlue->m_fontColor.setBlack();
                if (selectedObject != NULL)
                    selectedObject->m_material->setBlueCornflower();
            }
            else if (recorder.m_nearestCollision.m_object == labelOrange)
            {
                labelOrange->m_fontColor.setBlack();
                if (selectedObject != NULL)
                    selectedObject->m_material->setOrangeRed();
            }
            else if (recorder.m_nearestCollision.m_object == labelGray)
            {
                labelGray->m_fontColor.setBlack();
                if (selectedObject != NULL)
                    selectedObject->m_material->setGrayLight();
            }
        }
        else
        {
            // detect for any collision between mouse and world
            bool hit = camera->selectWorld(mouseX, (height - mouseY), width, height, recorder, settings);
            if (hit)
            {
                //sphereSelect = cShapeSphere(0.101);


                sphereSelect->setShowEnabled(true);
                normalSelect->setShowEnabled(true);
                selectedPoint = recorder.m_nearestCollision.m_globalPos;
                sphereSelect->setLocalPos(selectedPoint);
                normalSelect->m_pointA.zero();
                normalSelect->m_pointB = 0.1 * recorder.m_nearestCollision.m_globalNormal;
                selectedObject = recorder.m_nearestCollision.m_object;
                selectedObjectOffset = recorder.m_nearestCollision.m_globalPos - selectedObject->getLocalPos();
                mouseState = MOUSE_SELECTION;
            }
        }
    }
    else
    {
        mouseState = MOUSE_IDLE;
    }
}


void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY)
{

    if (mouseState == MOUSE_SELECTION)
    {
        int posX = a_posX - mouseX;
        int posY = a_posY - mouseY;

        double r = camera->getSphericalRadius();
        double polarDeg = camera->getSphericalPolarDeg() - 0.2 * posY;
        double azimuthDeg = camera->getSphericalAzimuthDeg() - 0.2 * posX;

        camera->setSphericalRadius(r);
        camera->setSphericalAzimuthDeg(azimuthDeg);
        camera->setSphericalPolarDeg(polarDeg);


        mouseX = a_posX;
        mouseY = a_posY;
    }


}