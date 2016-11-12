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
bool quit;

/**
 * Took this snippet from Irrlicht Documentation on KeyEvent Handling
 */
class KeyReceiver : public IEventReceiver 
{
    public:
        virtual bool OnEvent(const SEvent& event) {
            if (event.EventType == EET_KEY_INPUT_EVENT)
                KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
            
            return false;
        }

        virtual bool IsKeyDown(EKEY_CODE keyCode) const {
            return KeyIsDown[keyCode];
        }

        KeyReceiver() {
            for (unsigned int i=0; i < KEY_KEY_CODES_COUNT; ++i)
                KeyIsDown[i] = false;
        }

    private:
        bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

/**
 * Basic structure of the face detector was taken from OpenCV docs and we included
 * our parts in the necessary areas.
 */
void fetchData() { 
    VideoCapture cap(0);
    if (!cap.isOpened()) return;
   
    /**
     * These values were determined by experimentation.
     */
//    float scale = .0951447f;
    float scale = .1451447f;
    float thresh = .2366f;

    String face_cascade = "../config/haarcascade_frontalface_default.xml";
    CascadeClassifier face;
    if(!face.load(face_cascade)) return;

    Mat frame;
    Mat grey;
    vector<Rect> faces;

    float x, y, x_ = -1, y_ = -1;

    while(!quit) {

        cap >> frame;
        if(frame.empty()) {
            //Or interpolate some values here.
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
    float a = 50.0f;
    float b = 40.0f;

    float t = in.x*in.x/(a*a) + in.y*in.y/(b*b) + 1; 
    t = sqrt(1/t);

    point3 rval = {in.x * t, in.y * t, -40 * t};

    return rval;
}

/**
 * The basic game loop was taken from Irrlicht docs and we included the required
 * stuff by ourselves
 */
int main() {

    String face_cascade = "../config/haarcascade_frontalface_default.xml";
    CascadeClassifier face;
    if(!face.load(face_cascade)) return -1;

    KeyReceiver rx;

    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(1920, 1080), 32, true, false, false, &rx);

    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *scenemgr = device->getSceneManager();
    gui::IGUIEnvironment *guienv = device->getGUIEnvironment();
    ILogger *logger = device->getLogger();

    guienv->addStaticText(L"This is a test", core::rect<s32>(10,10, 260, 22));

    scene::ISceneNode *skybox = scenemgr->addSkyBoxSceneNode(
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_up.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_dn.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_lf.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_rt.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_ft.jpg"),
            driver->getTexture("../data/skybox/irrlicht/irrlicht2_bk.jpg"));
    skybox->setVisible(true);
    
    /*skybox[1] = scenemgr->addSkyBoxSceneNode(
            driver->getTexture("../data/skybox/hw_morning/morning_up.tga"),
            driver->getTexture("../data/skybox/hw_morning/morning_dn.tga"),
            driver->getTexture("../data/skybox/hw_morning/morning_lf.tga"),
            driver->getTexture("../data/skybox/hw_morning/morning_rt.tga"),
            driver->getTexture("../data/skybox/hw_morning/morning_ft.tga"),
            driver->getTexture("../data/skybox/hw_morning/morning_bk.tga"));
    skybox[1]->setVisible(false);

    skybox[2] = scenemgr->addSkyBoxSceneNode(
            driver->getTexture("../data/skybox/ame_siege/siege_up.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_dn.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_lf.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_rt.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_ft.tga"),
            driver->getTexture("../data/skybox/ame_siege/siege_bk.tga"));
    skybox[2]->setVisible(false);*/
    
    scene::IAnimatedMesh *mesh = scenemgr->getMesh("../data/models/NewCube/NewCube.obj");
    scene::ISceneNode *node = scenemgr->addAnimatedMeshSceneNode(mesh);
    scene::ILightSceneNode *light = scenemgr->addLightSceneNode();


    if(node) {
        node->setPosition(core::vector3df(0, 0, 50));
        node->setRotation(core::vector3df(0, 0, 0));
        node->setScale(core::vector3df(13, 13, 13));

        light->setPosition(core::vector3df(0, 0, -40));
    }
    
    /**
     * Because of normals and lighting issue, place the camera at -z, and work out the parabola.
     */
    scene::ICameraSceneNode *cam = scenemgr->addCameraSceneNode(0, core::vector3df(0, 0, -40), core::vector3df(0, 0, 0));


    int lastFPS = -1;
    /*int currSkyBox = 0;
    unsigned int then = device->getTimer()->getTime();
    unsigned int thres = 2000;
    unsigned int counter = 0;*/

    thread collector(fetchData);

    while(device->run() && driver && !quit) {
        // Work out a frame delta time.
        /*const unsigned int now = device->getTimer()->getTime();
        const float frameDeltaTime = (float) (now - then) / 1000.f; // Time in seconds
        then = now;*/

        driver->beginScene(true, true, video::SColor(255, 100, 101, 140));
        scenemgr->drawAll();
        guienv->drawAll();
        /**
         * Get the coordinates of the face.
         * Optional : write code such that the first face in the image is chosen
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

        if(rx.IsKeyDown(KEY_KEY_X)) quit = true;
        /*if(rx.IsKeyDown(KEY_KEY_O)) {

            for(int i = 0; i < 3; i++) {
                skybox[i]->setVisible(false);
            }

            if (++currSkyBox > 2) currSkyBox == 0;
            skybox[currSkyBox]->setVisible(true);
        
        }*/

        driver->endScene();

        int fps = driver->getFPS();

        if (lastFPS != fps) {
            core::stringw tmp(L"Virtual Reality - Irrlicht Engine [");
            tmp += driver->getName();
            tmp += L"] fps: ";
            tmp += fps;

            device->setWindowCaption(tmp.c_str());
            lastFPS = fps;
        }
    }

    collector.join();

    driver->drop();

    return 0;

}
