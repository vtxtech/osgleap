/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/PointerGraphicsUpdateCallback>

//-- OSG: osg --//
#include <osg/io_utils>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/ValueObject>

namespace osgLeap {

    osg::ref_ptr<osg::Node> PointerGraphicsUpdateCallback::createPointerGeode(unsigned int /*num*/) {
        osg::ref_ptr<osg::Geode> sphere = new osg::Geode();
        float radius = 10.0f;
        osg::TessellationHints* hints = new osg::TessellationHints();
        hints->setDetailRatio(0.5f);
        osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f), radius), hints);
        sd->setColor(getColor());
        sphere->addDrawable(sd);
        return sphere;
    }

    osg::Vec4 PointerGraphicsUpdateCallback::getColor() {
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

    void osgLeap::PointerGraphicsUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        // Grab data from Leap Motion
        intersectionController_->update();

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
                    if (referenceTime_ != 0) {
                        osg::Geode* geode = dynamic_cast<osg::Geode*>(pat->getChild(0));
                        if (geode) {
                            osg::ShapeDrawable* sd = dynamic_cast<osg::ShapeDrawable*>(geode->getDrawable(0));
                            float f = 1-p->clickTimeProgress(referenceTime_);
                            osg::Vec4 color(sd->getColor().x(), sd->getColor().y(), sd->getColor().z(), f);
                            sd->setColor(color);
                            sd->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
                            sd->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                        }
                    }
                } else {
                    // Add new pointer
					osg::ref_ptr<osg::Node> pt = createPointerGeode(transforms.size());
					if (pt.valid()) {
						pat = new osg::PositionAttitudeTransform();
						pat->setUserValue<int>("PointableID", itr->first);
						pat->addChild(pt);
						// Add to scene graph
						group->addChild(pat);
						// Add to local reference map
						transforms.insert(PatPair(itr->first, pat));
					}
                }

                osg::Vec3 vec = osg::Vec3(p->getPosition().x(), p->getPosition().y(), 0.0f);
				if (pat.valid()) pat->setPosition(vec);
            }
        }

        traverse(node, nv);
    }

} // namespace osgLeap
