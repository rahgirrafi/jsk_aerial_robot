cmake_minimum_required(VERSION 3.8)
project(aerial_robot_simulation)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_base REQUIRED)
find_package(aerial_robot_estimation REQUIRED)
find_package(aerial_robot_control REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(kdl_parser REQUIRED)
find_package(urdfdom_headers REQUIRED)
find_package(rclcpp REQUIRED)
find_package(spinal REQUIRED)
find_package(tf2 REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(gazebo REQUIRED)
find_package(Eigen3 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

ament_export_dependencies(
  aerial_robot_base
  aerial_robot_estimation
  aerial_robot_control
  aerial_robot_msgs
  aerial_robot_model
  kdl_parser
  rclcpp
  spinal
  tf2
  gazebo_ros
)

include_directories(
  include
  ${GAZEBO_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

### Gazebo plugin
add_library(aerial_robot_hw_sim
  src/aerial_robot_hw_sim.cpp
)

target_include_directories(aerial_robot_hw_sim PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(aerial_robot_hw_sim
  aerial_robot_base
  aerial_robot_estimation
  aerial_robot_control
  aerial_robot_model
  rclcpp
  gazebo_ros
)

target_link_libraries(aerial_robot_hw_sim ${GAZEBO_LIBRARIES})

# MuJoCo plugin (if applicable)
option(BUILD_MUJOCO_PLUGIN "Build MuJoCo plugin" OFF)

if(BUILD_MUJOCO_PLUGIN)
  find_package(mujoco REQUIRED)
  
  add_library(mujoco_aerial_robot_hw_sim
    src/mujoco_aerial_robot_hw_sim.cpp
  )
  
  target_include_directories(mujoco_aerial_robot_hw_sim PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  
  ament_target_dependencies(mujoco_aerial_robot_hw_sim
    aerial_robot_base
    aerial_robot_estimation
    aerial_robot_control
    aerial_robot_model
    rclcpp
  )
  
  target_link_libraries(mujoco_aerial_robot_hw_sim ${MUJOCO_LIBRARIES})
  
  install(TARGETS mujoco_aerial_robot_hw_sim
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
  )
endif()

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME})

install(TARGETS aerial_robot_hw_sim
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION lib
)

install(DIRECTORY launch config xacro launch
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()