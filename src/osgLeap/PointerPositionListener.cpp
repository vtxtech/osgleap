/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/PointerPositionListener>

//-- Project --//
#include <osgLeap/Controller>

//-- OSG: osg --//
#include <osg/CopyOp>
#include <osg/Referenced>
#include <osg/Timer>

//-- STL --//
#include <list>

namespace osgLeap {

    PointerPositionListener::PointerPositionListener(int windowwidth, int windowheight): osgLeap::Listener(),
        frame_(Leap::Frame()), lastFrame_(Leap::Frame()), camera_(NULL),
        gestures_(Leap::GestureList())
    {
        osgLeap::Controller::instance()->addListener(*this);
    }

     PointerPositionListener::PointerPositionListener(osg::Camera* camera): camera_(camera),
            windowwidth_(800), windowheight_(600), frame_(Leap::Frame()), lastFrame_(Leap::Frame()),
            gestures_(Leap::GestureList())
    {
        osgLeap::Controller::instance()->addListener(*this);
    }

    PointerPositionListener::~PointerPositionListener()
    {
        osgLeap::Controller::instance()->removeListener(*this);
    }

    PointerPositionListener::PointerPositionListener(const PointerPositionListener& lm,
        const osg::CopyOp& copyOp): osgLeap::Listener(*this),
        frame_(Leap::Frame()),
        lastFrame_(Leap::Frame()),
        gestures_(Leap::GestureList()),
        windowwidth_(lm.windowwidth_),
        windowheight_(lm.windowheight_),
        camera_(lm.camera_)
    {

    }

    void PointerPositionListener::setResolution(int windowwidth, int windowheight)
    {
        windowwidth_ = windowwidth;
        windowheight_ = windowheight;
    }

    void PointerPositionListener::onFrame(const Leap::Controller& controller)
    {
        // Get the most recent frame and store it to later use in handle(...)
        frame_ = controller.frame();

        // assume first screen is the one we want...
        screen_ = controller.locatedScreens()[0];
    }

    void PointerPositionListener::update()
    {
        Leap::PointableList pl = frame_.pointables();
        Leap::Screen screen = screen_;
        gestures_ = frame_.gestures(lastFrame_);
        removedPointers_.clear();

        // Auto-update to reference camera's resolution
        // Please use setResolution to update manually, if this PointerPositionListener
        // is constructed without reference camera.
        if (camera_ != NULL) {
            windowheight_ = camera_->getGraphicsContext()->getTraits()->height;
            windowwidth_  = camera_->getGraphicsContext()->getTraits()->width;
        }
        osg::Vec2 resolution(windowwidth_, windowheight_);

        // Update pointers as required. Add new pointers where additional pointables
        // result in a valid intersection.
        std::list<int> validIDs;
        for (Leap::PointableList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr) {
            Leap::Vector pos = screen.intersect(*itr, true);
            // Calculate pixel screen position from relative Leap values [X: 0.0 to 1.0, Y: 0.0 to 1.0]
            // using the 3D window resolution. Z is always zero.
            pos.x = std::ceil(pos.x * windowwidth_);
            pos.y = std::ceil(pos.y * windowheight_);

            // skip pointable if no valid intersection
            if (!pos.isValid()) { continue; }
            // lookup Pointer for this pointable
            PointerMap::iterator pointer = pointers_.find((*itr).id());
            validIDs.push_back((*itr).id());
            if (pointer == pointers_.end()) {
                // Not found: Add a new pointer
                osg::ref_ptr<Pointer> newPointer = new Pointer(osg::Vec2(pos.x, pos.y), resolution, (*itr).id());
                pointers_.insert(PointerPair((*itr).id(), newPointer));
            } else {
                // Found: Update pointer position
                pointer->second->setPosition(pos.x, pos.y);
                pointer->second->setResolution(resolution);
            }
        }

        // Lookup unused pointers
        std::list<int> unusedIDs;
        for (PointerMap::const_iterator itr = pointers_.begin(); itr != pointers_.end(); ++itr) {
            bool found = false;
            for (std::list<int>::const_iterator iitr = validIDs.begin(); iitr != validIDs.end(); ++iitr) {
                if (itr->first == *iitr) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                unusedIDs.push_back(itr->first);
            }
        }

        // Remove unused pointers
        for (std::list<int>::iterator itr = unusedIDs.begin(); itr != unusedIDs.end(); ++itr) {
            PointerMap::iterator pitr  = pointers_.find(*itr);
            if (pitr != pointers_.end()) {
                removedPointers_.insert(PointerPair((*pitr).first, (*pitr).second));
                pointers_.erase(pitr);
            }
        }

        // Remember the last frame we handled
        lastFrame_ = frame_;

    }

}
