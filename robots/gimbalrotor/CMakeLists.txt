cmake_minimum_required(VERSION 3.8)
project(gimbalrotor)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_control REQUIRED)
find_package(aerial_robot_estimation REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(spinal REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(Eigen3 REQUIRED)

ament_export_dependencies(
  aerial_robot_control
  aerial_robot_estimation
  aerial_robot_model
  aerial_robot_msgs
  spinal
  pluginlib
  rclcpp
  Eigen3
)

include_directories(include)

add_library(gimbalrotor_controller src/control/gimbalrotor_controller.cpp)
target_include_directories(gimbalrotor_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(gimbalrotor_controller
  aerial_robot_control
  aerial_robot_estimation
  aerial_robot_model
  aerial_robot_msgs
  pluginlib
  rclcpp
  spinal
)

add_library(gimbalrotor_robot_model src/model/gimbalrotor_robot_model.cpp)
target_include_directories(gimbalrotor_robot_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(gimbalrotor_robot_model
  aerial_robot_model
  aerial_robot_msgs
  rclcpp
)

add_library(gimbalrotor_navigation src/gimbalrotor_navigation.cpp)
target_include_directories(gimbalrotor_navigation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(gimbalrotor_navigation
  aerial_robot_control
  aerial_robot_msgs
  rclcpp
)

install(TARGETS gimbalrotor_controller gimbalrotor_robot_model gimbalrotor_navigation
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY config plugins launch
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
