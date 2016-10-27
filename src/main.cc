#include <iostream>
#include <queue>
#include <mutex>
#include <thread>

#include <irrlicht/irrlicht.h>
#include <irrlicht/driverChoice.h>

#include <opencv2/opencv.hpp>

using namespace irr;
using namespace std;
using namespace cv;

struct point {
    float x;
    float y;
};

struct point3 {
    float x;
    float y;
    float z;
};

queue<point> data;
mutex q_mutex;
point diff_2;
point diff_1;
point diff_0;

class KeyReceiver : public IEventReceiver
{
    public:
    //This is the one method that we have to implement
        virtual bool OnEvent(const SEvent& event) {
            // Remember whether each key is down or up
            if (event.EventType == EET_KEY_INPUT_EVENT) 
                KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
            return false;
        }

        // This is used to check whether a key is being held down
        virtual bool IsKeyDown(EKEY_CODE keyCode) const
        {
            return KeyIsDown[keyCode];
        }
    
        KeyReceiver()
        {
            for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
                KeyIsDown[i] = false;
        }

    private:
        // We use this array to store the current state of each key
        bool KeyIsDown[KEY_KEY_CODES_COUNT];
};


void fetchData() { 
    VideoCapture cap(0);
    if (!cap.isOpened()) return;
    
    float scale = .0951447f;
    float thresh = .2366f;

    String face_cascade = "../config/haarcascade_frontalface_default.xml";
    CascadeClassifier face;
    if(!face.load(face_cascade)) return;

    Mat frame;
    Mat grey;
    vector<Rect> faces;

    float x, y, x_ = -1, y_ = -1;

    while(true) {

        cap >> frame;
        if(frame.empty()) {
            //Interpolate some values here.
            continue;
        }

        cvtColor(frame, grey, COLOR_BGR2GRAY);

        face.detectMultiScale(grey, faces, 1.3, 3, 0);

        int area = 0;
        Rect big_face;

        for (unsigned int i = 0; i < faces.size(); i++) {
            if (area <= faces[i].x * faces[i].y) {
                area = faces[i].x * faces[i].y;
                big_face = faces[i];
            }
        }

        int r_y = big_face.y + (int) round(big_face.height/2.6);
        int r_x = big_face.x + (int) big_face.width/2;

        if (r_y == 0 && r_x == 0) continue;

        int h_2 = frame.rows/2;
        int w_2 = frame.cols/2;

        x = (w_2 - r_x) * scale;
        y = (h_2 - r_y) * scale;
        
        /**
         * Interpolate some values here between z_ and z
         */
        /**
         * Smooth the values here using exponential smoothing
         */
        
        float alpha = 0.1;
        if (x_ == -1 && y_ == -1) {
            x_ = x;
            y_ = y;
        }
        else {
            float t_x = alpha*x + (1-alpha)*x_;
            float t_y = alpha*y + (1-alpha)*y_;

            float d = pow(t_x - x_, 2) + pow(t_y - y_, 2);
            d = sqrt(d);

            if (d > thresh) {
                float b_x = t_x - x_;
                float b_y = t_y - y_;

                float a_x = 0.6f * diff_0.x + 0.3f * diff_1.x + 0.1f * diff_2.x;
                float a_y = 0.6f * diff_0.y + 0.3f * diff_1.y + 0.1f * diff_2.y;

                float norm_b = sqrt(pow(b_x, 2) + pow(b_y, 2));

                float b_n_x = b_x/norm_b;
                float b_n_y = b_y/norm_b;
                
                t_x = x_ + 0.8f * a_x + 0.2f * b_n_x;
                t_y = y_ + 0.8f * a_y + 0.2f * b_n_y;
            }

            diff_2 = diff_1;
            diff_1 = diff_0;

            diff_0.x = t_x - x_;
            diff_0.y = t_y - y_;

            x_ = t_x;
            y_ = t_y;
        }

        point recent = {x_, y_};

        q_mutex.lock();

        data.push(recent);

        q_mutex.unlock();
    }

}

point3 transform(point in) {
    float a = 40.0f;
    float b = 30.0f;

    float t = in.x*in.x/(a*a) + in.y*in.y/(b*b) + 1; 
    t = sqrt(1/t);

    point3 rval = {in.x * t, in.y * t, -40 * t};

    return rval;
}

/*
point transform(point in) {
    float a = 30;
    float b = 40;

    float t = in.x*in.x/(a*a) + 1;
    t = sqrt(1/t);

    point rval = {in.x * t, in.y * t};

    return rval;
}*/

