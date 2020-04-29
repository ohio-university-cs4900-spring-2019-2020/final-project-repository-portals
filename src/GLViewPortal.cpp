#include "GLViewPortal.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "AftrGLRendererBase.h"



//If we want to use way points, we need to include this.
#include "io.h";
#include <WOFTGLString.h>
#include <MGLFTGLString.h>



using namespace Aftr;

GLViewPortal* GLViewPortal::New( const std::vector< std::string >& args )
{
   GLViewPortal* glv = new GLViewPortal( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}

void GLViewPortal::init(float gScalar, Vector gNormVec, std::string confFileName, const PHYSICS_ENGINE_TYPE& physEType) {
    GLView::init(gScalar, gNormVec, confFileName, physEType);
}


GLViewPortal::GLViewPortal( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewPortal::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewPortal::onCreate() is invoked after this module's LoadMap() is completed.
}

void GLViewPortal::onKeyDown(const SDL_KeyboardEvent& key)
{
    GLView::onKeyDown(key);
    if (key.keysym.sym == SDLK_0)
        this->setNumPhysicsStepsPerRender(1);
    if (key.keysym.sym == SDLK_w) {
        this->player->moveRelative(Vector(1,0,0));
    }
    if (key.keysym.sym == SDLK_s) {
        this->player->moveRelative(Vector(-1, 0, 0));
    }
    if (key.keysym.sym == SDLK_a) {
        this->player->moveRelative(Vector(0, 1, 0));
    }
    if (key.keysym.sym == SDLK_d) {
        this->player->moveRelative(Vector(0, -1, 0));
    }
    if (key.keysym.sym == SDLK_LCTRL) {
        this->player->setPosition(this->player->getPosition() + Vector(0, 0, -1));
    }
    if (key.keysym.sym == SDLK_SPACE) {
        this->player->setPosition(this->player->getPosition() + Vector(0, 0, 1));
    }
    if (key.keysym.sym == SDLK_UP) {
        this->cam->moveInLookDirection();
        this->cam->moveInLookDirection();
        this->cam->moveInLookDirection();
        this->cam->moveInLookDirection();
        this->cam->moveInLookDirection();
    }
    if (key.keysym.sym == SDLK_DOWN) {
        this->cam->moveOppositeLookDirection();
        this->cam->moveOppositeLookDirection();
        this->cam->moveOppositeLookDirection();
        this->cam->moveOppositeLookDirection();
        this->cam->moveOppositeLookDirection();
    }
    if (key.keysym.sym == SDLK_LEFT) {
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
    }
    if (key.keysym.sym == SDLK_RIGHT) {
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
    }
    if (key.keysym.sym == SDLK_l) {
        firstPerson = false;
        thirdPerson = false;
    }
    if (key.keysym.sym == SDLK_1) {
        firstPerson = true;
        thirdPerson = false;
    }
    if (key.keysym.sym == SDLK_3) {
        firstPerson = false;
        thirdPerson = true;
    }
    if (key.keysym.sym == SDLK_t) {
        firstPerson = true;
        thirdPerson = false;
        playingGame = true;
        this->player->setPosition(Vector(-190, -183, 5));
        spawnAsteroids();
    }
}


void GLViewPortal::onCreate()
{
    //GLViewPortal::onCreate() is invoked after this module's LoadMap() is completed.
    //At this point, all the managers are initialized. That is, the engine is fully initialized.

    if (this->pe != NULL)
    {

        this->pe->setGravityNormalizedVector(Vector(0, 0, -1.0f));
        this->pe->setGravityScalar(Aftr::GRAVITY);
    }
    this->setActorChaseType(STANDARDEZNAV);
    createEnv();
    initPlayer();
}

void GLViewPortal::updateWorld()
{
    GLView::updateWorld(); //Just call the parent's update world first.
                           //If you want to add additional functionality, do it after
                           //this call.

    // calculate delta time
    checkForPlayerTeleport();
    checkForPortalFlip();
    updatePortalCam();
    updatePlayerCam();
    if (playingGame) {
        playGame();
    }
}

void Aftr::GLViewPortal::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();


   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   loadDemo();
}

GLViewPortal::~GLViewPortal()
{

    //Implicitly calls GLView::~GLView()
}


void GLViewPortal::onResizeWindow(GLsizei width, GLsizei height)
{
    GLView::onResizeWindow(width, height); //call parent's resize method.
}


