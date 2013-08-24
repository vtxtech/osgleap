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

	HandState::HandState(): osg::Geode(), Leap::Listener(),
		frame_(Leap::Frame())
	{
		addUpdateCallback(new UpdateCallback());

		osgLeap::Controller::instance()->addListener(*this);
		texLeftHand_.push_back(osgDB::readImageFile("nolefthand.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand0s.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand1s.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand2s.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand3s.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand4s.png"));
		texLeftHand_.push_back(osgDB::readImageFile("lefthand5s.png"));

		texRightHand_.push_back(osgDB::readImageFile("norighthand.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand0s.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand1s.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand2s.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand3s.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand4s.png"));
		texRightHand_.push_back(osgDB::readImageFile("righthand5s.png"));

		texLeftHand_.at(0)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(1)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(2)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(3)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(4)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(5)->scaleImage(1024, 1024, 1);
		texLeftHand_.at(6)->scaleImage(1024, 1024, 1);

		texRightHand_.at(0)->scaleImage(1024, 1024, 1);
		texRightHand_.at(1)->scaleImage(1024, 1024, 1);
		texRightHand_.at(2)->scaleImage(1024, 1024, 1);
		texRightHand_.at(3)->scaleImage(1024, 1024, 1);
		texRightHand_.at(4)->scaleImage(1024, 1024, 1);
		texRightHand_.at(5)->scaleImage(1024, 1024, 1);
		texRightHand_.at(6)->scaleImage(1024, 1024, 1);

		{
			osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
			osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array();
			va->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 160.0f, 0.0f));
			va->push_back(osg::Vec3(0.0f, 160.0f, 0.0f));
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

			osg::Vec2 myTexCoords[] =
			{
				osg::Vec2(0,0),
				osg::Vec2(1,0),
				osg::Vec2(1,1),
				osg::Vec2(0,1)
			};

			int numTexCoords = sizeof(myTexCoords)/sizeof(osg::Vec2);

			// pass the created tex coord array to the points geometry object,
			// and use it to set texture unit 0.
			geom->setTexCoordArray(0,new osg::Vec2Array(numTexCoords,myTexCoords));

			lhTex_ = new osg::Texture2D();
			lhTex_->setDataVariance(osg::Object::DYNAMIC);
			lhTex_->setImage(texLeftHand_.at(0));
			geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, lhTex_, osg::StateAttribute::ON);
			geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

			addDrawable(geom);
		}

		{
			osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
			osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array();
			va->push_back(osg::Vec3(128.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(256.0f, 0.0f, 0.0f));
			va->push_back(osg::Vec3(256.0f, 160.0f, 0.0f));
			va->push_back(osg::Vec3(128.0f, 160.0f, 0.0f));
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

			osg::Vec2 myTexCoords[] =
			{
				osg::Vec2(0,0),
				osg::Vec2(1,0),
				osg::Vec2(1,1),
				osg::Vec2(0,1)
			};

			int numTexCoords = sizeof(myTexCoords)/sizeof(osg::Vec2);

			// pass the created tex coord array to the points geometry object,
			// and use it to set texture unit 0.
			geom->setTexCoordArray(0,new osg::Vec2Array(numTexCoords, myTexCoords));

			rhTex_ = new osg::Texture2D();
			rhTex_->setDataVariance(osg::Object::DYNAMIC);
			rhTex_->setImage(texRightHand_.at(0));
			geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, rhTex_, osg::StateAttribute::ON);
			geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

			addDrawable(geom);
		}
	}

	HandState::~HandState()
	{
		osgLeap::Controller::instance()->removeListener(*this);
	}

	HandState::HandState(const HandState& hs,
		const osg::CopyOp& copyOp): osg::Geode(*this), Leap::Listener(*this),
		frame_(Leap::Frame())
	{

	}

	void HandState::onFrame(const Leap::Controller& controller)
	{
		// Get the most recent frame and store it to later use in update(...)
		frame_ = controller.frame();
	}

	void HandState::update()
	{
		Leap::Frame frame = frame_;

		osg::Image* lh = texLeftHand_.at(0);
		osg::Image* rh = texRightHand_.at(0);

		if (frame.hands().count() > 0) {
			Leap::Hand left = frame.hands().leftmost();
			Leap::Hand right = frame.hands().rightmost();
			if (left.id() == right.id()) {
				rh = texRightHand_.at(right.fingers().count()+1);
			} else {
				rh = texRightHand_.at(right.fingers().count()+1);
				lh = texLeftHand_.at(left.fingers().count()+1);
			}
		}

		lhTex_->setImage(lh);
		rhTex_->setImage(rh);
	}

}
