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

    osgViewer::Viewer viewer(arguments);

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

    viewer.setSceneData( loadedModel.get() );

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

    viewer.addSlave(hudCamera, false);

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
