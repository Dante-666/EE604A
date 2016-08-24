#include <irrlicht/irrlicht.h>

using namespace irr;

int main() {
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600));
    
    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();

    device->setWindowCaption(L"Hello World");

    scene::IAnimatedMesh *mesh = scenemgr->getMesh("../data/models/IPhone/IPhone 4Gs _5.obj");
    scene::ISceneNode *node = scenemgr->addAnimatedMeshSceneNode(mesh);
    scene::ILightSceneNode *light = scenemgr->addLightSceneNode();

    if(node) {
        node->setPosition(core::vector3df(0, -1, 0));
        scene::IMeshManipulator *mmp = driver->getMeshManipulator();
        mmp->flipSurfaces(mesh);
        light->setPosition(core::vector3df(-5, -1, 0));
    }
    scenemgr->addCameraSceneNode(0, core::vector3df(5, 0, 0), core::vector3df(0, 0, 0));

    while(device->run() && driver) {
        driver->beginScene(true, true, video::SColor(255, 100, 101, 140));
        scenemgr->drawAll();
        core::vector3df curr = node->getRotation();
        core::vector3df next = curr + core::vector3df(0, 1, 0);
        node->setRotation(next);
        driver->endScene();
    }

    driver->drop();

    return 0;

}
