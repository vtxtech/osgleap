/*
 * Library osgLeap
 * Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
 *
 * This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
 * but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

//-- Project --//
#include <osgLeap/LeapManipulator>

//-- OSG: osg --//
#include <osg/Referenced>
#include <osg/Timer>

//-- Leap --//
#include <LeapMath.h>

namespace osgLeap {

	Controller* Controller::instance(bool erase)
	{
		static osg::ref_ptr<Controller> instance_ = new Controller;

		OSG_DEBUG<<"Accessed: osgLeap::Controller="<<instance_<<std::endl;

		if (erase) 
		{   
			instance_ = NULL;
		}
		return instance_.get(); // will return NULL on erase
	}

	LeapManipulator::LeapManipulator(): osgGA::OrbitManipulator(), Leap::Listener()
	{
		osgLeap::Controller::instance()->addListener(*this);
	}

	LeapManipulator::~LeapManipulator()
	{
		osgLeap::Controller::instance()->removeListener(*this);
	}

	LeapManipulator::LeapManipulator(const LeapManipulator& lm,
		const osg::CopyOp& copyOp): OrbitManipulator(lm, copyOp), Leap::Listener(*this)
	{

	}

	bool LeapManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		sceneRadius_ = us.asView()->getCamera()->getBound().radius();

		return osgGA::OrbitManipulator::handle(ea, us);
	}

	void LeapManipulator::onFrame(const Leap::Controller& controller) {
		// Get the most recent frame and report some basic information
		const Leap::Frame frame = controller.frame();
		osg::Timer_t currentFrameStamp = osg::Timer::instance()->tick();
		osg::Timer_t deltaTimeSinceLastFrame = currentFrameStamp - lastFrameStamp_;

		OSG_DEBUG_FP << "Frame id: " << frame.id()
			<< ", timestamp: " << frame.timestamp()
			<< ", hands: " << frame.hands().count()
			<< ", fingers: " << frame.fingers().count()
			<< ", tools: " << frame.tools().count()
			<< ", gestures: " << frame.gestures().count() << std::endl;

		if (!frame.hands().empty()) {
			Leap::Hand handRight;
			Leap::Hand handLeft;
			if (leftHandID_ == -1 || rightHandID_ == -1 || (leftHandID_ == rightHandID_ && frame.hands().count() > 1)) {
				// Get the hands
				handRight = frame.hands().rightmost();
				handLeft = frame.hands().leftmost();
				rightHandID_ = handRight.id();
				leftHandID_ = handLeft.id();
			} else {
				handRight = frame.hand(rightHandID_);
				handLeft = frame.hand(leftHandID_);
				if (!handRight.isValid() || !handLeft.isValid()) {
					// Get the hands
					handRight = frame.hands().rightmost();
					handLeft = frame.hands().leftmost();
					rightHandID_ = handRight.id();
					leftHandID_ = handLeft.id();
				}
			}

			if (frame.hands().count() == 1 && handRight.fingers().count() >= 3) {
				if ((currentAction_ != LM_Pan)) {
					lastPositionRightHand_ = handRight.stabilizedPalmPosition();
				}
				currentAction_ = LM_Pan;
			} else if (frame.hands().count() > 1 && handLeft.fingers().count() >= 3 && handRight.fingers().count() >=3) {
				currentAction_ = LM_Rotate;
			} else if (frame.hands().count() > 1 && 
				((handLeft.fingers().count() >= 3 && handRight.fingers().count() <= 1) || (handLeft.fingers().count() <= 1 && handRight.fingers().count() >= 3 ) ))
			{
				if ((currentAction_ != LM_Zoom)) {
					handsDistance_ = (handLeft.stabilizedPalmPosition() - handRight.stabilizedPalmPosition()).magnitude();
				}
				currentAction_ = LM_Zoom;
			} else {
				currentAction_ = LM_None;
			}

			OSG_DEBUG<<"MODE: ";
			if (currentAction_ & LM_Rotate) OSG_DEBUG<<"ROTATE ";
			if (currentAction_ & LM_Zoom) OSG_DEBUG<<"ZOOM ";
			if (currentAction_ & LM_Pan) OSG_DEBUG<<"PAN ";
			if (currentAction_ == 0) OSG_DEBUG<<"NONE";
			OSG_DEBUG<<std::endl;

			osg::Vec3 trans;
			osg::Quat rot;
			osg::Vec3 scale;
			osg::Quat so;

			rot = getMatrix().getRotate();
			trans = getMatrix().getTrans();

			// Calculate delta position (movement)
			osg::Vec3 deltaPos = osg::Vec3(-(handRight.stabilizedPalmPosition().x-lastPositionRightHand_.x), -(handRight.stabilizedPalmPosition().y-lastPositionRightHand_.y), -(handRight.stabilizedPalmPosition().z-lastPositionRightHand_.z));
			if (currentAction_ & LM_Pan) {
				if (deltaPos.x() != 0.0f || deltaPos.y() != 0.0f || deltaPos.z() != 0.0f) { 
					// scale by model size to fit for very large and very small models
					double factor = 2*sceneRadius_;
					// scale translation units. leap tracking: mm, OSG units: m
					osg::Vec3 deltaTrans((deltaPos*factor/1000.0f));
					trans += rot*deltaTrans;
					panModel(deltaTrans.x(), deltaTrans.y());
				}
			}

			double distance = (handLeft.stabilizedPalmPosition() - handRight.stabilizedPalmPosition()).magnitude();
			if (handsDistance_ != 0.0f) {
				double reference_length = 100.0f;
				if (currentAction_ & LM_Zoom) {
					double factor = (handsDistance_-distance)/(reference_length);
					if (factor > 1.0f || factor < -1.0f) { factor = 1.0f; }
					//OSG_DEBUG_FP<<"zooming by "<<factor<<std::endl;
					setDistance(getDistance()*(1+factor));
				}
				if (currentAction_ & LM_Rotate) {
#if 0
					Leap::Vector movement = osg::PI_2*(handRight.stabilizedPalmPosition()-lastPositionRightHand_)/reference_length;
					osg::Quat addRotX(-movement.x, osg::Y_AXIS);
					osg::Quat addRotY(movement.y, osg::X_AXIS);
					osg::Quat addRotZ;//(movement.z, osg::Z_AXIS);//movement very strange
					manipulator_->setRotation(addRotX*addRotY*addRotZ*rot);
#else
					Leap::Vector movement = (handRight.stabilizedPalmPosition()-lastPositionRightHand_)/reference_length;
					Leap::Vector lastPosNorm = lastPositionRightHand_/reference_length;
					Leap::Vector curPosNorm = handRight.stabilizedPalmPosition()/reference_length;

					// At the moment, Fixed VerticalAxis is the only mode supported
					// because rotateTrackball is not yet working correctly.
					if( true /*manipulator_->getVerticalAxisFixed()*/ ) {
						OSG_DEBUG<<"FIXED VERTICAL"<<std::endl;
						rotateWithFixedVertical( movement.x, movement.y );
					} else {
						OSG_DEBUG<<"FLOATING VERTICAL"<<std::endl;
						rotateTrackball( lastPosNorm.x, lastPosNorm.y,
									curPosNorm.x, curPosNorm.y,
									getThrowScale( deltaTimeSinceLastFrame ) );
					}
#endif
				}
			}

			handsDistance_ = distance;

			lastPositionLeftHand_ = handLeft.stabilizedPalmPosition();
			lastPositionRightHand_ = handRight.stabilizedPalmPosition();

		} else {
			currentAction_ = LM_None;
		}

		lastFrame_ = frame;
		lastFrameStamp_ = currentFrameStamp;
	}
}