int main() {

    String face_cascade = "../config/haarcascade_frontalface_default.xml";
    CascadeClassifier face;
    if(!face.load(face_cascade)) return -1;
    //KeyReceiver receiver;
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600), 32, false);

    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();
    gui::IGUIEnvironment *guienv = device->getGUIEnvironment();
    ILogger *logger = device->getLogger();

    guienv->addStaticText(L"This is a test", core::rect<s32>(10,10, 260, 22));

    scene::ISceneNode* skybox=scenemgr->addSkyBoxSceneNode(
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_up.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_dn.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_lf.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_rt.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_ft.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_bk.jpg"));

    //scene::IAnimatedMesh *mesh = scenemgr->getMesh("../data/models/IPhone/IPhone 4Gs _5.obj");
    scene::IAnimatedMesh *mesh = scenemgr->getMesh("../data/models/Box/Box.obj");
    scene::ISceneNode *node = scenemgr->addAnimatedMeshSceneNode(mesh);
    //scene::ISceneNode *dummy = scenemgr->addEmptySceneNode();
    scene::ILightSceneNode *light = scenemgr->addLightSceneNode();

    //KeyReceiver receiver;

    if(node) {
        node->setPosition(core::vector3df(0, 0, 0));
        node->setRotation(core::vector3df(0, 0, 0));
        node->setScale(core::vector3df(0.1, 0.1, 0.1));

        //dummy->setPosition(core::vector3df(1.7046/2, 0, 3.2837/2));
        //node->setParent(dummy);

        //dummy->setPosition(core::vector3df(-1, 1, 0));
        //char buf[33];
        //snprintf(buf, 33, "%f,%f,%f", node->getPosition().X, node->getPosition().Y, node->getPosition().Z);
        //logger->log(buf);
        //scene::IMeshManipulator *mmp = driver->getMeshManipulator();
        //mmp->flipSurfaces(mesh);
        light->setPosition(core::vector3df(0, 0, -50));
    }
    
    /**
     * Because of normals and lighting issue, place the camera at -z, and work out the parabola.
     */
    //scene::ICameraSceneNode *cam = scenemgr->addCameraSceneNodeFPS(0, 10.f, 0.2f);//, core::vector3df(0, 0, -15));//, core::vector3df(0, 0, 0));
    //cam->setPosition(core::vector3df(0, 0, -100));
    scene::ICameraSceneNode *cam = scenemgr->addCameraSceneNode(0, core::vector3df(0, 0, -40), core::vector3df(0, 0, 0));

    /*Mat frame;
    Mat grey;
    vector<Rect> faces;*/

    int lastFPS = -1;
//    bool quit = false;

    thread collector(fetchData);
    /*float x_0 = -23.094f;
    float delta = 0.2f;
    float x_n = -x_0;
    float y_0 = -14.58f;
    float delta = 0.1f;
    float y_n = -y_0;*/

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
        point my_point;
	
        q_mutex.lock();

        if(!data.empty()) {
            my_point = data.front();
            data.pop();

            char buf[43];
            snprintf(buf, 43, "Actual Data: %f, %f", my_point.x, my_point.y);
            logger->log(buf);

            point3 new_point = transform(my_point);

            char bufi[50];
            snprintf(bufi, 50, "Transformed Data: %f, %f, %f", new_point.x, new_point.y, new_point.z);
            logger->log(bufi);

            cam->setPosition(core::vector3df(new_point.x, new_point.y, new_point.z));
            cam->setTarget(node->getPosition());
        }
        q_mutex.unlock();
/*
        if (y_0 > y_n) break;

        y_0 += delta;
        point curr = {y_0, -40};
        point new_p = transform(curr);

        cam->setPosition(core::vector3df(0, new_p.x, new_p.y));

        char buf[33];
        snprintf(buf, 33, "%f,%f,%f", cam->getPosition().X, cam->getPosition().Y, cam->getPosition().Z);
        logger->log(buf);
        cam->setTarget(node->getPosition());

        //if(receiver.IsKeyDown(KEY_ESCAPE)) quit = true;

        core::vector3df nodePosition = cam->getPosition();

        if(receiver.IsKeyDown(KEY_KEY_W))
            nodePosition.Y += 0.1;
        else if(receiver.IsKeyDown(KEY_KEY_S))
            nodePosition.Y -= 0.1;

        if(receiver.IsKeyDown(KEY_KEY_A))
            nodePosition.X += 0.1;
        else if(receiver.IsKeyDown(KEY_KEY_D))
            nodePosition.X += 0.1;

        if(receiver.IsKeyDown(KEY_KEY_P))
            nodePosition.Z += 0.1;
        else if(receiver.IsKeyDown(KEY_KEY_L))
            nodePosition.Z += 0.1;

        cam->setPosition(nodePosition);

        if(receiver.IsKeyDown(KEY_KEY_U))
            cam->setTarget(dummy->getPosition());
*/
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
