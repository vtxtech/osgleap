/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/Device>

//-- Project --//
#include <osgLeap/Event>

namespace osgLeap {

    bool Device::checkEvents()
    {
        OSG_DEBUG_FP<<"PointerEventDevice::checkEvents"<<std::endl;
		if (frame_.id() != lastFrame_.id()) {
			// Create new 'USER' event of class osgLeap::Event and push it to the queue
			osg::ref_ptr<Event> e = new Event();
			e->setFrame(frame_);
			_eventQueue->addEvent(e);
			lastFrame_ = frame_;
		}
        return _eventQueue.valid() ? !(getEventQueue()->empty()) : false;
    }

    void Device::sendEvent(const osgGA::GUIEventAdapter& ea)
    {
        OSG_DEBUG_FP<<"PointerEventDevice::sendEvent"<<std::endl;
    }

	void Device::onFrame(const Leap::Controller& controller)
	{
		frame_ = controller.frame();
	}

} // namespace osgLeap
