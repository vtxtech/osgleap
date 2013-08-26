/*
* Example leaporbit
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgLeap/OrbitManipulator>
#include <osgLeap/HandState>


#include <osg/io_utils>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/ValueObject>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgLeap/IntersectionController>
#include <osgLeap/HandState>
#include <osgLeap/HUDCamera>

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an example showing osgLeap::OrbitManipulator use.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--twohanded", "Initialize the OrbitManipulator in two-handed mode. PAN: One hand, ZOOM: Left hand closed+Right hand open, ROTATE: Both hands open. Move right hand for rotation (default).");
    arguments.getApplicationUsage()->addCommandLineOption("--singlehanded", "Initialize the OrbitManipulator in simple one-handed mode (rotate+zoom) without panning.");
    arguments.getApplicationUsage()->addCommandLineOption("--trackball", "Initialize the OrbitManipulator in trackball one-handed mode. Imagine to hold a basketball in your hand palm down (pan+rotate+zoom).");

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

    osgLeap::OrbitManipulator::Mode mode = osgLeap::OrbitManipulator::TwoHanded;
    while (arguments.read("--twohanded")) {
        mode = osgLeap::OrbitManipulator::TwoHanded;
    }
    while (arguments.read("--singlehanded")) {
        mode = osgLeap::OrbitManipulator::SingleHanded;
    }
    while (arguments.read("--trackball")) {
        mode = osgLeap::OrbitManipulator::Trackball;
    }

    viewer.setCameraManipulator( new osgLeap::OrbitManipulator(mode) );

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

    viewer.addSlave(hudCamera, false);

    return viewer.run();

}
