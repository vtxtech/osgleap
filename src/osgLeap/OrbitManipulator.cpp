/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/OrbitManipulator>

//-- Project --//
#include <osgLeap/Controller>
#include <osgLeap/Event>

//-- OSG: osg --//
#include <osg/Referenced>
#include <osg/Timer>

//-- Leap --//
#include <LeapMath.h>

namespace osgLeap {

    // Workaround for LeapSDK 0.8.0 or lower which do not include stabilizedPalmPosition
    // ToDo/j.kroeger: Detect version of LeapSDK instead
    Leap::Vector getPalmPosition(const Leap::Hand& hand) {
#ifdef LEAPSDK_080_COMPATIBILITYMODE
        return hand.palmPosition();
#else
        return hand.stabilizedPalmPosition();
#endif
    }

    OrbitManipulator::OrbitManipulator(const Mode& mode): osgGA::OrbitManipulator(),
        mode_(mode),
        leftHandID_(0),
        rightHandID_(0),
        lastLeftHand_(Leap::Hand()),
        lastRightHand_(Leap::Hand()),
        handsDistance_(0.0f),
        currentAction_(LM_None)
    {
  //      osgLeap::Controller::instance()->addListener(*this);
		//OSG_DEBUG_FP<<"Added osgLeap::OrbitManipulator "<<this<<" to osgLeap::Controller::instance() "<<osgLeap::Controller::instance()<<std::endl;
    }

    OrbitManipulator::~OrbitManipulator()
    {
  //      osgLeap::Controller::instance()->removeListener(*this);
		//OSG_DEBUG_FP<<"Removed osgLeap::OrbitManipulator "<<this<<" from osgLeap::Controller::instance() "<<osgLeap::Controller::instance()<<std::endl;
    }

    OrbitManipulator::OrbitManipulator(const OrbitManipulator& lm,
        const osg::CopyOp& copyOp): osgGA::OrbitManipulator(lm, copyOp),
        mode_(lm.mode_),
        leftHandID_(0),
        rightHandID_(0),
        lastLeftHand_(Leap::Hand()),
        lastRightHand_(Leap::Hand()),
        handsDistance_(0.0f),
        currentAction_(LM_None)
    {

    }

