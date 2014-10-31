/*
* Library osgLeap
* Copyright (C) 2013 Johannes Kroeger/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/HUDCamera>

//-- OSG: osgDB --//
#include <osgDB/ReadFile>

namespace osgLeap {

    class ResizeUpdateCallback: public osg::NodeCallback
    {
    public:
        ResizeUpdateCallback(osg::Camera* masterCamera, osg::Camera* slaveCamera): masterCamera_(masterCamera), slaveCamera_(slaveCamera)
        {

        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
			if (masterCamera_->getViewport() != NULL) {
				float windowheight = masterCamera_->getViewport()->height();
				float windowwidth  = masterCamera_->getViewport()->width();
				
				slaveCamera_->setProjectionMatrix(osg::Matrix::ortho2D(0, windowwidth, 0, windowheight));
			} else {
				OSG_WARN<<"WARN: ResizeUpdateCallback::operator() -- masterCamera_ has no osg::Viewport defined!"<<std::endl;
			}
            traverse(node, nv);
        }

    private:
        osg::Camera* masterCamera_;
        osg::Camera* slaveCamera_;
    };

    HUDCamera::HUDCamera(osg::Camera* masterCamera): osg::Camera()
    {
        // Initialize UpdateCallback to update myself during updateTraversal
        addUpdateCallback(new ResizeUpdateCallback(masterCamera, this));

        // set the projection matrix
		if (masterCamera->getViewport() != NULL) {
			setProjectionMatrix(osg::Matrix::ortho2D(0, masterCamera->getViewport()->width(), 0, masterCamera->getViewport()->height()));
		} else {
			OSG_WARN<<"WARN: HUDCamera::HUDCamera(osg::Camera* masterCamera) -- masterCamera has no osg::Viewport defined!"<<std::endl;
		}

        // set the view matrix
        setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        setViewMatrix(osg::Matrix::identity());

        // only clear the depth buffer
        setClearMask(GL_DEPTH_BUFFER_BIT);

        // draw subgraph after main camera view.
        setRenderOrder(osg::Camera::POST_RENDER);

        // we don't want the camera to grab event focus from the viewers main camera(s).
        setAllowEventFocus(false);
    }

    HUDCamera::~HUDCamera()
    {
        if (getUpdateCallback() != NULL) {
            removeUpdateCallback(getUpdateCallback());
        }
    }

    HUDCamera::HUDCamera(const HUDCamera& hs,
        const osg::CopyOp& copyOp): osg::Camera(hs)
    {

    }

}
