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



    // add to this camera a subgraph to render
    {

        osg::Geode* geode = new osg::Geode();

        std::string timesFont("fonts/arial.ttf");

        // turn lighting off for the text and disable depth test to ensure it's always ontop.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::Vec3 position(150.0f,800.0f,0.0f);
        osg::Vec3 delta(0.0f,-120.0f,0.0f);

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Head Up Displays are simple :-)");

            position += delta;
        }

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("Start your Leap Motion pointing at the screen!");

            position += delta;
        }


        camera->addChild(geode);

		camera->addChild(new osgLeap::HandState());
    }

    return camera;
}

class PointerUpdateCallback: public osg::NodeCallback
{
public:
	typedef std::map<int, osg::ref_ptr<osg::PositionAttitudeTransform> > PatMap;
	typedef std::pair<int, osg::ref_ptr<osg::PositionAttitudeTransform> > PatPair;

	PointerUpdateCallback(osg::Camera* camera): camera_(camera),
		intersectionController_(new osgLeap::IntersectionController()), colorIndex_(0)
	{
		
	}

	osg::ref_ptr<osg::Node> createPointerGeode() {
		osg::ref_ptr<osg::Geode> sphere = new osg::Geode();
		float radius = 10.0f;
		osg::TessellationHints* hints = new osg::TessellationHints();
		hints->setDetailRatio(0.5f);
		osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius),hints);
		sd->setColor(getColor());
		sphere->addDrawable(sd);
		return sphere;
	}

	osg::Vec4 getColor() {
		osg::Vec4 result;

		switch (colorIndex_) {
		case 0:
			result = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 1:
			result = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 2:
			result = osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f);
			break;
		case 3:
			result = osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 4:
			result = osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 5:
			result = osg::Vec4(1.0f, 0.0f, 1.0f, 1.0f);
			break;
		case 6:
			result = osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f);
			break;
		case 7:
			result = osg::Vec4(0.5f, 0.1f, 0.0f, 1.0f);
			break;
		case 8:
			result = osg::Vec4(1.0f, 0.5f, 0.5f, 1.0f);
			break;
		case 9:
			result = osg::Vec4(1.0f, 1.0f, 0.5f, 1.0f);
			break;
		default:
			result = osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		}

		colorIndex_++;
		if (colorIndex_ > 9) { colorIndex_ = 0; }

		return result;
	}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		// Grab data from Leap Motion
		intersectionController_->update();

		float screenheight = camera_->getGraphicsContext()->getTraits()->height;
		float screenwidth  = camera_->getGraphicsContext()->getTraits()->width;

		// Now update our Geode to display the pointers
		osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(node);
		if (group.valid()) {
			osgLeap::PointerMap pointers = intersectionController_->getPointers();

			PatMap transforms;
			// Remove any pointers not visible anymore
			for (int n = group->getNumChildren()-1; n >= 0; --n) {
				int pid = -1;
				bool remove = true;
				if (group->getChild(n)->getUserValue<int>("PointableID", pid)) {
					for (osgLeap::PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
						// Still visible in current frame
						if (itr->first == pid) remove = false;
					}
				}
				if (remove) {
					group->removeChild(n);
				} else {
					transforms.insert(PatPair(pid, dynamic_cast<osg::PositionAttitudeTransform*>(group->getChild(n))));
				}
			}

			// Add more pointers if required or update if they have moved
			for (osgLeap::PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
				osg::ref_ptr<osgLeap::Pointer> p = itr->second;

				PatMap::iterator patitr = transforms.find(itr->first);
				osg::ref_ptr<osg::PositionAttitudeTransform> pat = NULL;
				if (patitr != transforms.end()) {
					pat = patitr->second;
				} else {
					// Add new pointer
					pat = new osg::PositionAttitudeTransform();
					pat->setUserValue<int>("PointableID", itr->first);
					pat->addChild(createPointerGeode());
					// Add to scene graph
					group->addChild(pat);
					// Add to local reference map
					transforms.insert(PatPair(itr->first, pat));
				}

				// Calculate pixel screen position from relative Leap values [X: 0.0 to 1.0, Y: 0.0 to 1.0]
				// using the 3D window resolution. Z is always zero.
				osg::Vec3 vec = osg::Vec3(p->getPosition().x()*screenwidth, p->getPosition().y()*screenheight, 0.0f);
				pat->setPosition(vec);

				// ToDo: Handle camera resize in a better way
				camera_->setProjectionMatrix(osg::Matrix::ortho2D(0.0f, screenwidth, 0.0f, screenheight));
			}
		}

        traverse(node, nv);
    }

private:
	osg::ref_ptr<osgLeap::IntersectionController> intersectionController_;
	osg::ref_ptr<osg::Camera> camera_;
	int colorIndex_;
};

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is an example showing osgLeap::IntersectionController use.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    //arguments.getApplicationUsage()->addCommandLineOption("--twohanded", "Initialize the OrbitManipulator in two-handed mode. PAN: One hand, ZOOM: Left hand closed+Right hand open, ROTATE: Both hands open. Move right hand for rotation (default).");

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
	
	// ToDo: Remove this...
	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	//osgLeap::OrbitManipulator::Mode mode = osgLeap::OrbitManipulator::TwoHanded;
	//while (arguments.read("--twohanded")) {
	//	mode = osgLeap::OrbitManipulator::TwoHanded;
	//}
	//while (arguments.read("--singlehanded")) {
	//	mode = osgLeap::OrbitManipulator::SingleHanded;
	//}
	//while (arguments.read("--trackball")) {
	//	mode = osgLeap::OrbitManipulator::Trackball;
	//}

    //viewer.setCameraManipulator( new osgLeap::OrbitManipulator(mode) );

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

	osg::ref_ptr<osg::Group> pointersGroup = new osg::Group();

	osg::ref_ptr<PointerUpdateCallback> puc = new PointerUpdateCallback(hudCamera);
	pointersGroup->addUpdateCallback(puc);
	//pointersGroup->addCullCallback(puc);
	hudCamera->addChild(pointersGroup);

    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);

    if (windows.empty()) return 1;

	// set up cameras to render on the first window available.
    hudCamera->setGraphicsContext(windows[0]);
    hudCamera->setViewport(0, 0, windows[0]->getTraits()->width, windows[0]->getTraits()->height);

    viewer.addSlave(hudCamera, false);

    return viewer.run();

}