void GLViewPortal::onMouseDown(const SDL_MouseButtonEvent& e)
{
    GLView::onMouseDown(e);
}


void GLViewPortal::onMouseUp(const SDL_MouseButtonEvent& e)
{
    GLView::onMouseUp(e);
}


void GLViewPortal::onMouseMove(const SDL_MouseMotionEvent& e)
{
    GLView::onMouseMove(e);
}

void GLViewPortal::onKeyUp(const SDL_KeyboardEvent& key)
{
    GLView::onKeyUp(key);
}


/*
    Function: loadDemo
    Description: Initializes a few world objects and portals for the demo of the portals.
    Also initializes the portals and their corresponding cameras.

*/

void GLViewPortal::loadDemo() {

    std::string chest(ManagerEnvironmentConfiguration::getLMM() + "/models/bottle.wrl");
    WO* chestWO = WO::New(chest, Vector(20, 20, 20), MESH_SHADING_TYPE::mstAUTO);
    chestWO->setPosition(40, 100, 5);
    chestWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(chestWO);

    std::string box(ManagerEnvironmentConfiguration::getLMM() + "/models/box.blend");
    WO* wo2 = WO::New(box, Vector(1, 1, 1), MESH_SHADING_TYPE::mstAUTO);
    wo2->setPosition(40, 70, 5);
    wo2->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo2);

    std::string stone(ManagerEnvironmentConfiguration::getLMM() + "/models/stone.wrl");
    WO* stoneWO = WO::New(stone, Vector(1, 1, 1), MESH_SHADING_TYPE::mstAUTO);
    stoneWO->setPosition(0, 100, 5);
    stoneWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(stoneWO);

    std::string cone(ManagerEnvironmentConfiguration::getLMM() + "/models/cone.wrl");
    WO* coneWO = WO::New(cone, Vector(1, 1, 1), MESH_SHADING_TYPE::mstAUTO);
    coneWO->setPosition(0, 70, 5);
    coneWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(coneWO);


    portalCam1 = new Camera*;
    portalCam2 = new Camera*;
    *portalCam1 = new CameraStandardEZNav(this, &this->mouseHandler);
    *portalCam2 = new CameraStandardEZNav(this, &this->mouseHandler);
    this->portal1 = WOCameraSink::New(portalCam1, (WorldList*)worldLst, 5, 10, 10);
    this->portal1->setPosition(20, 100, 5);
    this->portal2 = WOCameraSink::New(portalCam2, (WorldList*)worldLst, 5, 10, 10);
    this->portal2->setPosition(20, 70, 5);

    this->portal1->getModel()->getSkin().getMultiTextureSet().at(0) = this->portal1->getFbo()->generateTextureFromFBOTextureOwnsTexDataSharesGLHandle();
    this->portal2->getModel()->getSkin().getMultiTextureSet().at(0) = this->portal2->getFbo()->generateTextureFromFBOTextureOwnsTexDataSharesGLHandle();
    worldLst->push_back(portal1);
    worldLst->push_back(portal2);

    portal1->rotateAboutGlobalZ(PI);
    portal2->rotateAboutGlobalZ(PI);


    portalCam3 = new Camera*;
    portalCam4 = new Camera*;
    *portalCam3 = new CameraStandardEZNav(this, &this->mouseHandler);
    *portalCam4 = new CameraStandardEZNav(this, &this->mouseHandler);
    this->portal3 = WOCameraSink::New(portalCam3, (WorldList*)worldLst, 5, 10, 10);
    this->portal3->setPosition(190, -183, 5);
    this->portal4 = WOCameraSink::New(portalCam4, (WorldList*)worldLst, 5, 10, 10);
    this->portal4->setPosition(100, -100, 5);

    this->portal3->getModel()->getSkin().getMultiTextureSet().at(0) = this->portal3->getFbo()->generateTextureFromFBOTextureOwnsTexDataSharesGLHandle();
    this->portal4->getModel()->getSkin().getMultiTextureSet().at(0) = this->portal4->getFbo()->generateTextureFromFBOTextureOwnsTexDataSharesGLHandle();
    worldLst->push_back(portal3);
    worldLst->push_back(portal4);

    portal3->rotateAboutGlobalZ(PI);
    portal4->rotateAboutGlobalZ(PI);

    for (int i = 0; i < 4; i++) {
       portalFlips.push_back(false);
    }
}