    bool OrbitManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::USER) {
			const osgLeap::Event* ev = dynamic_cast<const osgLeap::Event*>(&ea);
			if (ev != NULL) {
				const Leap::Frame& frame = ev->getFrame();

				OSG_DEBUG_FP << "Frame id: " << frame.id()
					<< ", timestamp: " << frame.timestamp()
					<< ", hands: " << frame.hands().count()
					<< ", fingers: " << frame.fingers().count()
					<< ", tools: " << frame.tools().count()
					<< ", gestures: " << frame.gestures().count() << std::endl;

				if (!frame.hands().isEmpty()) {
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
					double reference_length = 100.0f;

					if (mode_ == SingleHanded) {
						if (frame.hands().count() > 0 && handRight.fingers().count() >= 3) {
							if (!(currentAction_ & LM_Rotate )) {
								lastRightHand_ = handRight;
							}
							currentAction_ = LM_Rotate | LM_Zoom;
							Leap::Vector movement = (getPalmPosition(handRight)-getPalmPosition(lastRightHand_))/reference_length;
							OSG_DEBUG<<"FIXED VERTICAL"<<std::endl;
							rotateWithFixedVertical( movement.x, movement.y );
							zoomModel(-movement.z);
							us.requestRedraw();
						}
					} else if (mode_ == Trackball) {
						if (frame.hands().count() > 0 && handRight.fingers().count() >= 3) {
							if (!(currentAction_ & LM_Rotate)) {
								lastRightHand_ = handRight;
							}
							currentAction_ = LM_Rotate | LM_Zoom | LM_Pan;

							if (currentAction_ & LM_Rotate) {
								//Leap::FloatArray a(handRight.rotationMatrix(lastFrame_).toArray4x4());
								//osg::Matrix mat_rot(a.m_array[0], a.m_array[1], a.m_array[2], a.m_array[3], a.m_array[4], a.m_array[5], a.m_array[6], a.m_array[7], a.m_array[8], 
								//	a.m_array[9], a.m_array[10], a.m_array[11], a.m_array[12], a.m_array[13], a.m_array[14], a.m_array[15]);
								//osg::Matrix rot(a.m_array[0], a.m_array[4], a.m_array[8], a.m_array[12], a.m_array[1], a.m_array[5], a.m_array[9], a.m_array[13], a.m_array[2], 
								//	a.m_array[6], a.m_array[10], a.m_array[14], a.m_array[3], a.m_array[7], a.m_array[11], a.m_array[15]);
								//setRotation(mat_rot.getRotate()*getRotation());

								Leap::Vector lastRot(lastRightHand_.direction().yaw(), lastRightHand_.direction().pitch(), lastRightHand_.palmNormal().roll());
								Leap::Vector curRot(handRight.direction().yaw(), handRight.direction().pitch(), handRight.palmNormal().roll());
								Leap::Vector deltaRot = curRot - lastRot;
								//if ((deltaRot.x != 0.0f) || (deltaRot.y != 0.0f) || (deltaRot.z != 0.0f)) { 
								//	OSG_DEBUG<<"FIXED VERTICAL"<<std::endl;
								//	rotateWithFixedVertical( -deltaRot.x, deltaRot.z );
								//}

								osg::Quat addRot(deltaRot.x, getRotation()*osg::Y_AXIS, -deltaRot.y, getRotation()*osg::X_AXIS, -deltaRot.z, getRotation()*osg::Z_AXIS);
								setRotation(getRotation()*addRot); // does work only if no rotation is there..
								us.requestRedraw();

								//osg::Vec3 trans;
								//osg::Quat rot;
								//trans = getMatrix().getTrans();
								//rot = getMatrix().getRotate();
								//setByMatrix(/*osg::Matrix::rotate(addRot)**/osg::Matrix::rotate(rot)*osg::Matrix::translate(trans)*osg::Matrix::rotate(addRot));//flipping
							}

							if (currentAction_ & LM_Zoom) {
								Leap::Vector movement = (getPalmPosition(handRight)-getPalmPosition(lastRightHand_))/reference_length;
								zoomModel(-movement.z);
								us.requestRedraw();
							}

							if (currentAction_ & LM_Pan) {
								osg::Vec3 deltaPos = osg::Vec3(-(getPalmPosition(handRight).x-getPalmPosition(lastRightHand_).x), -(getPalmPosition(handRight).y-getPalmPosition(lastRightHand_).y), -(getPalmPosition(handRight).z-getPalmPosition(lastRightHand_).z));
								if (deltaPos.x() != 0.0f || deltaPos.y() != 0.0f || deltaPos.z() != 0.0f) { 
									// scale by model size to fit for very large and very small models
									double factor = 2*us.asView()->getCamera()->getBound().radius();
									// scale translation units. leap tracking: mm, OSG units: m
									osg::Vec3 deltaTrans((deltaPos*factor/1000.0f));
									panModel(deltaTrans.x(), deltaTrans.y());
									us.requestRedraw();
								}
							}

						} else {
							currentAction_ = LM_None;
						}
					} else { //TwoHanded
						if (frame.hands().count() == 1 && handRight.fingers().count() >= 3) {
							if ((currentAction_ != LM_Pan)) {
								lastRightHand_ = handRight;
							}
							currentAction_ = LM_Pan;
						} else if (frame.hands().count() > 1 && handLeft.fingers().count() >= 3 && handRight.fingers().count() >=3) {
							currentAction_ = LM_Rotate;
						} else if (frame.hands().count() > 1 && 
							((handLeft.fingers().count() >= 3 && handRight.fingers().count() <= 1) || (handLeft.fingers().count() <= 1 && handRight.fingers().count() >= 3 ) ))
						{
							if ((currentAction_ != LM_Zoom)) {
								handsDistance_ = (getPalmPosition(handLeft) - getPalmPosition(handRight)).magnitude();
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
						osg::Vec3 deltaPos = osg::Vec3(-(getPalmPosition(handRight).x-getPalmPosition(lastRightHand_).x), -(getPalmPosition(handRight).y-getPalmPosition(lastRightHand_).y), -(getPalmPosition(handRight).z-getPalmPosition(lastRightHand_).z));
						if (currentAction_ & LM_Pan) {
							if (deltaPos.x() != 0.0f || deltaPos.y() != 0.0f || deltaPos.z() != 0.0f) { 
								// scale by model size to fit for very large and very small models
								double factor = 2*us.asView()->getCamera()->getBound().radius();
								// scale translation units. leap tracking: mm, OSG units: m
								osg::Vec3 deltaTrans((deltaPos*factor/1000.0f));
								trans += rot*deltaTrans;
								panModel(deltaTrans.x(), deltaTrans.y());
								us.requestRedraw();
							}
						}

						double distance = (getPalmPosition(handLeft) - getPalmPosition(handRight)).magnitude();
						if (handsDistance_ != 0.0f) {
							if (currentAction_ & LM_Zoom) {
								double factor = (handsDistance_-distance)/(reference_length);
								if (factor > 1.0f || factor < -1.0f) { factor = 1.0f; }
								zoomModel(factor);
								us.requestRedraw();
							}
							if (currentAction_ & LM_Rotate) {
#if 0
								Leap::Vector movement = osg::PI_2*(handRight.stabilizedPalmPosition()-lastRightHand_.stabilizedPalmPosition())/reference_length;
								osg::Quat addRotX(-movement.x, osg::Y_AXIS);
								osg::Quat addRotY(movement.y, osg::X_AXIS);
								osg::Quat addRotZ;//(movement.z, osg::Z_AXIS);//movement very strange
								manipulator_->setRotation(addRotX*addRotY*addRotZ*rot);
#else
								// At the moment, Fixed VerticalAxis is the only mode supported
								// because rotateTrackball is not yet working correctly.
								if( true /*manipulator_->getVerticalAxisFixed()*/ ) {
									Leap::Vector movement = (getPalmPosition(handRight)-getPalmPosition(lastRightHand_))/reference_length;
									OSG_DEBUG<<"FIXED VERTICAL"<<std::endl;
									rotateWithFixedVertical( movement.x, movement.y );
									us.requestRedraw();
								} else {
									Leap::Vector lastPosNorm = getPalmPosition(lastRightHand_)/reference_length;
									Leap::Vector curPosNorm = getPalmPosition(handRight)/reference_length;
									OSG_DEBUG<<"FLOATING VERTICAL"<<std::endl;
									//rotateTrackball( lastPosNorm.x, lastPosNorm.y,
									//	curPosNorm.x, curPosNorm.y,
									//	getThrowScale( deltaTimeSinceLastFrame ) );
									us.requestRedraw();
								}
#endif
							}
						}

						handsDistance_ = distance;

					}

					lastLeftHand_ = handLeft;
					lastRightHand_ = handRight;

				} else {
					currentAction_ = LM_None;
				}

			}
		}

        return osgGA::OrbitManipulator::handle(ea, us);
    }

  //  void OrbitManipulator::onFrame(const Leap::Controller& controller) {
		//// Get the most recent frame and store it to later use in handle(...)
  //      frame_ = controller.frame();
		//OSG_DEBUG_FP<<"onFrame: frame_.fingers().count()="<<frame_.fingers().count()<<std::endl;
  //      frameStamp_ = osg::Timer::instance()->tick();
  //  }

}
