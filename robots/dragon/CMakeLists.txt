cmake_minimum_required(VERSION 3.8)
project(dragon)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(aerial_robot_control REQUIRED)
find_package(aerial_robot_model REQUIRED)
find_package(aerial_robot_msgs REQUIRED)
find_package(hydrus REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(spinal REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(NLopt REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

ament_export_dependencies(
  aerial_robot_control
  aerial_robot_model
  aerial_robot_msgs
  hydrus
  pluginlib
  rclcpp
  spinal
  Eigen3
  NLopt
)

include_directories(include test ${EIGEN3_INCLUDE_DIRS} ${NLOPT_INCLUDE_DIRS})

add_library(dragon_sensor_pluginlib src/sensor/imu.cpp)
target_include_directories(dragon_sensor_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(dragon_sensor_pluginlib
  aerial_robot_estimation
  aerial_robot_msgs
  pluginlib
  rclcpp
  spinal
)

add_library(dragon_robot_model
  src/model/hydrus_like_robot_model.cpp
  src/model/full_vectoring_robot_model.cpp
)
target_include_directories(dragon_robot_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(dragon_robot_model
  aerial_robot_model
  aerial_robot_msgs
  hydrus
  pluginlib
  rclcpp
)
target_link_libraries(dragon_robot_model ${NLOPT_LIBRARIES})

add_library(dragon_navigation src/dragon_navigation.cpp)
target_include_directories(dragon_navigation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(dragon_navigation
  aerial_robot_control
  aerial_robot_msgs
  rclcpp
)

add_library(dragon_aerial_robot_controllib
  src/control/lqi_gimbal_control.cpp
  src/control/full_vectoring_control.cpp
)
target_include_directories(dragon_aerial_robot_controllib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(dragon_aerial_robot_controllib
  aerial_robot_control
  aerial_robot_model
  aerial_robot_msgs
  rclcpp
)
target_link_libraries(dragon_aerial_robot_controllib
  dragon_robot_model
  dragon_navigation
  dragon_sensor_pluginlib
  ${EIGEN3_LIBRARIES}
)

# Keep desktop integration script behavior from ROS1 package.
add_custom_target(${PROJECT_NAME}_install_desktop ALL
  COMMAND ${PROJECT_SOURCE_DIR}/bin/desktop/desktop_install.sh
)

add_library(dragon_numerical_jacobians test/dragon/numerical_jacobians.cpp)
target_include_directories(dragon_numerical_jacobians PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(dragon_numerical_jacobians aerial_robot_model rclcpp)
target_link_libraries(dragon_numerical_jacobians dragon_robot_model)

install(DIRECTORY include/${PROJECT_NAME}/ test/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

install(TARGETS dragon_sensor_pluginlib dragon_robot_model dragon_aerial_robot_controllib dragon_navigation dragon_numerical_jacobians
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY config scripts plugins launch images
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(DIRECTORY bin
  DESTINATION lib/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()

