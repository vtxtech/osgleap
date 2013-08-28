/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/IntersectionDevice>

//-- OSG: osg --//
#include <osg/io_utils>

namespace osgLeap {

    bool IntersectionDevice::checkEvents()
    {
        //OSG_NOTICE<<"IntersectionDevice::checkEvents"<<std::endl;
        update();
        return _eventQueue.valid() ? !(getEventQueue()->empty()) : false;
    }

    void IntersectionDevice::sendEvent(const osgGA::GUIEventAdapter& ea)
    {
        OSG_NOTICE<<"IntersectionDevice::sendEvent"<<std::endl;
    }

    void IntersectionDevice::update()
    {
        intersectionController_->update();

        PointerMap pointers = intersectionController_->getPointers();
        for (PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
            // ToDo: Emit mouseMotion if pointer has moved, only.
            _eventQueue->mouseMotion(itr->second->getPosition().x(), itr->second->getPosition().y());
            bool doClick = false;
            if (mode_ == TIMEBASED_MOUSECLICK) {
                // ToDo: Add a NodeMask & combine that with an intersection. Invoke a click only if matching intersection is under cursor.
                if (itr->second->clickTimeHasElapsed(referenceTime_)) doClick = true;
            } else if (mode_ == SCREENTAP) {
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
                                doClick = true;
                            }
                        }
                    }
                }
            }

            if (doClick) {
                _eventQueue->mouseButtonPress(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                _eventQueue->mouseButtonRelease(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                OSG_NOTICE<<"CLICK @ "<<itr->second->getPosition()<<std::endl;
            }
        }
    }

} // namespace osgLeap
