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
        OSG_NOTICE<<"PointerEventDevice::sendEvent"<<std::endl;
    }

    bool PointerEventDevice::hasIntersections(osgLeap::Pointer* p) {
        bool hasIntersections = false;
        if (getTraversalMask() != 0 && getView() != NULL) {
            osgUtil::LineSegmentIntersector::Intersections intersections;
            if (getView()->computeIntersections(getView()->getCamera(), osgUtil::Intersector::CoordinateFrame::VIEW, p->getPosition().x(), p->getPosition().y(), intersections, getTraversalMask())) {
                if (intersections.size() > 0) {
                    hasIntersections = true;
                    OSG_NOTICE<<"I HAVE INTERSECTIONS"<<std::endl;
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
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        OSG_NOTICE<<"x="<<pos.x()<<", y="<<pos.y()<<std::endl;
        //OSG_NOTICE<<"Resolution: "<<p->getResolution().x()<<" / "<<p->getResolution().y()<<std::endl;
        osg::ref_ptr<osgGA::GUIEventAdapter> e = new osgGA::GUIEventAdapter(*osgGA::GUIEventAdapter::getAccumulatedEventState());
        e->setX(pos.x());
        e->setY(pos.y());
        e->setTime(_eventQueue->getTime());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        e->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::mouseMotion(osgLeap::Pointer* p)
    {
        osg::ref_ptr<osgGA::GUIEventAdapter> e = makeMouseEvent(p);
        e->setEventType(e->getButtonMask() ? osgGA::GUIEventAdapter::DRAG : osgGA::GUIEventAdapter::MOVE);
        _eventQueue->addEvent(e);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::mouseButtonPress(osgLeap::Pointer* p)
    {
        OSG_NOTICE<<"mouse press @ "<<p->getPosition()<<std::endl;
        //_eventQueue->mouseButtonPress(p->getPosition().x(), p->getPosition().y(), 1);
        osg::ref_ptr<osgGA::GUIEventAdapter> e = makeMouseEvent(p);
        e->setButtonMask(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
        e->setEventType(osgGA::GUIEventAdapter::PUSH);
        e->setButton(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
        _eventQueue->addEvent(e);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::mouseButtonRelease(osgLeap::Pointer* p)
    {
        OSG_NOTICE<<"mouse release @ "<<p->getPosition()<<std::endl;
        //_eventQueue->mouseButtonRelease(p->getPosition().x(), p->getPosition().y(), 1);
        osg::ref_ptr<osgGA::GUIEventAdapter> e = makeMouseEvent(p);
        e->setButtonMask(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
        e->setEventType(osgGA::GUIEventAdapter::RELEASE);
        e->setButton(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
        _eventQueue->addEvent(e);
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchBegan(osgLeap::Pointer* p)
    {
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchBegan(p->getPointableID(), osgGA::GUIEventAdapter::TouchPhase::TOUCH_BEGAN, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchMoved(osgLeap::Pointer* p)
    {
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchMoved(p->getPointableID(), osgGA::GUIEventAdapter::TouchPhase::TOUCH_MOVED, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchEnded(osgLeap::Pointer* p, unsigned int taps)
    {
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchEnded(p->getPointableID(), osgGA::GUIEventAdapter::TouchPhase::TOUCH_ENDED, pos.x(), pos.y(), taps);
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }

    osgGA::GUIEventAdapter* PointerEventDevice::touchStationary(osgLeap::Pointer* p)
    {
        osg::Vec2 pos = p->getRelativePositionInScreenCoordinates();
        osgGA::GUIEventAdapter* e = _eventQueue->touchMoved(p->getPointableID(), osgGA::GUIEventAdapter::TouchPhase::TOUCH_STATIONERY, pos.x(), pos.y());
        e->setWindowWidth(p->getResolution().x());
        e->setWindowHeight(p->getResolution().y());
        return e;
    }


    void PointerEventDevice::update()
    {
        intersectionController_->update();

        PointerMap removedPointers = intersectionController_->getRemovedPointers();
        for (PointerMap::iterator itr = removedPointers.begin(); itr != removedPointers.end(); ++itr) {
            touchEnded(itr->second, 0);
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
                        Leap::PointableList pointables = (*gtr).pointables();
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
                    mouseButtonPress(itr->second);
                    mouseButtonRelease(itr->second);
                } else if (emulationMode_ == TOUCH) {
                    // Fire a touch began and a touch ended event with 1 tap
                    touchBegan(itr->second);
                    touchEnded(itr->second, 1);
                }
            }
        }
    }

} // namespace osgLeap
