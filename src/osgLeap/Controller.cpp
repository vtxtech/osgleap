/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/Controller>

//-- OSG: osg --//
#include <osg/Notify>

#ifdef LEAPSDK_080_COMPATIBILITYMODE
// Fix for missing implementation of virtual destructor
namespace Leap {
    Controller::~Controller() {

    }
}
#endif

namespace osgLeap {

    Controller* Controller::instance(bool erase)
    {
        static osg::ref_ptr<Controller> instance_ = new Controller;

        OSG_DEBUG<<"Accessed: osgLeap::Controller="<<instance_<<std::endl;

        if (erase) 
        {   
            instance_ = NULL;
        }
        return instance_.get(); // will return NULL on erase
    }

} // namespace osgLeap
