/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/PointerEventDevice>

//-- OSG: osg --//
#include <osg/io_utils>

//-- OSG: osgUtil --//
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
//-- OSG: osgGA --//
#include <osgGA/GUIEventAdapter>

namespace osgLeap {

    bool PointerEventDevice::checkEvents()
    {
        //OSG_NOTICE<<"PointerEventDevice::checkEvents"<<std::endl;
        update();
        return _eventQueue.valid() ? !(getEventQueue()->empty()) : false;
    }

    void PointerEventDevice::sendEvent(const osgGA::GUIEventAdapter& ea)
    {
        OSG_DEBUG_FP<<"PointerEventDevice::sendEvent"<<std::endl;
    }

    bool PointerEventDevice::hasIntersections(osgLeap::Pointer* p) {
        bool hasIntersections = false;
        if (getTraversalMask() != 0 && getView() != NULL) {
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if (getView()->computeIntersections(getView()->getCamera(), osgUtil::Intersector::VIEW, p->getPosition().x(), p->getPosition().y(), intersections, getTraversalMask())) {
                if (intersections.size() > 0) {
                    hasIntersections = true;
                    OSG_DEBUG_FP<<"I HAVE INTERSECTIONS"<<std::endl;
                }
            }
        }
        return hasIntersections;
    }

    bool PointerEventDevice::allowedToClick(osgLeap::Pointer* p)
    {
        return (getView() == NULL || getTraversalMask() == 0 || hasIntersections(p));
    }

    osg::ref_ptr<osgGA::GUIEventAdapter> PointerEventDevice::makeMouseEvent(osgLeap::Pointer* p)
    {
#if 0 // cannot pick osgWidget HUD geodes -> might be a bug in osgWidget or I just dont understand how it works...
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        //OSG_NOTICE<<"x="<<pos.x()<<", y="<<pos.y()<<std::endl;
        //OSG_NOTICE<<"Resolution: "<<p->getResolution().x()<<" / "<<p->getResolution().y()<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> e = new osgGA::GUIEventAdapter(*osgGA::GUIEventAdapter::getAccumulatedEventState());
        e->setX(pos.x());
        e->setY(pos.y());
        e->setTime(_eventQueue->getTime());
#else
		osg::Vec2 pos = p->getPosition(); // e.g. 0..1280, 0..1024
        osg::ref_ptr<osgGA::GUIEventAdapter> e = new osgGA::GUIEventAdapter(*osgGA::GUIEventAdapter::getAccumulatedEventState());
        e->setX(pos.x());
        e->setY(pos.y());
		e->setXmin(0);
		e->setXmax(p->getResolution().x());
		e->setYmin(0);
		e->setYmax(p->getResolution().y());
#endif
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        e->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::mouseMotion(osgLeap::Pointer* p)
    {
#ifdef _DEBUG
        OSG_NOTICE<<"mouseMotion: "<<p->getPosition()<<std::endl;
#endif
        osg::ref_ptr<osgGA::GUIEventAdapter> e = makeMouseEvent(p);
        e->setEventType(e->getButtonMask() ? osgGA::GUIEventAdapter::DRAG : osgGA::GUIEventAdapter::MOVE);
        _eventQueue->addEvent(e);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::mouseButton(osgLeap::Pointer* p, int button, osgGA::GUIEventAdapter::EventType eventType)
    {
#ifdef _DEBUG
        OSG_NOTICE<<"mouseButton: "<<p->getPosition()<<" "<<(eventType==osgGA::GUIEventAdapter::PUSH?"pressed":"released")<<std::endl;
#endif
        osg::ref_ptr<osgGA::GUIEventAdapter> e = makeMouseEvent(p);
        e->setButtonMask(button);
        e->setEventType(eventType);
        e->setButton(button);
        _eventQueue->addEvent(e);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchBegan(osgLeap::Pointer* p)
    {
#ifdef _DEBUG
        OSG_NOTICE<<"touchBegan: "<<p->getPointableID()<<std::endl;
#endif
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchBegan(p->getPointableID(), osgGA::GUIEventAdapter::TOUCH_BEGAN, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchMoved(osgLeap::Pointer* p)
    {
#ifdef _DEBUG
        OSG_NOTICE<<"touchMoved: "<<p->getPosition()<<std::endl;
#endif
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchMoved(p->getPointableID(), osgGA::GUIEventAdapter::TOUCH_MOVED, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchEnded(osgLeap::Pointer* p, unsigned int taps)
    {
#ifdef _DEBUG
        OSG_NOTICE<<"touchEnded: "<<p->getPointableID()<<std::endl;
#endif
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchEnded(p->getPointableID(), osgGA::GUIEventAdapter::TOUCH_ENDED, pos.x(), pos.y(), taps);
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchStationary(osgLeap::Pointer* p)
    {
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchMoved(p->getPointableID(), osgGA::GUIEventAdapter::TOUCH_STATIONERY, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }


    void PointerEventDevice::update()
    {
        intersectionController_->update();

        PointerMap removedPointers = intersectionController_->getRemovedPointers();
        if (emulationMode_ == TOUCH) {
            for (PointerMap::iterator itr = removedPointers.begin(); itr != removedPointers.end(); ++itr) {
                touchEnded(itr->second, 0);
            }
        }

        PointerMap pointers = intersectionController_->getPointers();
        for (PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
            if (emulationMode_ == MOUSE) {
                if (itr->second->hasMoved()) {
                    mouseMotion(itr->second);
                }
            } else if (emulationMode_ == TOUCH) {
                if (itr->second->isNew()) {
                    touchBegan(itr->second);
                } else if (itr->second->hasMoved()) {
                    touchMoved(itr->second);
                } else {
                    touchStationary(itr->second);
                }
            }

            bool doClick = false;
            if (clickMode_ == TIMEBASED_MOUSECLICK) {
                if (allowedToClick(itr->second)) {
                    // ToDo/j.kroeger: Set time to zero as long as there is no appropriate intersection
                    if (itr->second->clickTimeHasElapsed(referenceTime_)) doClick = true;
                }
            } else if (clickMode_ == SCREENTAP) {
                for (Leap::GestureList::const_iterator gtr = intersectionController_->getGestures().begin();
                    gtr != intersectionController_->getGestures().end(); ++gtr)
                {
                    if ((*gtr).type() == Leap::Gesture::TYPE_SCREEN_TAP)
                    {
#ifdef LEAPSDK_1X_COMPATIBILITY
						Leap::PointableList pointables = (*gtr).pointables();
#else
						Leap::PointableList pointables = (*gtr).pointables().extended();
#endif
                        for (Leap::PointableList::const_iterator ptr = pointables.begin();
                            ptr != pointables.end(); ++ptr)
                        {
                            Leap::Pointable pointable = (*ptr);
                            if (pointable.id() == itr->first)
                            {
                                if (allowedToClick(itr->second)) {
                                    doClick = true;
                                }
                            }
                        }
                    }
                }
            }

            if (doClick) {
                if (emulationMode_ == MOUSE) {
                    // Fire a mouse press and a mouse release event on "left mouse button"
                    mouseButton(itr->second, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::PUSH);
                    mouseButton(itr->second, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, osgGA::GUIEventAdapter::RELEASE);
                } else if (emulationMode_ == TOUCH) {
                    // Fire a touch began and a touch ended event with 1 tap
                    touchBegan(itr->second);
                    touchEnded(itr->second, 1);
                }
            }
        }
    }

} // namespace osgLeap
