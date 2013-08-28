/*
* Library osgLeap
* Copyright (C) 2013 Johannes Scholz/vtxtech. All rights reserved.
*
* This file is licensed under the GNU Lesser General Public License 3 (LGPLv3),
* but distributed WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osgLeap/HUDCamera>

//-- Project --//
#include <osgLeap/Controller>

//-- OSG: osgDB --//
#include <osgDB/ReadFile>

namespace osgLeap {

    class ResizeUpdateCallback: public osg::NodeCallback
    {
    public:
        ResizeUpdateCallback(osg::Camera* camera): camera_(camera)
        {

        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            float windowheight = camera_->getGraphicsContext()->getTraits()->height;
            float windowwidth  = camera_->getGraphicsContext()->getTraits()->width;

            camera_->setProjectionMatrix(osg::Matrix::ortho2D(0.0f, windowwidth, 0.0f, windowheight));
            camera_->setViewport(0, 0, windowwidth, windowheight);

            traverse(node, nv);
        }

    private:
        osg::Camera* camera_;
    };

    HUDCamera::HUDCamera(osgViewer::GraphicsWindow* graphicsWindow): osg::Camera()
    {
        // Initialize UpdateCallback to update myself during updateTraversal
        addUpdateCallback(new ResizeUpdateCallback(this));

        // set the projection matrix
        setProjectionMatrix(osg::Matrix::ortho2D(0, 640, 0, 480));

        // set the view matrix
        setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        setViewMatrix(osg::Matrix::identity());

        // only clear the depth buffer
        setClearMask(GL_DEPTH_BUFFER_BIT);

        // draw subgraph after main camera view.
        setRenderOrder(osg::Camera::POST_RENDER);

        // we don't want the camera to grab event focus from the viewers main camera(s).
        setAllowEventFocus(false);

        setGraphicsContext(graphicsWindow);
    }

    HUDCamera::~HUDCamera()
    {
        if (getUpdateCallback() != NULL) {
            removeUpdateCallback(getUpdateCallback());
        }
    }

    HUDCamera::HUDCamera(const HUDCamera& hs,
        const osg::CopyOp& copyOp): osg::Camera(*this)
    {

    }

}