/*
    Function: initPlayer
    Description: Initializes the player.

*/

void GLViewPortal::initPlayer() {
    std::string apple(ManagerEnvironmentConfiguration::getLMM() + "/models/apple.wrl");
    player = WO::New(apple, Vector(20,20,20), MESH_SHADING_TYPE::mstAUTO);
    player->setPosition(-10, 80, 5);
    worldLst->push_back(player);

}

/*
    Function: updatePlayerCam
    Description: If in first person mode, updates the player cam to follow the position of the player and sets the player invisible.
    If in third person mode, updates the player cam to a third person POV following the camera and sets the player visible.

*/

void GLViewPortal::updatePlayerCam() {
    if (firstPerson) {
        this->cam->setPosition(this->player->getPosition());
        this->player->isVisible = false;
    }
    else if (thirdPerson) {
        this->player->isVisible = true;
        this->cam->setPosition(this->player->getPosition() + Vector(-5, 0, 5));
    }
    else {
        this->player->isVisible = true;
    }
}

/*
    Function: updatePortalCam
    Description: Updates the position of each portal cam to its corresponding portal's position.

*/

void GLViewPortal::updatePortalCam() {
    (*portalCam1)->setPosition(portal2->getPosition());
    (*portalCam2)->setPosition(portal1->getPosition());
    (*portalCam3)->setPosition(portal4->getPosition());
    (*portalCam4)->setPosition(portal3->getPosition());
}

/*
    Function: checkForPortalFlip
    Description: By default the WOCameraSink objects don't flip their cameras depending on what side the player is looking at it from.
    This function does this for them. Its rotates or flips the cameras by 180 degrees when the player is on a different side of the portal.

*/

void GLViewPortal::checkForPortalFlip() {
    if (this->cam->getPosition().x > portal1->getPosition().x && portalFlips[0] == false) {
        (*portalCam1)->rotateAboutGlobalZ(PI);
        portalFlips[0] = true;
    }
    else if (this->cam->getPosition().x < portal1->getPosition().x && portalFlips[0] == true) {
        (*portalCam1)->rotateAboutGlobalZ(PI);
        portalFlips[0] = false;
    }
    if (this->cam->getPosition().x > portal2->getPosition().x && portalFlips[1] == false) {
        (*portalCam2)->rotateAboutGlobalZ(PI);
        portalFlips[1] = true;
    }
    else if (this->cam->getPosition().x < portal2->getPosition().x && portalFlips[1] == true) {
        (*portalCam2)->rotateAboutGlobalZ(PI);
        portalFlips[1] = false;
    }
    if (this->cam->getPosition().x > portal3->getPosition().x && portalFlips[2] == false) {
        (*portalCam3)->rotateAboutGlobalZ(PI);
        portalFlips[2] = true;
    }
    else if (this->cam->getPosition().x < portal3->getPosition().x && portalFlips[2] == true) {
        (*portalCam3)->rotateAboutGlobalZ(PI);
        portalFlips[2] = false;
    }
    if (this->cam->getPosition().x > portal4->getPosition().x && portalFlips[3] == false) {
        (*portalCam4)->rotateAboutGlobalZ(PI);
        portalFlips[3] = true;
    }
    else if (this->cam->getPosition().x < portal4->getPosition().x && portalFlips[3] == true) {
        (*portalCam4)->rotateAboutGlobalZ(PI);
        portalFlips[3] = false;
    }
}

/*
    Function: checkForPlayerTeleport
    Description: Checks to see if a player has entered one of the four portals and if so it teleports them to the corresponding portal using
    the onTeleport function.

*/

void GLViewPortal::checkForPlayerTeleport() {
    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            if (player->getPosition() == portal1->getPosition() + Vector(0, i, j)) {
                onTeleport(1, i, j);
                std::cout << "Portal 1 Teleported!" << std::endl;
            }
            if (player->getPosition() == portal2->getPosition() + Vector(0, i, j)) {
                onTeleport(2, i, j);
                std::cout << "Portal 2 Teleported!" << std::endl;
            }
            if (player->getPosition() == portal3->getPosition() + Vector(0, i, j)) {
                onTeleport(3, i, j);
                std::cout << "Portal 3 Teleported!" << std::endl;
            }
            if (player->getPosition() == portal4->getPosition() + Vector(0, i, j)) {
                onTeleport(4, i, j);
                std::cout << "Portal 4 Teleported!" << std::endl;
            }
        }
    }
    oldPlayerPos = player->getPosition();
}

