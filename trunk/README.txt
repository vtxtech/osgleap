# -----------------------------------------------------------------------------
# osgLeap is released under the GNU LESSER GENERAL PUBLIC LICENSE v3.0.
# Created by Johannes Scholz (vtxtech)
# -----------------------------------------------------------------------------
#
# -----------------------------------------------------------------------------
# Change Notes osgLeap v.0.1.3
# ------------------------------
#
# * Added osgLeap::HandState visualizer to give the user a hint on what the Leap
#     Device is currently seeing. 
# * Introduced osgLeap::UpdateCallback for visualization of the positions you
#     are pointing at. Refer to example_leappointer for usage examples.
# * Renamed osgLeap::LeapManipulator to osgLeap::OrbitManipulator. This is done
#     in order to avoid naming issues with future additional manipulators.
# * Renamed example_leapdemo to example_leaporbit
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
