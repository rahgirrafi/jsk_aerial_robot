cmake_minimum_required(VERSION 3.8)
project(aerial_robot_control)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_estimation REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(spinal REQUIRED)
find_package(tf2 REQUIRED)
find_package(Eigen3 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

ament_export_dependencies(
  aerial_robot_estimation
  aerial_robot_model
  aerial_robot_msgs
  pluginlib
  rclcpp
  spinal
  tf2
  Eigen3
)

include_directories(include ${EIGEN3_INCLUDE_DIRS})

### control utils
add_library(control_utils
  src/control/utils/care.cpp
)
target_link_libraries(control_utils ${EIGEN3_LIBRARIES})

### flight control plugin
add_library(flight_control_pluginlib
  src/control/base/pose_linear_controller.cpp
  src/control/fully_actuated_controller.cpp
  src/control/under_actuated_controller.cpp
  src/control/under_actuated_lqi_controller.cpp
  src/control/under_actuated_tilted_lqi_controller.cpp
)

target_include_directories(flight_control_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(flight_control_pluginlib
  aerial_robot_estimation
  aerial_robot_model
  aerial_robot_msgs
  pluginlib
  rclcpp
  spinal
  tf2
)

target_link_libraries(flight_control_pluginlib control_utils)

### flight navigation
add_library(flight_navigation src/flight_navigation.cpp src/util/joy_parser.cpp)

target_include_directories(flight_navigation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(flight_navigation
  aerial_robot_msgs
  geometry_msgs
  rclcpp
  sensor_msgs
  spinal
)

### trajectory generation
add_library(trajectory_generation
  src/trajectory/reference_base.cpp
  src/trajectory/trajectory_reference/polynomial.cpp
  src/trajectory/trajectory_reference/polynomial_trajectory.cpp
  src/trajectory/trajectory_reference/sampled_trajectory.cpp
  src/trajectory/base/parameter_base.cpp
  src/trajectory/math/math.cpp
  src/trajectory/types/command.cpp
  src/trajectory/types/quad_state.cpp
  src/trajectory/types/quadrotor.cpp
  src/trajectory/utils/logger.cpp
  src/trajectory/utils/timer.cpp
)

target_include_directories(trajectory_generation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(trajectory_generation
  aerial_robot_msgs
  geometry_msgs
  rclcpp
  nav_msgs
)

add_executable(trajectory_generation_node src/trajectory/test.cpp)
target_link_libraries(trajectory_generation_node trajectory_generation)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME})

install(TARGETS control_utils flight_control_pluginlib flight_navigation trajectory_generation
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS trajectory_generation_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch plugins scripts cfg
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()