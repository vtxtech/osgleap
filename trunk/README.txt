# -----------------------------------------------------------------------------
# osgLeap is released under the GNU LESSER GENERAL PUBLIC LICENSE v3.0.
# Created by Johannes Kroeger (vtxtech)
# -----------------------------------------------------------------------------
#
# -----------------------------------------------------------------------------
# Change Notes osgLeap v.0.3.0
# ------------------------------
#
# * Introduced osgLeap::PointerEventDevice to generate native OpenSceneGraph
#     events from your Leap Motion actions. As the name suggests this class
#     focuses on generating (or 'emulating') 2D mouse or touch events for all
#     fingers detected pointing at the screen. PointerEventDevice supports
#     two modes for click emulation or may be deactivated:
#      1) Time-based (e.g. if pointer is standing still for 5 seconds, however
#         the actual trigger time can be defined by the user)
#      2) Screentap (fires a mouse or touch event if the Leap 'Screentap'
#         gesture is done)
#     PointerEventDevice may be configured to fire either mouse or touch events
#     however the touch emulation is to be considered experimental. Feedback on
#     the touch emulation mode is greatly appreciated.
#     To test PointerEventDevice run the leappointer example.
#
# * Added osgLeap::HandState visualizer to give the user a hint on what the Leap
#     Device is currently seeing. 
#
# * Introduced osgLeap::UpdateCallback for visualization of the positions you
#     are pointing at. Refer to example_leappointer for usage examples.
#
# * Renamed osgLeap::LeapManipulator to osgLeap::OrbitManipulator. This is done
#     in order to avoid naming issues with future additional manipulators.
#
# * Renamed example_leapdemo to example_leaporbit
#
# * Introduced osgLeap::HUDCamera class to simplify examples
#
# -----------------------------------------------------------------------------
# Change Notes osgLeap v.0.1.2
# ------------------------------
#
# * Build support for g++ on Linux (tested on g++ 4.6 on Xubuntu 12.04)
# * Introduced "singlehanded" and "trackball" single handed manipulator modes
# * Introduced compatiblity mode for LeapSDK 0.8.0 (in CMake options please
#     enable LEAPSDK_080_COMPATIBILITYMODE)
#
# -----------------------------------------------------------------------------
# Change Notes osgLeap v.0.1.1
# ------------------------------
#
# * Code cleanup and build support for MSVC++ 2008, 2010 and 2012
#
# -----------------------------------------------------------------------------
# Change Notes osgLeap v.0.1.0
# ------------------------------
#
# * Introduced osgLeap::LeapManipulator
#
# -----------------------------------------------------------------------------
