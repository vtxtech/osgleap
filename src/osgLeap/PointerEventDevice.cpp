/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
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

    void PointerEventDevice::update()
    {
        intersectionController_->update();

        PointerMap removedPointers = intersectionController_->getRemovedPointers();
        for (PointerMap::iterator itr = removedPointers.begin(); itr != removedPointers.end(); ++itr) {
            // ToDo/j.scholz: No tapping here. Could it be useful to simulate somehow?
            _eventQueue->touchEnded(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_ENDED, itr->second->getPosition().x(), itr->second->getPosition().y(), 0);
        }

        PointerMap pointers = intersectionController_->getPointers();
        for (PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
            if (emulationMode_ == MOUSE) {
                if (itr->second->hasMoved()) {
                    _eventQueue->mouseMotion(itr->second->getPosition().x(), itr->second->getPosition().y());
                }
            } else if (emulationMode_ == TOUCH) {
                if (itr->second->isNew()) {
                    _eventQueue->touchBegan(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_BEGAN, itr->second->getPosition().x(), itr->second->getPosition().y());
                } else if (itr->second->hasMoved()) {
                    _eventQueue->touchMoved(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_MOVED, itr->second->getPosition().x(), itr->second->getPosition().y());
                } else {
                    _eventQueue->touchMoved(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_STATIONERY, itr->second->getPosition().x(), itr->second->getPosition().y());
                }
            }

            bool doClick = false;
            if (clickMode_ == TIMEBASED_MOUSECLICK) {
                if (allowedToClick(itr->second)) {
                    // ToDo/j.scholz: Set time to zero as long as there is no appropriate intersection
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
                    OSG_NOTICE<<"CLICK @ "<<itr->second->getPosition()<<std::endl;
                    _eventQueue->mouseButtonPress(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                    _eventQueue->mouseButtonRelease(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                } else if (emulationMode_ == TOUCH) {
                    // Fire a touch began and a touch ended event with 1 tap
                    OSG_NOTICE<<"TOUCH TAP @ "<<itr->second->getPosition()<<std::endl;
                    _eventQueue->touchBegan(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_BEGAN, itr->second->getPosition().x(), itr->second->getPosition().y());
                    _eventQueue->touchEnded(itr->first, osgGA::GUIEventAdapter::TouchPhase::TOUCH_ENDED, itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                }
            }
        }
    }

} // namespace osgLeap
