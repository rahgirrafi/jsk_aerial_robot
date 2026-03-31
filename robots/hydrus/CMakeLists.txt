cmake_minimum_required(VERSION 3.8)
project(hydrus)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(aerial_robot_control REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(spinal REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_conversions REQUIRED)
find_package(std_srvs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(Eigen3 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

ament_export_dependencies(
  aerial_robot_msgs
  aerial_robot_control
  aerial_robot_model
  spinal
  tf2_eigen
  tf2_conversions
  std_srvs
  pluginlib
  rclcpp
  Eigen3
)

include_directories(include test ${EIGEN3_INCLUDE_DIRS})

add_library(hydrus_robot_model
  src/hydrus_robot_model.cpp
  src/hydrus_tilted_robot_model.cpp
)
target_include_directories(hydrus_robot_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(hydrus_robot_model
  aerial_robot_msgs
  aerial_robot_model
  tf2_eigen
  tf2_conversions
  rclcpp
)

add_library(hydrus_controller_pluginlib
  src/hydrus_lqi_controller.cpp
  src/hydrus_tilted_lqi_controller.cpp
)
target_include_directories(hydrus_controller_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(hydrus_controller_pluginlib
  aerial_robot_control
  aerial_robot_model
  aerial_robot_msgs
  pluginlib
  spinal
  rclcpp
)
target_link_libraries(hydrus_controller_pluginlib hydrus_robot_model ${EIGEN3_LIBRARIES})

add_library(hydrus_numerical_jacobians test/hydrus/numerical_jacobians.cpp)
target_include_directories(hydrus_numerical_jacobians PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(hydrus_numerical_jacobians
  aerial_robot_model
  rclcpp
)
target_link_libraries(hydrus_numerical_jacobians hydrus_robot_model)

install(DIRECTORY include/${PROJECT_NAME}/ test/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

install(TARGETS hydrus_robot_model hydrus_controller_pluginlib hydrus_numerical_jacobians
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY config scripts plugins launch
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(DIRECTORY bin
  DESTINATION lib/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()