/*
    Function: onTeleport
    Description: When the player teleports this function determines where the players new position will be depending on 
    what portal it entered.

*/


void GLViewPortal::onTeleport(int pIndex, int i, int j) {
    if (pIndex == 1) {
        if (oldPlayerPos.x > player->getPosition().x) {
            player->setPosition(portal2->getPosition() + Vector(0, i, j) + Vector(-1, 0, 0));
        }
        else {
            player->setPosition(portal2->getPosition() + Vector(0, i, j) + Vector(1, 0, 0));
        }
    }
    else if(pIndex == 2) {
        if (oldPlayerPos.x > player->getPosition().x) {
            player->setPosition(portal1->getPosition() + Vector(0, i, j) + Vector(-1, 0, 0));
        }
        else {
            player->setPosition(portal1->getPosition() + Vector(0, i, j) + Vector(1, 0, 0));
        }
    }
    else if (pIndex == 3) {
        if (oldPlayerPos.x > player->getPosition().x) {
            player->setPosition(portal4->getPosition() + Vector(0, i, j) + Vector(-1, 0, 0));
        }
        else {
            player->setPosition(portal4->getPosition() + Vector(0, i, j) + Vector(1, 0, 0));
        }
    }
    else if (pIndex == 4) {
        if (oldPlayerPos.x > player->getPosition().x) {
            player->setPosition(portal3->getPosition() + Vector(0, i, j) + Vector(-1, 0, 0));
        }
        else {
            player->setPosition(portal3->getPosition() + Vector(0, i, j) + Vector(1, 0, 0));
        }
    }
}

/*
    Function: spawnWalls
    Description: Spawns all the wall found at the edge of the grass plane.

*/

void GLViewPortal::spawnWalls() {
    std::string wall(ManagerEnvironmentConfiguration::getLMM() + "/models/wallcenter.wrl");
    for (int i = 0; i < 11; i++) {
        WO* wall1 = WO::New(wall, Vector(10, 10, 10), MESH_SHADING_TYPE::mstAUTO);
        wall1->rotateAboutGlobalZ(PI / 2);
        wall1->setPosition(-180 + (36 * i), -200, 15);
        wall1->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(wall1);

        WO* wall2 = WO::New(wall, Vector(10, 10, 10), MESH_SHADING_TYPE::mstAUTO);
        wall2->rotateAboutGlobalZ(PI / 2);
        wall2->setPosition(-180 + (36 * i), -164, 15);
        wall2->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(wall2);
    }

        WO* backWall = WO::New(wall, Vector(10, 10, 10), MESH_SHADING_TYPE::mstAUTO);
        backWall->setPosition(-200, -182 , 15);
        backWall->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(backWall);

        WO* frontWall = WO::New(wall, Vector(10, 10, 10), MESH_SHADING_TYPE::mstAUTO);
        frontWall->setPosition(200, -182 , 15);
        frontWall->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(frontWall);
}

/*
    Function: spawnAsteroids
    Description: Spawns the asteroids at the start of the game.

*/

void GLViewPortal::spawnAsteroids() {
    std::string asteroid(ManagerEnvironmentConfiguration::getLMM() + "/models/spaceAsteroid.wrl");
    WO* wo1 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo1->setPosition(190, -193, 5);
    wo1->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo1);
    asteroids.push_back(wo1);


    WO* wo2 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo2->setPosition(190, -183, 5);
    wo2->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo2);
    asteroids.push_back(wo2);

    WO* wo3 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo3->setPosition(190, -173, 5);
    wo3->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo3);
    asteroids.push_back(wo3);

    WO* wo4 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo4->setPosition(190, -193, 15);
    wo4->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo4);
    asteroids.push_back(wo4);

    WO* wo5 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo5->setPosition(190, -183, 15);
    wo5->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo5);
    asteroids.push_back(wo5);

    WO* wo6 = WO::New(asteroid, Vector(5, 5, 5), MESH_SHADING_TYPE::mstAUTO);
    wo6->setPosition(190, -173, 15);
    wo6->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo6);
    asteroids.push_back(wo6);
}

