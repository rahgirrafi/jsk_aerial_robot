cmake_minimum_required(VERSION 3.8)
project(aerial_robot_estimation)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(geodesy REQUIRED)
find_package(geographic_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_localization REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(spinal REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_conversions REQUIRED)
find_package(jsk_recognition_msgs REQUIRED)
find_package(OpenCV REQUIRED)

ament_export_dependencies(
  aerial_robot_model
  aerial_robot_msgs
  cv_bridge
  geodesy
  geographic_msgs
  geometry_msgs
  robot_localization
  nav_msgs
  pluginlib
  sensor_msgs
  spinal
  tf2
  tf2_conversions
  jsk_recognition_msgs
)

include_directories(include ${OpenCV_INCLUDE_DIRS})

### state estimation
add_library(aerial_robot_estimation
  src/state_estimation.cpp)

target_include_directories(aerial_robot_estimation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(aerial_robot_estimation
  aerial_robot_model
  aerial_robot_msgs
  cv_bridge
  geodesy
  geographic_msgs
  geometry_msgs
  robot_localization
  nav_msgs
  pluginlib
  sensor_msgs
  spinal
  tf2
  tf2_conversions
  jsk_recognition_msgs
)

### sensor plugins
add_library(sensor_pluginlib
  src/sensor/vo.cpp
  src/sensor/altitude.cpp
  src/sensor/mocap.cpp
  src/sensor/gps.cpp
  src/sensor/imu.cpp
  src/sensor/plane_detection.cpp
)

target_include_directories(sensor_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(sensor_pluginlib
  aerial_robot_model
  aerial_robot_msgs
  cv_bridge
  geometry_msgs
  robot_localization
  nav_msgs
  pluginlib
  sensor_msgs
  spinal
  tf2
  tf2_conversions
)

target_link_libraries(sensor_pluginlib aerial_robot_estimation)

### kalman filter plugins
add_library(kf_baro_bias_pluginlib
  src/kf/baro_bias_plugin.cpp
  src/kf/xy_roll_pitch_bias_plugin.cpp)

target_include_directories(kf_baro_bias_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(kf_baro_bias_pluginlib
  robot_localization
  nav_msgs
  sensor_msgs
)

set(USE_GPU OFF)

if(USE_GPU)
  add_definitions(-DUSE_GPU)
endif()

add_library(optical_flow src/vision/optical_flow.cpp)

target_include_directories(optical_flow PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(optical_flow
  cv_bridge
  sensor_msgs
  geometry_msgs
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME})

install(TARGETS aerial_robot_estimation sensor_pluginlib kf_baro_bias_pluginlib optical_flow
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY launch plugins config
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
