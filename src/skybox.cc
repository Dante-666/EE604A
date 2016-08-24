#include <irrlicht/irrlicht.h>
#include <irrlicht/ILogger.h>
#include <irrlicht/irrString.h>

using namespace irr;

int main() {
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600));
    
    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();
    ILogger *logger = device->getLogger();

    device->setWindowCaption(L"Hello World");

    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

    scene::ISceneNode *skybox = scenemgr->addSkyBoxSceneNode(
            driver->getTexture("../data/skybox/ame_siege/siege_up.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_dn.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_lf.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_rt.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_ft.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_bk.tga"));

    driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
    skybox->setVisible(true);

    scene::ISceneNode *node = scenemgr->addAnimatedMeshSceneNode(scenemgr->getMesh("Soccer-Ball.obj"));
    node->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
    scene::ILightSceneNode *light = scenemgr->addLightSceneNode();

    if(node) {
        node->setPosition(core::vector3df(0, 0, 0));
        node->setScale(core::vector3df(30, 30, 30));
        light->setPosition(core::vector3df(-15, -15, -15));
        u32 count = node->getMaterialCount();
        core::stringc pval = core::stringc(count);
        const c8 *p = pval.c_str();;
        logger->log(p);
    }
    scenemgr->addCameraSceneNode(0, core::vector3df(-5, 0 ,0), core::vector3df(0, 0, 0));

    while(device->run() && driver) {
        driver->beginScene(true, true, video::SColor(255, 100, 101, 140));
        scenemgr->drawAll();
        driver->endScene();
    }

    driver->drop();

    return 0;

}