/*
    Function: playGame
    Description: 
    First it moves the asteroids along the spawned in asteroids towards the player along the x axis.
    Second it then checks if the player has dodged or moved past the asteroids and resets their positions.
    Thirdly it checks if the player has tried to exit the bound of the game by calling the checkBoundsCollision function.
    Finally it checks if the player has collided with any of the asteroids by calling the checkAsteroidCollision function.
*/

void GLViewPortal::playGame() {
    for (int i = 0; i < asteroids.size(); i++) {
        asteroids[i]->setPosition(asteroids[i]->getPosition() - Vector(1, 0, 0));
    }

    if (player->getPosition().x > asteroids[0]->getPosition().x && player->getPosition().x < 100) {
        for (int i = 0; i < asteroids.size(); i++) {
            int j = 5 - i;
            if (asp == 1) {
                if (i <= (asteroids.size() / 2) - 1) {
                    asteroids[i]->setPosition(190, -193 + (10 * i), 5);
                }
                else {
                    asteroids[i]->setPosition(190, -193 + (10 * j), 25);
                }
            }
            else if (asp == 2) {
                if (i <= (asteroids.size() / 2) - 1) {
                    asteroids[i]->setPosition(190, -193 + (10 * i), 5);
                }
                else {
                    asteroids[i]->setPosition(190, -193 + (10 * j), 15);
                }
            }
            else if (asp == 3) {
                if (i <= (asteroids.size() / 2) - 1) {
                    asteroids[i]->setPosition(190, -193 + (10 * i), 15);
                }
                else {
                    asteroids[i]->setPosition(190, -193 + (10 * j), 25);
                }
            }
        }
        asp = rand() % 3 + 1;
    }
    if (player->getPosition().x < 100) {
        checkBoundsCollision();
    }
    checkAsteroidCollision();
}

/*
    Function: checkAsteroidCollision
    Description: Checks to see if the player has collided with any of the spawned asteroids and resets the player
    to the spawn position if they did.

*/

void GLViewPortal::checkAsteroidCollision() {
    for (int i = 0; i < asteroids.size(); i++) {
        for (int j = -5; j < 5; j++) {
            for (int k = -5; k < 5; k++) {
                for (int l = -5; l < 5; l++) {
                    if (player->getPosition() == asteroids[i]->getPosition() + Vector(j, k, l)) {
                        this->player->setPosition(Vector(-190, -183, 5));
                    }
                }
            }
        }
    }
}

/*
    Function: checkBoundsCollision
    Description: Checks to see if the player collided with the boundaries of the game.
    The walls, the ceiling, or the floor. Teleports the player back away from the boundary
    if they do.

*/

void GLViewPortal::checkBoundsCollision() {
    int playerX = player->getPosition().x;
    int playerY = player->getPosition().y;
    int playerZ = player->getPosition().z;
    if (playerY > -168 || playerY < -198) {  
        this->player->setPosition(Vector(playerX, -183, playerZ));
    }

    if (playerZ < 1 || playerZ > 30) {
        this->player->setPosition(Vector(playerX, playerY, 5));
    }
}

/*
    Function: createEnv
    Description: Creates the lighting, grass plane, walls, and skyboxes.

*/

void GLViewPortal::createEnv() {
    std::string grass(ManagerEnvironmentConfiguration::getLMM() + "/models/grassFloor400x400_pp.wrl");

    //SkyBox Textures readily available
    std::vector< std::string > skyBoxImageNames; //vector to store texture paths
    skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getLMM() + "/images/skyboxes/space_ice_field+6.jpg");

    float ga = 0.1f; //Global Ambient Light level for this module
    ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
    WOLight* light = WOLight::New();
    light->isDirectionalLight(true);
    light->setPosition(Vector(0, 0, 100));
    //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
    //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
    light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({ 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD));
    light->setLabel("Light");
    worldLst->push_back(light);

    //Create the SkyBox
    WO* wo = WOSkyBox::New(skyBoxImageNames.at(0), this->getCameraPtrPtr());
    wo->setPosition(Vector(0, 0, 0));
    wo->setLabel("Sky Box");
    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo);

    ////Create the infinite grass plane (the floor)
    wo = WO::New(grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
    wo->setPosition(Vector(0, 0, 0));
    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
    grassSkin.getMultiTextureSet().at(0)->setTextureRepeats(5.0f);
    grassSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
    grassSkin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
    grassSkin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
    grassSkin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
    wo->setLabel("Grass");
    worldLst->push_back(wo);

    spawnWalls();
}

//Skyboxes
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
//skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
 //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );