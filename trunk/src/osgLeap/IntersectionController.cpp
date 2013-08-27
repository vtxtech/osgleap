/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/IntersectionController>

//-- Project --//
#include <osgLeap/Controller>

//-- OSG: osg --//
#include <osg/CopyOp>
#include <osg/Referenced>
#include <osg/Timer>

//-- STL --//
#include <list>

namespace osgLeap {

    IntersectionController::IntersectionController(int screenwidth, int screenheight): osgLeap::Listener(),
        frame_(Leap::Frame()), camera_(NULL)
    {
        osgLeap::Controller::instance()->addListener(*this);
    }

     IntersectionController::IntersectionController(osg::Camera* camera): camera_(camera),
            screenwidth_(800), screenheight_(600)
    {
        osgLeap::Controller::instance()->addListener(*this);
    }

    IntersectionController::~IntersectionController()
    {
        osgLeap::Controller::instance()->removeListener(*this);
    }

    IntersectionController::IntersectionController(const IntersectionController& lm,
        const osg::CopyOp& copyOp): osgLeap::Listener(*this),
        frame_(Leap::Frame()),
        screenwidth_(lm.screenwidth_),
        screenheight_(lm.screenheight_),
        camera_(lm.camera_)
    {

    }

    void IntersectionController::setResolution(int screenwidth, int screenheight)
    {
        screenwidth_ = screenwidth;
        screenheight_ = screenheight;
    }

    void IntersectionController::onFrame(const Leap::Controller& controller)
    {
        // Get the most recent frame and store it to later use in handle(...)
        frame_ = controller.frame();

        // assume first screen is the one we want...
        screen_ = controller.locatedScreens()[0];
    }

    void IntersectionController::update()
    {
        Leap::PointableList pl = frame_.pointables();
        Leap::Screen screen = screen_;

        // Auto-update to reference camera's resolution
        // Please use setResolution to update manually, if this object
        // is constructed without reference camera.
        if (camera_ != NULL) {
            screenheight_ = camera_->getGraphicsContext()->getTraits()->height;
            screenwidth_  = camera_->getGraphicsContext()->getTraits()->width;
        }

        // Update pointers as required. Add new pointers where additional pointables
        // result in a valid intersection.
        std::list<int> validIDs;
        for (Leap::PointableList::const_iterator itr = pl.begin(); itr != pl.end(); ++itr) {
            Leap::Vector pos = screen.intersect(*itr, true);
            // Calculate pixel screen position from relative Leap values [X: 0.0 to 1.0, Y: 0.0 to 1.0]
            // using the 3D window resolution. Z is always zero.
            pos.x = std::ceil(pos.x * screenwidth_);
            pos.y = std::ceil(pos.y * screenheight_);

            // skip pointable if no valid intersection
            if (!pos.isValid()) { continue; }
            // lookup Pointer for this pointable
            PointerMap::iterator pointer = pointers_.find((*itr).id());
            validIDs.push_back((*itr).id());
            if (pointer == pointers_.end()) {
                // Not found: Add a new pointer
                osg::ref_ptr<Pointer> newPointer = new Pointer(osg::Vec2(pos.x, pos.y), (*itr).id());
                pointers_.insert(PointerPair((*itr).id(), newPointer));
            } else {
                // Found: Update pointer position
                pointer->second->setPosition(pos.x, pos.y);
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
        for (std::list<int>::const_iterator itr = unusedIDs.begin(); itr != unusedIDs.end(); ++itr) {
            PointerMap::const_iterator pitr  = pointers_.find(*itr);
            if (pitr != pointers_.end()) {
                pointers_.erase(pitr);
            }
        }

    }

}
