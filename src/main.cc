#include <iostream>

#include <irrlicht/irrlicht.h>

#include <opencv2/opencv.hpp>

using namespace irr;
using namespace std;
using namespace cv;

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) 
        return -1;
    
    float scale = 15.f;
    String face_cascade = "../config/haarcascade_frontalface_default.xml";
    CascadeClassifier face;
    if(!face.load(face_cascade)) return -1;
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600));

    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();
    gui::IGUIEnvironment *guienv = device->getGUIEnvironment();
    ILogger *logger = device->getLogger();

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

        dummy->setPosition(core::vector3df(-1, 1, 0));
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

    Mat frame;
    Mat grey;
    vector<Rect> faces;

    int lastFPS = -1;

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
        cap >> frame;
        if(frame.empty()) {
            driver->endScene();
            continue;
        }

        cvtColor(frame, grey, COLOR_BGR2GRAY);

        face.detectMultiScale(grey, faces, 1.1, 3, 0);
        int r_y = faces[0].y + (int) round(faces[0].width/2.6);
        int r_x = faces[0].x + (int) faces[0].height/2;

        int h_2 = frame.rows/2;
        int w_2 = frame.cols/2;

        float x = (w_2 - r_x)/scale;
        float y = (h_2 - r_y)/scale;

        char buf[33];
        snprintf(buf, 33, "%f,%f", x, y);
        logger->log(buf);

        //float z = 5 * sqrt((1 - pow(x,2)/25 - pow(y, 2)/100));
        
        cam->setPosition(core::vector3df(x, y, -5));
        cam->setTarget(dummy->getPosition());        

        driver->endScene();

        int fps = driver->getFPS();

        if (lastFPS != fps) {
            core::stringw tmp(L"Movement Example - Irrlicht Engine [");
            tmp += driver->getName();
            tmp += L"] fps: ";
            tmp += fps;

            device->setWindowCaption(tmp.c_str());
            lastFPS = fps;
        }
    }

    driver->drop();

    return 0;

}
