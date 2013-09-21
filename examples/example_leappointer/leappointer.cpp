/*
* Example leappointer
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osg/io_utils>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/ValueObject>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Label>
#include <osgWidget/Box>
#include <osgWidget/WindowManager>
#include <osgWidget/ViewerEventHandlers>

#include <osgLeap/Controller>
#include <osgLeap/HandState>
#include <osgLeap/HUDCamera>
#include <osgLeap/PointerPositionListener>
#include <osgLeap/PointerEventDevice>
#include <osgLeap/PointerGraphicsUpdateCallback>

osg::ref_ptr<osg::Node> createText()
{
    osg::Geode* geode = new osg::Geode();

    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to ensure it's always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Vec3 position(150.0f,200.0f,0.0f);
    osg::Vec3 delta(0.0f,-120.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Start your Leap Motion pointing at the screen!");

        position += delta;
    }

    return geode;
}

const unsigned int MASK_2D = 0xF0000000;
const unsigned int MASK_3D = 0x0F000000;

struct ColorWidget: public osgWidget::Widget {
    ColorWidget():
    osgWidget::Widget("", 256.0f, 256.0f) {
        setEventMask(osgWidget::EVENT_ALL);
    }

    bool mouseEnter(double, double, const osgWidget::WindowManager*) {
        addColor(-osgWidget::Color(0.4f, 0.4f, 0.4f, 0.0f));
        
        // osgWidget::warn() << "enter: " << getColor() << std::endl;

        return true;
    }

    bool mouseLeave(double, double, const osgWidget::WindowManager*) {
        addColor(osgWidget::Color(0.4f, 0.4f, 0.4f, 0.0f));
        
        // osgWidget::warn() << "leave: " << getColor() << std::endl;
        
        return true;
    }

    bool mouseOver(double x, double y, const osgWidget::WindowManager*) {
        
        osgWidget::Color c = getImageColorAtPointerXY(x, y);

        if(c.a() < 0.001f) {
            // osgWidget::warn() << "Transparent Pixel: " << x << " " << y << std::endl;

            return false;
        }
        return true;
    }

    bool keyUp(int key, int keyMask, osgWidget::WindowManager*) {
        // osgWidget::warn() << "..." << key << " - " << keyMask << std::endl;

        return true;
    }
};

osgWidget::Box* createBox(const std::string& name, osgWidget::Box::BoxType bt) {
    osgWidget::Box*    box     = new osgWidget::Box(name, bt, true);
    osgWidget::Widget* widget1 = new osgWidget::Widget(name + "_widget1", 100.0f, 100.0f);
    osgWidget::Widget* widget2 = new osgWidget::Widget(name + "_widget2", 100.0f, 100.0f);
    osgWidget::Widget* widget3 = new ColorWidget();

    widget1->setColor(0.3f, 0.3f, 0.3f, 1.0f);
    widget2->setColor(0.6f, 0.6f, 0.6f, 1.0f);

    widget3->setImage("osgWidget/natascha.png");
    widget3->setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
    widget3->setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
    widget3->setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
    widget3->setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);

    box->addWidget(widget1);
    box->addWidget(widget2);
    box->addWidget(widget3);

    return box;
}

void setupWidgets(osgViewer::Viewer* viewer, osg::Group* root)
{   
    osg::ref_ptr<osgWidget::WindowManager> wm = new osgWidget::WindowManager(
        viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );
    
    wm->setPointerFocusMode(osgWidget::WindowManager::PFM_SLOPPY);

    osgWidget::Window* box1 = createBox("HBOX", osgWidget::Box::HORIZONTAL);
    osgWidget::Window* box2 = createBox("VBOX", osgWidget::Box::VERTICAL);
    osgWidget::Window* box3 = createBox("HBOX2", osgWidget::Box::HORIZONTAL);
    osgWidget::Window* box4 = createBox("VBOX2", osgWidget::Box::VERTICAL);

    box1->getBackground()->setColor(1.0f, 0.0f, 0.0f, 0.8f);
    box1->attachMoveCallback();

    box2->getBackground()->setColor(0.0f, 1.0f, 0.0f, 0.8f);
    box2->attachMoveCallback();

    box3->getBackground()->setColor(0.0f, 0.0f, 1.0f, 0.8f);
    box3->attachMoveCallback();

    wm->addChild(box1);
    wm->addChild(box2);
    wm->addChild(box3);
    wm->addChild(box4);

    box4->hide();

    osg::Camera* camera = wm->createParentOrthoCamera();

    root->addChild(camera);

    viewer->addEventHandler(new osgWidget::MouseHandler(wm));
    viewer->addEventHandler(new osgWidget::KeyboardHandler(wm));
    viewer->addEventHandler(new osgWidget::ResizeHandler(wm, camera));
    viewer->addEventHandler(new osgWidget::CameraSwitchHandler(wm, camera));
    viewer->addEventHandler(new osgViewer::StatsHandler());
    viewer->addEventHandler(new osgViewer::WindowSizeHandler());

    wm->resizeAllWindows();
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an example showing osgLeap::PointerPositionListener use.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--timebased", "Invoke mouse clicks after some time if the pointer is not moving (Default)");
    arguments.getApplicationUsage()->addCommandLineOption("--time <milliseconds>", "Invoke time-based mouse clicks after <milliseconds> (Default: 3000)");
    arguments.getApplicationUsage()->addCommandLineOption("--noclick", "Initialize osgLeap::PointerEventDevice without ability to send clicks");
    arguments.getApplicationUsage()->addCommandLineOption("--useintersection", "Invoke clicks above a valid geometry, only.");
    arguments.getApplicationUsage()->addCommandLineOption("--screentap", "Invoke mouse clicks upon the screen tap gesture");
    arguments.getApplicationUsage()->addCommandLineOption("--mouse", "While moving pointer send mouse motion events. Clicks are sent as mouse clicks.");
    arguments.getApplicationUsage()->addCommandLineOption("--touch", "While moving pointer send touch move events. Clicks are sent as touch taps.");

    osgViewer::Viewer viewer;
    osg::Group* root = new osg::Group();
    viewer.setSceneData(root);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // ToDo/j.scholz: Remove this...
    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    // Setup example osgWidgets
    setupWidgets(&viewer, root);

    // Defines the time that a pointer needs to stand still
    // before a mouse click is performed at the current position
    int clickEmulateStillStandTime = 3000;
    while (arguments.read("--time", clickEmulateStillStandTime)) {
        // Nothing else to be done.
    }

    osgLeap::PointerEventDevice::EmulationMode emulationMode = osgLeap::PointerEventDevice::MOUSE;
    while (arguments.read("--mouse")) {
        emulationMode = osgLeap::PointerEventDevice::MOUSE;
    }
    while (arguments.read("--touch")) {
    	emulationMode = osgLeap::PointerEventDevice::TOUCH;
    }

    osgLeap::PointerEventDevice::ClickMode clickMode = osgLeap::PointerEventDevice::TIMEBASED_MOUSECLICK;
    while (arguments.read("--noclick")) {
        clickMode = osgLeap::PointerEventDevice::NONE;
        clickEmulateStillStandTime = 0;
    }
    while (arguments.read("--timebased")) {
    	clickMode = osgLeap::PointerEventDevice::TIMEBASED_MOUSECLICK;
    }
    while (arguments.read("--screentap")) {
        clickMode = osgLeap::PointerEventDevice::SCREENTAP;
        osgLeap::Controller::instance()->enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
        clickEmulateStillStandTime = 0;
    }

    bool useIntersection = false;
    while (arguments.read("--useintersection")) {
        useIntersection = true;
    }

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    root->addChild(loadedModel.get());

    viewer.realize();

    // set up cameras to render on the first window available.
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);

    if (windows.empty()) return 1;

    osg::Camera* hudCamera = new osgLeap::HUDCamera(windows[0]);

    // Adds the osgLeap::HandState visualizer
    hudCamera->addChild(new osgLeap::HandState());

    // Add some text to the HUD
    hudCamera->addChild(createText());

    osg::ref_ptr<osg::Group> pointersGroup = new osg::Group();
    // PointerGraphicsUpdateCallback needs clickEmulateStillStandTime to visualize
    // the remaining time until the click is executed.
    osg::ref_ptr<osgLeap::PointerGraphicsUpdateCallback> puc = new osgLeap::PointerGraphicsUpdateCallback(hudCamera, clickEmulateStillStandTime);
    pointersGroup->addUpdateCallback(puc);
    hudCamera->addChild(pointersGroup);

    // Our PointerEventDevice is initialized to fire mouseclicks after clickEmulateStillStandTime is gone
    osg::ref_ptr<osgLeap::PointerEventDevice> dev = new osgLeap::PointerEventDevice(clickMode, emulationMode, clickEmulateStillStandTime, puc->getPointerPositionListener());
    if (useIntersection) {
        // Setup viewer and nodemask, so clicks will be sent only if we are hovering above some node
        dev->setView(&viewer);
        dev->setTraversalMask(0xffffffff);
    }
    viewer.addDevice(dev);

    viewer.addSlave(hudCamera, false);

    return viewer.run();

}
