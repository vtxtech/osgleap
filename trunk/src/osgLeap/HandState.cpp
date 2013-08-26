/*
 * Library osgLeap
 * Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
 *
 * This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
 * but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <osgLeap/HandState>

//-- Project --//
#include <osgLeap/Controller>

//-- OSG: osgDB --//
#include <osgDB/ReadFile>

namespace osgLeap {

	// UpdateCallback "auto-updates" the osgLeap::HandState Geode from within
	// the update traversal of the osgViewer
	class UpdateCallback: public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{
			HandState* hs = dynamic_cast<HandState*>(node);
			if (hs) {
				hs->update();
			}
			traverse(node, nv);
		}
	};

	void HandState::createHandQuad(WhichHand hand)
	{
		// Just create a textured quad here...
		// LEFT_HAND and RIGHT_HAND differs in
		// - vertex coordinates,
		// - texture coordinates and
		// - the texture itself.

		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
		osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array();
		geom->setVertexArray(va);
		osg::ref_ptr<osg::Vec3Array> na = new osg::Vec3Array();
		na->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		geom->setNormalArray(na);
		geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
		osg::Vec4Array* colors = new osg::Vec4Array();
		colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

		osg::ref_ptr<osg::Vec2Array> texCoords = new osg::Vec2Array();

		if (hand == LEFT_HAND) {
			va->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 160.0f, 0.0f));
			va->push_back(osg::Vec3(0.0f, 160.0f, 0.0f));

			texCoords->push_back(osg::Vec2(1,0));
			texCoords->push_back(osg::Vec2(0,0));
			texCoords->push_back(osg::Vec2(0,1));
			texCoords->push_back(osg::Vec2(1,1));

			geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, lhTex_, osg::StateAttribute::ON);
		} else {
			va->push_back(osg::Vec3(128.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(256.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(256.0f, 160.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 160.0f, 0.0f));

			texCoords->push_back(osg::Vec2(0,0));
			texCoords->push_back(osg::Vec2(1,0));
			texCoords->push_back(osg::Vec2(1,1));
			texCoords->push_back(osg::Vec2(0,1));

			geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, rhTex_, osg::StateAttribute::ON);
		}

		geom->setTexCoordArray(0, texCoords);
		geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		addDrawable(geom);

	}

	HandState::HandState(): osg::Geode(), Leap::Listener(),
		frame_(Leap::Frame()),
		lhTex_(new osg::Texture2D()),
		rhTex_(new osg::Texture2D())
	{
		// Initialize UpdateCallback to update myself during updateTraversal
		addUpdateCallback(new UpdateCallback());

		osgLeap::Controller::instance()->addListener(*this);

		// Load textures for hands visualization.
		// Note that all hands images must be either in the same
		// directory as the executable, or in the current working
		// directory or in a path that is defined in the OSG_FILE_PATH
		// system enviroment variable
		handsTextures_.push_back(osgDB::readImageFile("nohand.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand0.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand1.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand2.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand3.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand4.png"));
		handsTextures_.push_back(osgDB::readImageFile("hand5.png"));

		// Prescale images to square resolution so we avoid doing that
		// during update
		handsTextures_.at(0)->scaleImage(1024, 1024, 1);
		handsTextures_.at(1)->scaleImage(1024, 1024, 1);
		handsTextures_.at(2)->scaleImage(1024, 1024, 1);
		handsTextures_.at(3)->scaleImage(1024, 1024, 1);
		handsTextures_.at(4)->scaleImage(1024, 1024, 1);
		handsTextures_.at(5)->scaleImage(1024, 1024, 1);
		handsTextures_.at(6)->scaleImage(1024, 1024, 1);

		// Set DataVariance to DYNAMIC to avoid the texture changes being
		// optimized away.
		lhTex_->setDataVariance(osg::Object::DYNAMIC);
		lhTex_->setImage(handsTextures_.at(0));
		rhTex_->setDataVariance(osg::Object::DYNAMIC);
		rhTex_->setImage(handsTextures_.at(0));

		// Now finally, create the QUAD geometry to put our texture onto
		// Note that this method should be called once per hand, only.
		createHandQuad(LEFT_HAND);
		createHandQuad(RIGHT_HAND);
	}

	HandState::~HandState()
	{
		osgLeap::Controller::instance()->removeListener(*this);
	}

	HandState::HandState(const HandState& hs,
		const osg::CopyOp& copyOp): osg::Geode(*this), Leap::Listener(*this),
		frame_(Leap::Frame())
	{
		// ToDo: Copy texture2d member variables (lhTex_, rhTex_) correctly...
	}

	void HandState::onFrame(const Leap::Controller& controller)
	{
		// Get the most recent frame and store it for later use in update(...)
		frame_ = controller.frame();
	}

	void HandState::update()
	{
		// Grab the frame to work on ...
		Leap::Frame frame = frame_;

		// Setup "no-hand" image as default
		osg::Image* lh = handsTextures_.at(0);
		osg::Image* rh = handsTextures_.at(0);

		// Continue if there it at least one hand, only.
		if (frame.hands().count() > 0) {
			// Using leftmost and rightmost hands
			Leap::Hand left = frame.hands().leftmost();
			Leap::Hand right = frame.hands().rightmost();
			// Count the fingers we have detected...
			int r_fingers = right.fingers().count()+1;
			int l_fingers = left.fingers().count()+1;
			// Avoid crash if textures were not loaded
			// or if we have more than 5 fingers per hand ;-)
			if (r_fingers > handsTextures_.size()) {
				OSG_WARN<<"WARN: Not enough images ("<<handsTextures_.size()<<") for right hand finger count ("<<r_fingers-1<<"), aborting HandState::update."<<std::endl;
				return;
			}
			if (l_fingers > handsTextures_.size()) {
				OSG_WARN<<"WARN: Not enough images ("<<handsTextures_.size()<<") for left hand finger count ("<<l_fingers-1<<"), aborting HandState::update."<<std::endl;
				return;
			}
			// Compare hands IDs to determine if leftmost hand and rightmost
			// hand are the same
			if (left.id() == right.id()) {
				// Assume right hand if we have one hand, only.
				// (As we cannot distinguish between the actual right and left
				// hand. We operate on "leftmost" and "rightmost" hands only.)
				rh = handsTextures_.at(r_fingers);
			} else {
				rh = handsTextures_.at(r_fingers);
				lh = handsTextures_.at(l_fingers);
			}
		}

		lhTex_->setImage(lh);
		rhTex_->setImage(rh);
	}

}
