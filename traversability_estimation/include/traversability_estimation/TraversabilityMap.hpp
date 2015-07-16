/*
 * TraversabilityMap.hpp
 *
 *  Created on: Jun 09, 2015
 *      Author: Martin Wermelinger
 *	 Institute: ETH Zurich, Autonomous Systems Lab
 */

#pragma once

// Grid Map
#include <grid_map/grid_map.hpp>
#include <traversability_msgs/TraversabilityResult.h>
#include <traversability_msgs/FootprintPath.h>

// ROS
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <filters/filter_chain.h>
#include <std_srvs/Empty.h>
#include <sensor_msgs/Image.h>

// Schweizer-Messer
#include <sm/timing/Timer.hpp>

// STD
#include <vector>
#include <string>

// Boost
#include <boost/thread/recursive_mutex.hpp>

namespace traversability_estimation {

/*!
 * The terrain traversability estimation main class. Coordinates the ROS
 * interfaces, the timing, and the data handling between the other classes.
 */
class TraversabilityMap
{

 public:
  /*!
   * Constructor.
   */
  TraversabilityMap(ros::NodeHandle& nodeHandle);

  /*!
   * Destructor.
   */
  virtual ~TraversabilityMap();

  /*!
   * Computes the traversability based on the elevation map.
   * Traversability is set between 0.0 and 1.0, where a value of 0.0 means not
   * traversable and 1.0 means fully traversable.
   * @return true if successful.
   */
  bool computeTraversability();

  /*!
   * Checks the traversability of a footprint path and returns the traversability.
   * @param[in] path the footprint path that has to be checked.
   * @param[out] result the traversability result.
   * @return true if successful.
   */
  bool checkFootprintPath(const traversability_msgs::FootprintPath& path, traversability_msgs::TraversabilityResult& result);

  /*!
   * Computes the traversability of a footprint at each map cell position twice:
   * first oriented in x-direction, and second oriented according to the yaw angle.
   * @param[in] footprintYaw orientation of the footprint.
   * @return true if successful.
   */
  bool traversabilityFootprint(double footprintYaw);
  bool traversabilityFootprint(const double& radius, const double& offset);

  /*!
   * The filter chain is reconfigured with the actual parameter on the parameter server.
   * @return true if successful.
   */
  bool updateFilter();

  /*!
   * Set the traversability map from layers of a grid_map_msgs::GridMap.
   * @param[in] msg grid map with the layers of a traversability map.
   * @return true if successful.
   */
  bool setTraversabilityMap(const grid_map_msgs::GridMap& msg);
  void setTraversabilityMap(const grid_map::GridMap& map);

  /*!
   * Set the elevation map from layers of a grid_map_msgs::GridMap.
   * @param[in] msg grid map with a layer 'elevation'.
   * @return true if successful.
   */
  bool setElevationMap(const grid_map_msgs::GridMap& msg);

  /*!
   * Get the traversability map.
   * @return the requested traversability map.
   */
  grid_map::GridMap getTraversabilityMap();

  void resetTraversabilityFootprintLayers();

  void printTraversableFraction();

  void publishTraversabilityMap();

  bool traversabilityMapInitialized();

 private:

  /*!
   * Reads and verifies the ROS parameters.
   * @return true if successful.
   */
  bool readParameters();

  /*!
   * Gets the traversability value of the submap defined by the polygon. Is true if the
   * whole polygon is traversable.
   * @param[in] polygon polygon that defines submap of the traversability map.
   * @param[out] traversability traversability value of submap defined by the polygon, the traversability
   * is the mean of each cell within the polygon.
   * @return true if the whole polygon is traversable, false otherwise.
   */
  bool isTraversable(grid_map::Polygon& polygon, double& traversability);

  bool isTraversable(const grid_map::Position& center, const double& radiusMax, double& traversability, const double& radiusMin = 0);

  /*!
   * Checks if the overall inclination of the robot on a line between two
   * positions is feasible.
   * @param[in] start first position of the line.
   * @param[in] end last position of the line.
   * @return true if the whole line has a feasible inclination, false otherwise.
   */
  bool checkInclination(const grid_map::Position& start,
                        const grid_map::Position& end);

  /*!
   * Checks if the map is traversable, only regarding steps, at the position defined
   * by the map index.
   * Small ditches and holes are not detected as steps.
   * @param[in] index index of the map to check.
   * @return true if no step is detected, false otherwise.
   */
  bool checkForStep(const grid_map::Index& indexStep);

  /*!
   * Checks if the map is traversable, only regarding slope, at the position defined
   * by the map index.
   * Small local slopes are not detected as slopes.
   * @param[in] index index of the map to check.
   * @return true if no step is detected, false otherwise.
   */
  bool checkForSlope(const grid_map::Index& index);

  //! ROS node handle.
  ros::NodeHandle& nodeHandle_;

  //! Id of the frame of the elevation map.
  std::string mapFrameId_;

  //! Id of the frame of the robot.
  std::string robotFrameId_;

  //! Publisher of the traversability occupancy grid.
  ros::Publisher traversabilityMapPublisher_;

  //! Publisher of the roughness filter occupancy grid.
  ros::Publisher footprintPolygonPublisher_;

  //! Vertices of the footprint polygon in base frame.
  std::vector<geometry_msgs::Point32> footprintPoints_;

  //! Default value for traversability of unknown regions.
  double traversabilityDefault_;

  //! Traversability map types.
  const std::string traversabilityType_;
  const std::string slopeType_;
  const std::string stepType_;
  const std::string roughnessType_;
  const std::string robotSlopeType_;

  //! Filter Chain
  filters::FilterChain<grid_map::GridMap> filter_chain_;

  //! Traversability map.
  grid_map::GridMap traversabilityMap_;
  std::vector<std::string> traversabilityMapLayers_;
  bool traversabilityMapInitialized_;

  //! Traversability map.
  grid_map::GridMap elevationMap_;
  std::vector<std::string> elevationMapLayers_;
  bool elevationMapInitialized_;

  int nTraversable_;
  int nNotTraversable_;

  //! Mutex lock for traversability map.
  boost::recursive_mutex traversabilityMapMutex_;
  boost::recursive_mutex elevationMapMutex_;

//  //! Timer
//  std::string timerId_;
//  sm::timing::Timer timer_;
};

} /* namespace */
