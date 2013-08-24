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

osg::Camera* createHUD()
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::Camera* camera = new osg::Camera;

    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);

	camera->addChild(new osgLeap::HandState());

    return camera;
}

class ResizeUpdateCallback: public osg::NodeCallback
{
public:
	ResizeUpdateCallback(osg::Camera* camera): camera_(camera)
	{
		
	}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		// ToDo: Handle camera resize in a better way
		float screenheight = camera_->getGraphicsContext()->getTraits()->height;
		float screenwidth  = camera_->getGraphicsContext()->getTraits()->width;

		camera_->setProjectionMatrix(osg::Matrix::ortho2D(0.0f, screenwidth, 0.0f, screenheight));

        traverse(node, nv);
    }

private:
	osg::ref_ptr<osg::Camera> camera_;
};


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

	osg::Camera* hudCamera = createHUD();
	hudCamera->addUpdateCallback(new ResizeUpdateCallback(hudCamera));

	// set up cameras to render on the first window available.
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);

    if (windows.empty()) return 1;

    hudCamera->setGraphicsContext(windows[0]);
    hudCamera->setViewport(0, 0, windows[0]->getTraits()->width, windows[0]->getTraits()->height);

    viewer.addSlave(hudCamera, false);

    return viewer.run();

}
