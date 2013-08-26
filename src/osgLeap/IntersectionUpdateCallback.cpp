/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/IntersectionUpdateCallback>

//-- OSG: osg --//
#include <osg/io_utils>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/ValueObject>

namespace osgLeap {

    void IntersectionUpdateCallback::setResolution(int screenwidth, int screenheight)
    {
        screenwidth_ = screenwidth;
        screenheight_ = screenheight;
    }

    osg::ref_ptr<osg::Node> IntersectionUpdateCallback::createPointerGeode() {
        osg::ref_ptr<osg::Geode> sphere = new osg::Geode();
        float radius = 10.0f;
        osg::TessellationHints* hints = new osg::TessellationHints();
        hints->setDetailRatio(0.5f);
        osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius),hints);
        sd->setColor(getColor());
        sphere->addDrawable(sd);
        return sphere;
    }

    osg::Vec4 IntersectionUpdateCallback::getColor() {
        osg::Vec4 result;

        switch (colorIndex_) {
        case 0:
            result = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 1:
            result = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 2:
            result = osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f);
            break;
        case 3:
            result = osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 4:
            result = osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f);
            break;
        case 5:
            result = osg::Vec4(1.0f, 0.0f, 1.0f, 1.0f);
            break;
        case 6:
            result = osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f);
            break;
        case 7:
            result = osg::Vec4(0.5f, 0.1f, 0.0f, 1.0f);
            break;
        case 8:
            result = osg::Vec4(1.0f, 0.5f, 0.5f, 1.0f);
            break;
        case 9:
            result = osg::Vec4(1.0f, 1.0f, 0.5f, 1.0f);
            break;
        default:
            result = osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }

        colorIndex_++;
        if (colorIndex_ > 9) { colorIndex_ = 0; }

        return result;
    }

    void osgLeap::IntersectionUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        // Grab data from Leap Motion
        intersectionController_->update();

        // Auto-update to reference camera's resolution
        // Please use setResolution to update manually, if this object
        // is constructed without reference camera.
        if (camera_ != NULL) {
            screenheight_ = camera_->getGraphicsContext()->getTraits()->height;
            screenwidth_  = camera_->getGraphicsContext()->getTraits()->width;
        }

        // Now update our Geode to display the pointers
        osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(node);
        if (group.valid()) {
            group->setDataVariance(osg::Object::DYNAMIC);

            osgLeap::PointerMap pointers = intersectionController_->getPointers();

            PatMap transforms;
            // Remove any pointers not visible anymore
            for (int n = group->getNumChildren()-1; n >= 0; --n) {
                int pid = -1;
                bool remove = true;
                if (group->getChild(n)->getUserValue<int>("PointableID", pid)) {
                    for (osgLeap::PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
                        // Still visible in current frame
                        if (itr->first == pid) remove = false;
                    }
                }
                if (remove) {
                    group->removeChild(n);
                } else {
                    transforms.insert(PatPair(pid, dynamic_cast<osg::PositionAttitudeTransform*>(group->getChild(n))));
                }
            }

            // Add more pointers if required or update if they have moved
            for (osgLeap::PointerMap::iterator itr = pointers.begin(); itr != pointers.end(); ++itr) {
                osg::ref_ptr<osgLeap::Pointer> p = itr->second;

                PatMap::iterator patitr = transforms.find(itr->first);
                osg::ref_ptr<osg::PositionAttitudeTransform> pat = NULL;
                if (patitr != transforms.end()) {
                    pat = patitr->second;
                } else {
                    // Add new pointer
                    pat = new osg::PositionAttitudeTransform();
                    pat->setUserValue<int>("PointableID", itr->first);
                    pat->addChild(createPointerGeode());
                    // Add to scene graph
                    group->addChild(pat);
                    // Add to local reference map
                    transforms.insert(PatPair(itr->first, pat));
                }

                // Calculate pixel screen position from relative Leap values [X: 0.0 to 1.0, Y: 0.0 to 1.0]
                // using the 3D window resolution. Z is always zero.
                osg::Vec3 vec = osg::Vec3(p->getPosition().x()*screenwidth_, p->getPosition().y()*screenheight_, 0.0f);
                pat->setPosition(vec);
                if (p->clickTimeHasElapsed(3000)) {
                    OSG_NOTICE<<"CLICK @ "<<vec<<std::endl;
                }
            }
        }

        traverse(node, nv);
    }

} // namespace osgLeap
