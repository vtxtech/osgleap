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
            // ToDo: Emit mouseMotion if mouse has moved, only.
            OSG_NOTICE<<"MOUSEMOTION @ "<<itr->second->getPosition()<<std::endl;
            _eventQueue->mouseMotion(itr->second->getPosition().x(), itr->second->getPosition().y());
            // ToDo: Make elapsedTime definable
            if (itr->second->clickTimeHasElapsed(3000)) {
                _eventQueue->mouseButtonPress(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                _eventQueue->mouseButtonRelease(itr->second->getPosition().x(), itr->second->getPosition().y(), 1);
                OSG_NOTICE<<"CLICK @ "<<itr->second->getPosition()<<std::endl;
            }
        }
    }

} // namespace osgLeap
