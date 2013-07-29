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
		std::cout << "Initialized" << std::endl;
	}

	void Listener::onConnect(const Leap::Controller& controller)
	{
		std::cout << "Connected" << std::endl;
		controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
		controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
		controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
		controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
	}

	void Listener::onDisconnect(const Leap::Controller& controller)
	{
		//Note: not dispatched when running in a debugger.
		std::cout << "Disconnected" << std::endl;
	}

	void Listener::onExit(const Leap::Controller& controller)
	{
		std::cout << "Exited" << std::endl;
	}

	void Listener::onFrame(const Leap::Controller& controller)
	{
		// Get the most recent frame and report some basic information
		const Leap::Frame frame = controller.frame();
		osg::Timer_t currentFrameStamp = osg::Timer::instance()->tick();
		osg::Timer_t deltaTimeSinceLastFrame = currentFrameStamp - lastFrameStamp_;

		//std::cout << "Frame id: " << frame.id()
		//	<< ", timestamp: " << frame.timestamp()
		//	<< ", hands: " << frame.hands().count()
		//	<< ", fingers: " << frame.fingers().count()
		//	<< ", tools: " << frame.tools().count()
		//	<< ", gestures: " << frame.gestures().count() << std::endl;

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

			std::cout<<"MODE: ";
			if (currentAction_ & LeapManipulator::LM_Rotate) std::cout<<"ROTATE ";
			if (currentAction_ & LeapManipulator::LM_Zoom) std::cout<<"ZOOM ";
			if (currentAction_ & LeapManipulator::LM_Pan) std::cout<<"PAN ";
			if (currentAction_ == 0) std::cout<<"NONE";
			std::cout<<std::endl;

			//// Get the hand's normal vector and direction
			//const Leap::Vector normal = hand.palmNormal();
			//const Leap::Vector direction = hand.direction();

			//if (hand.fingers().count() >= 3) {
			//	//if (hand.sphereRadius() <= 100.0f) {
			//		if (!(currentAction_ & LeapManipulator::LM_Rotate)) {
			//			lastRotation_ = osg::Vec3(direction.yaw(), direction.pitch(), normal.roll());
			//		}
			//		currentAction_ = currentAction_ | LeapManipulator::LM_Rotate;	
			//	//} else {
			//		currentAction_ = currentAction_ | LeapManipulator::LM_Pan;	
			//	//}
			//} else {
			//	currentAction_ = currentAction_ &~ LeapManipulator::LM_Rotate;
			//	currentAction_ = currentAction_ &~ LeapManipulator::LM_Pan;
			//}

			//Leap::FloatArray mat = hand.rotationMatrix(lastFrame_).toArray4x4();
			//osg::Matrix weiter(mat.m_array[0], mat.m_array[1], mat.m_array[2], mat.m_array[3], 
			//	mat.m_array[4], mat.m_array[5], mat.m_array[6], mat.m_array[7], 
			//	mat.m_array[8], mat.m_array[9], mat.m_array[10], mat.m_array[11], 
			//	mat.m_array[12], mat.m_array[13], mat.m_array[14], mat.m_array[15]);
			//
			//manipulator_->setByMatrix(weiter * manipulator_->getMatrix());

			//// Check if the hand has any fingers
			//const Leap::FingerList fingers = hand.fingers();
			//if (!fingers.empty()) {
			//	// Calculate the hand's average finger tip position
			//	Leap::Vector avgPos;
			//	for (int i = 0; i < fingers.count(); ++i) {
			//		avgPos += fingers[i].tipPosition();
			//	}
			//	avgPos /= (float)fingers.count();
			//	std::cout << "Hand has " << fingers.count()
			//		<< " fingers, average finger tip position" << avgPos << std::endl;
			//}

			// Get the hand's sphere radius and palm position
			//std::cout << "Hand sphere radius: " << hand.sphereRadius()
			//	<< " mm, palm position: " << hand.palmPosition() << std::endl;
		
			//if (lastPosition_.isValid()) {
			//	if (currentAction_ & LeapManipulator::LM_Rotate) {
			//		osg::Quat handRot;
			//		osg::Vec3 deltaRot;

			//		Leap::Matrix mat = hand.rotationMatrix(lastFrame_);
			//		Leap::FloatArray values = mat.toArray4x4();
			//		osg::Matrix osgMat(values[0], values[1], values[2], values[3],
			//							values[4], values[5], values[6], values[7],
			//							values[8], values[9], values[10], values[11], 
			//							values[12], values[13], values[14], values[15]);

			//		manipulator_->setRotation(manipulator_->getRotation()*osgMat.getRotate());

			//		//deltaRot = osg::Vec3(lastRotation_.x() - direction.yaw(),
			//		//					lastRotation_.y() - direction.pitch(),
			//		//					lastRotation_.z() - normal.roll());
			//		////handRot = osg::Quat(0.0f, osg::X_AXIS,//deltaRot.x()
			//		////					deltaRot.z(), osg::Y_AXIS,//deltaRot.y()
			//		////					0.0f, osg::Z_AXIS);
			//		////rot = handRot*rot;
			//		//lastRotation_ = osg::Vec3(direction.yaw(), direction.pitch(), normal.roll());

			//		//std::cout<<"\n";
			//		//std::cout << "direction.yaw(): " << direction.yaw() << std::endl;
			//		//std::cout << "direction.pitch(): " << direction.pitch() << std::endl;
			//		//std::cout << "normal.roll(): " << normal.roll() << std::endl;
			//		////std::cout << "Hand stab palm position: " << hand.stabilizedPalmPosition() << std::endl;

			//		//manipulator_->setRotation(manipulator_->getRotation()*osg::Quat(deltaRot.z(), osg::Z_AXIS)*osg::Quat(deltaRot.y(), osg::Y_AXIS)*osg::Quat(deltaRot.x(), osg::X_AXIS));
			//	}

			osg::Vec3 trans;
			osg::Quat rot;
			osg::Vec3 scale;
			osg::Quat so;
			//manipulator_->getMatrix().decompose(trans, rot, scale, so);
			rot = manipulator_->getMatrix().getRotate();
			trans = manipulator_->getMatrix().getTrans();

			// Rotate Leap axes to match OSG camera axes
			osg::Vec3 deltaPos = osg::Vec3(-(handRight.stabilizedPalmPosition().x-lastPositionRightHand_.x), -(handRight.stabilizedPalmPosition().y-lastPositionRightHand_.y), -(handRight.stabilizedPalmPosition().z-lastPositionRightHand_.z));
			if (currentAction_ & LeapManipulator::LM_Pan) {
				if (deltaPos.x() != 0.0f || deltaPos.y() != 0.0f || deltaPos.z() != 0.0f) { 
					double factor = 2*manipulator_->getSceneRadius(); // scale by model size to fit for very large and very small models
					osg::Vec3 deltaTrans((deltaPos*factor/1000.0f));
					//deltaTrans.set(deltaTrans.x(), deltaTrans.z(), deltaTrans.y());
					trans += rot*deltaTrans; // leap tracking: mm, OSG units: m
					//manipulator_->setByMatrix(osg::Matrix::rotate(rot)*osg::Matrix::translate(trans));
					manipulator_->panModel(deltaTrans.x(), deltaTrans.y());
				}
			}

			double distance = (handLeft.stabilizedPalmPosition() - handRight.stabilizedPalmPosition()).magnitude();
			if (handsDistance_ != 0.0f) {
				double reference_length = 100.0f;
				if (currentAction_ & LeapManipulator::LM_Zoom) {
					double factor = (handsDistance_-distance)/(reference_length);
					if (factor > 1.0f || factor < -1.0f) { factor = 1.0f; }
					std::cout<<"zooming by "<<factor<<std::endl;
					manipulator_->setDistance(manipulator_->getDistance()*(1+factor));
				}
				if (currentAction_ & LeapManipulator::LM_Rotate) {
					//**********************
					//Leap::Vector movement = osg::PI_2*(handRight.stabilizedPalmPosition()-lastPositionRightHand_)/reference_length;
					//osg::Quat addRotX(-movement.x, osg::Y_AXIS);//OK!
					//osg::Quat addRotY(movement.y, osg::X_AXIS);//OK!
					//osg::Quat addRotZ(movement.z, osg::Z_AXIS);//movement very strange
					//std::cout<<movement.z<<std::endl;
					//manipulator_->setRotation(addRotX*addRotY*addRotZ*rot);
					//**********************

					Leap::Vector movement = (handRight.stabilizedPalmPosition()-lastPositionRightHand_)/reference_length;
					Leap::Vector lastPosNorm = lastPositionRightHand_/reference_length;
					Leap::Vector curPosNorm = handRight.stabilizedPalmPosition()/reference_length;

					manipulator_->setVerticalAxisFixed(true);

					// rotate camera
					if( manipulator_->getVerticalAxisFixed() ) {
						std::cout<<"FIXED VERTICAL"<<std::endl;
						manipulator_->rotateWithFixedVertical( movement.x, movement.y );
					} else {
						std::cout<<"FLOATING VERTICAL"<<std::endl;
						manipulator_->rotateTrackball( lastPosNorm.x, lastPosNorm.y,
										 curPosNorm.x, curPosNorm.y,
										 manipulator_->getThrowScale( deltaTimeSinceLastFrame ) );
					}

				}
			}

			handsDistance_ = distance;

			lastPositionLeftHand_ = handLeft.stabilizedPalmPosition();
			lastPositionRightHand_ = handRight.stabilizedPalmPosition();

		} else {
			currentAction_ = LeapManipulator::LM_None;
		}

		//// Calculate the hand's pitch, roll, and yaw angles
		//std::cout << "Hand pitch: " << direction.pitch() * Leap::RAD_TO_DEG << " degrees, "
		//	<< "roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees, "
		//	<< "yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees" << std::endl;
		lastFrame_ = frame;

		//// Get gestures
		//const Leap::GestureList gestures = frame.gestures();
		//for (int g = 0; g < gestures.count(); ++g) {
		//	Leap::Gesture gesture = gestures[g];

		//	switch (gesture.type()) {
		//  case Leap::Gesture::TYPE_CIRCLE:
		//	  {
		//		  Leap::CircleGesture circle = gesture;
		//		  std::string clockwiseness;

		//		  if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI/4) {
		//			  clockwiseness = "clockwise";
		//		  } else {
		//			  clockwiseness = "counterclockwise";
		//		  }

		//		  // Calculate angle swept since last frame
		//		  float sweptAngle = 0;
		//		  if (circle.state() != Leap::Gesture::STATE_START) {
		//			  Leap::CircleGesture previousUpdate = Leap::CircleGesture(controller.frame(1).gesture(circle.id()));
		//			  sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
		//		  }
		//		  //std::cout << "Circle id: " << gesture.id()
		//			 // << ", state: " << gesture.state()
		//			 // << ", progress: " << circle.progress()
		//			 // << ", radius: " << circle.radius()
		//			 // << ", angle " << sweptAngle * Leap::RAD_TO_DEG
		//			 // <<  ", " << clockwiseness << std::endl;
		//		  break;
		//	  }
		//  case Leap::Gesture::TYPE_SWIPE:
		//	  {
		//		  Leap::SwipeGesture swipe = gesture;
		//		  //std::cout << "Swipe id: " << gesture.id()
		//			 // << ", state: " << gesture.state()
		//			 // << ", direction: " << swipe.direction()
		//			 // << ", speed: " << swipe.speed() << std::endl;
		//		  break;
		//	  }
		//  case Leap::Gesture::TYPE_KEY_TAP:
		//	  {
		//		  Leap::KeyTapGesture tap = gesture;
		//		  std::cout << "Key Tap id: " << gesture.id()
		//			  << ", state: " << gesture.state()
		//			  << ", position: " << tap.position()
		//			  << ", direction: " << tap.direction()<< std::endl;
		//		  break;
		//	  }
		//  case Leap::Gesture::TYPE_SCREEN_TAP:
		//	  {
		//		  Leap::ScreenTapGesture screentap = gesture;
		//		  std::cout << "Screen Tap id: " << gesture.id()
		//			  << ", state: " << gesture.state()
		//			  << ", position: " << screentap.position()
		//			  << ", direction: " << screentap.direction()<< std::endl;
		//		  break;
		//	  }
		//  default:
		//	  std::cout << "Unknown gesture type." << std::endl;
		//	  break;
		//	}
		//}

		//if (!frame.hands().empty() || !gestures.empty()) {
		//	std::cout << std::endl;
		//}
		lastFrameStamp_ = currentFrameStamp;
	}

	void Listener::onFocusGained(const Leap::Controller& controller) {
		std::cout << "Focus Gained" << std::endl;
	}

	void Listener::onFocusLost(const Leap::Controller& controller) {
		std::cout << "Focus Lost" << std::endl;
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
