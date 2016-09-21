#include <iostream>

#include <irrlicht/irrlicht.h>

#include <opencv2/opencv.hpp>

using namespace irr;
using namespace std;

int main() {
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600));
    
    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();
    gui::IGUIEnvironment *guienv = device->getGUIEnvironment();
    ILogger *logger = device->getLogger();

    device->setWindowCaption(L"Hello World");
    guienv->addStaticText(L"This is a test", core::rect<s32>(10,10, 260, 22));

    scene::IAnimatedMesh *mesh = scenemgr->getMesh("../data/models/IPhone/IPhone 4Gs _5.obj");
    scene::ISceneNode *node = scenemgr->addAnimatedMeshSceneNode(mesh);
    scene::ISceneNode *dummy = scenemgr->addEmptySceneNode();
    scene::ILightSceneNode *light = scenemgr->addLightSceneNode();

    if(node) {
        node->setPosition(core::vector3df(0, 0, 0));
        node->setRotation(core::vector3df(0, -90, 0));

        dummy->setPosition(core::vector3df(1.7046/2, 0, 3.2837/2));
        node->setParent(dummy);

        //dummy->setPosition(core::vector3df(-1, 1, 0));
        char buf[33];
        snprintf(buf, 33, "%f,%f,%f", node->getPosition().X, node->getPosition().Y, node->getPosition().Z);
        logger->log(buf);
        scene::IMeshManipulator *mmp = driver->getMeshManipulator();
        mmp->flipSurfaces(mesh);
        light->setPosition(core::vector3df(0, 0, 15));
    }
    /**
     * Because of normals and lighting issue, place the camera at -z, and work out the parabola.
     */
    scene::ICameraSceneNode *cam = scenemgr->addCameraSceneNode(0, core::vector3df(0, 0, -5), core::vector3df(0, 0, 0));

    while(device->run() && driver) {
        driver->beginScene(true, true, video::SColor(255, 100, 101, 140));
        scenemgr->drawAll();
        guienv->drawAll();
        /**
         * Get the coordinates of the face.
         * Optional : write code such that the first face in the image is chosen
         * 1) Base this on the area of the rectangle formed.
         *
         * The pseudocode here moves the camera in 0.1, 0.1 direction.
         */
        core::vector3df curr = cam->getPosition();
        core::vector3df next = curr + core::vector3df(0, 0, 0);
        cam->setPosition(next);
        cam->setTarget(node->getPosition());        

        driver->endScene();
    }

    driver->drop();

    return 0;

}
