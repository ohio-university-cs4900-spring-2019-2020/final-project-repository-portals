#pragma once

#include "GLView.h";
#include <WOCameraSink.h>
#include "AftrFrameBufferObject.h"
#include <WOFTGLString.h>

namespace Aftr
{
    class Camera;

    class GLViewPortal : public GLView {
        public:
           static GLViewPortal* New( const std::vector< std::string >& outArgs );
           virtual ~GLViewPortal();
           virtual void init(float gScalar, Vector gravityNormVec, std::string confFileName, const PHYSICS_ENGINE_TYPE& physEType);
           virtual void updateWorld(); ///< Called once per frame
           virtual void loadMap(); ///< Called once at startup to build this module's scene
           virtual void onResizeWindow( GLsizei width, GLsizei height );
           virtual void onMouseDown( const SDL_MouseButtonEvent& e );
           virtual void onMouseUp( const SDL_MouseButtonEvent& e );
           virtual void onMouseMove( const SDL_MouseMotionEvent& e );
           virtual void onKeyDown( const SDL_KeyboardEvent& key );
           virtual void onKeyUp( const SDL_KeyboardEvent& key );
           virtual void createEnv();
           virtual void initPlayer();
           virtual void updatePlayerCam();
           virtual void loadDemo();
           virtual void updatePortalCam();
           virtual void checkForPortalFlip();
           virtual void checkForPlayerTeleport();
           virtual void onTeleport(int pIndex, int i, int j);
           virtual void spawnWalls();
           virtual void spawnAsteroids();
           virtual void playGame();
           virtual void checkAsteroidCollision();
           virtual void checkBoundsCollision();
        
        protected:
           GLViewPortal( const std::vector< std::string >& args );
           virtual void onCreate();
           WOCameraSink* portal1;
           WOCameraSink* portal2;
           Camera** portalCam1;
           Camera** portalCam2;
           WOCameraSink* portal3;
           WOCameraSink* portal4;
           Camera** portalCam3;
           Camera** portalCam4;
           WO* player;
           bool firstPerson = true;
           bool thirdPerson = false;
           bool playingGame = false;
           int asp = 2;
           std::vector<WO*> asteroids;
           std::vector<bool> portalFlips;
           Vector oldPlayerPos;

};

} 
