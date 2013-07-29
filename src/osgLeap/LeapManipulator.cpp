/*
 * Library osgLeap
 * Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
 *
 * This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
 * but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <osgLeap/LeapManipulator>

#include <osg/Timer>

#include <Leap.h>
#include <LeapMath.h>

namespace osgLeap {

	class Listener: public Leap::Listener {
	public:
		Listener(LeapManipulator* manipulator):
			manipulator_(manipulator),
			currentAction_(LeapManipulator::LM_None),
			leftHandID_(-1),
			rightHandID_(-1),
			handsDistance_(0.0f),
			lastFrameStamp_(0.0f)
		{
			
		}

		virtual ~Listener() {
			// Nothing to be done.
		}

		virtual void onInit(const Leap::Controller&);
		virtual void onConnect(const Leap::Controller&);
		virtual void onDisconnect(const Leap::Controller&);
		virtual void onExit(const Leap::Controller&);
		virtual void onFrame(const Leap::Controller&);
		virtual void onFocusGained(const Leap::Controller&);
		virtual void onFocusLost(const Leap::Controller&);

	private:
		int32_t leftHandID_;
		int32_t rightHandID_;
		Leap::Frame lastFrame_;
		Leap::Vector lastPositionLeftHand_;
		Leap::Vector lastPositionRightHand_;
		double handsDistance_;

		osg::ref_ptr<LeapManipulator> manipulator_;
		int currentAction_;
		osg::Timer_t lastFrameStamp_;
	};

	void Listener::onInit(const Leap::Controller& controller)
	{
		OSG_DEBUG<<"Initialized"<<std::endl;
	}

	void Listener::onConnect(const Leap::Controller& controller)
	{
		OSG_DEBUG<<"Connected"<<std::endl;
		// Gestures are not used in this implementation, so we will not
		// activate them here. Note that all gestures you might want to
		// use must be activated before otherwise they are not calculated
		// by the Leap Motion driver.
		//controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
		//controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
		//controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
		//controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
	}

	void Listener::onDisconnect(const Leap::Controller& controller)
	{
		// Note: not dispatched when running in a debugger.
		OSG_DEBUG<<"Disconnected"<<std::endl;
	}

	void Listener::onExit(const Leap::Controller& controller)
	{
		OSG_DEBUG<<"Exited"<<std::endl;
	}

	void Listener::onFrame(const Leap::Controller& controller)
	{
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
				if ((currentAction_ != LeapManipulator::LM_Pan)) {
					lastPositionRightHand_ = handRight.stabilizedPalmPosition();
				}
				currentAction_ = LeapManipulator::LM_Pan;
			} else if (frame.hands().count() > 1 && handLeft.fingers().count() >= 3 && handRight.fingers().count() >=3) {
				currentAction_ = LeapManipulator::LM_Rotate;
			} else if (frame.hands().count() > 1 && 
				((handLeft.fingers().count() >= 3 && handRight.fingers().count() <= 1) || (handLeft.fingers().count() <= 1 && handRight.fingers().count() >= 3 ) ))
			{
				if ((currentAction_ != LeapManipulator::LM_Zoom)) {
					handsDistance_ = (handLeft.stabilizedPalmPosition() - handRight.stabilizedPalmPosition()).magnitude();
				}
				currentAction_ = LeapManipulator::LM_Zoom;
			} else {
				currentAction_ = LeapManipulator::LM_None;
			}

			OSG_DEBUG<<"MODE: ";
			if (currentAction_ & LeapManipulator::LM_Rotate) OSG_DEBUG<<"ROTATE ";
			if (currentAction_ & LeapManipulator::LM_Zoom) OSG_DEBUG<<"ZOOM ";
			if (currentAction_ & LeapManipulator::LM_Pan) OSG_DEBUG<<"PAN ";
			if (currentAction_ == 0) OSG_DEBUG<<"NONE";
			OSG_DEBUG<<std::endl;

			osg::Vec3 trans;
			osg::Quat rot;
			osg::Vec3 scale;
			osg::Quat so;

			rot = manipulator_->getMatrix().getRotate();
			trans = manipulator_->getMatrix().getTrans();

			// Rotate Leap axes to match OSG camera axes
			osg::Vec3 deltaPos = osg::Vec3(-(handRight.stabilizedPalmPosition().x-lastPositionRightHand_.x), -(handRight.stabilizedPalmPosition().y-lastPositionRightHand_.y), -(handRight.stabilizedPalmPosition().z-lastPositionRightHand_.z));
			if (currentAction_ & LeapManipulator::LM_Pan) {
				if (deltaPos.x() != 0.0f || deltaPos.y() != 0.0f || deltaPos.z() != 0.0f) { 
					double factor = 2*manipulator_->getSceneRadius(); // scale by model size to fit for very large and very small models
					osg::Vec3 deltaTrans((deltaPos*factor/1000.0f)); // leap tracking: mm, OSG units: m
					trans += rot*deltaTrans;
					manipulator_->panModel(deltaTrans.x(), deltaTrans.y());
				}
			}

			double distance = (handLeft.stabilizedPalmPosition() - handRight.stabilizedPalmPosition()).magnitude();
			if (handsDistance_ != 0.0f) {
				double reference_length = 100.0f;
				if (currentAction_ & LeapManipulator::LM_Zoom) {
					double factor = (handsDistance_-distance)/(reference_length);
					if (factor > 1.0f || factor < -1.0f) { factor = 1.0f; }
					//OSG_DEBUG_FP<<"zooming by "<<factor<<std::endl;
					manipulator_->setDistance(manipulator_->getDistance()*(1+factor));
				}
				if (currentAction_ & LeapManipulator::LM_Rotate) {
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

					manipulator_->setVerticalAxisFixed(true);

					// rotate camera
					if( manipulator_->getVerticalAxisFixed() ) {
						OSG_DEBUG<<"FIXED VERTICAL"<<std::endl;
						manipulator_->rotateWithFixedVertical( movement.x, movement.y );
					} else {
						OSG_DEBUG<<"FLOATING VERTICAL"<<std::endl;
						manipulator_->rotateTrackball( lastPosNorm.x, lastPosNorm.y,
										 curPosNorm.x, curPosNorm.y,
										 manipulator_->getThrowScale( deltaTimeSinceLastFrame ) );
					}
#endif
				}
			}

			handsDistance_ = distance;

			lastPositionLeftHand_ = handLeft.stabilizedPalmPosition();
			lastPositionRightHand_ = handRight.stabilizedPalmPosition();

		} else {
			currentAction_ = LeapManipulator::LM_None;
		}

		lastFrame_ = frame;
		lastFrameStamp_ = currentFrameStamp;
	}

	void Listener::onFocusGained(const Leap::Controller& controller) {
		OSG_DEBUG<<"Focus Gained"<<std::endl;
	}

	void Listener::onFocusLost(const Leap::Controller& controller) {
		OSG_DEBUG<<"Focus Lost"<<std::endl;
	}

	LeapManipulator::LeapManipulator(): osgGA::OrbitManipulator(),
						controller_(new Leap::Controller()),
						listener_(NULL),
						sceneRadius_(1)
	{
		listener_ = new osgLeap::Listener(this);
		controller_->addListener(*listener_);
	}

	LeapManipulator::~LeapManipulator()
	{
		controller_->removeListener(*listener_);
		delete listener_;
		delete controller_;
	}

	LeapManipulator::LeapManipulator(const LeapManipulator& lm,
		const osg::CopyOp& copyOp): OrbitManipulator(lm, copyOp),
		sceneRadius_(lm.sceneRadius_)
	{
		listener_ = new osgLeap::Listener(this);
		controller_ = new Leap::Controller();
		controller_->addListener(*listener_);
	}

	bool LeapManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
	{
		sceneRadius_ = us.asView()->getCamera()->getBound().radius();
		if (sceneRadius_ == 0.0f) { sceneRadius_ = 1.0f; }

		return osgGA::OrbitManipulator::handle(ea, us);
	}
}
