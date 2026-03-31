cmake_minimum_required(VERSION 3.8)
project(aerial_robot_model)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(interactive_markers REQUIRED)
find_package(robot_localization REQUIRED)
find_package(kdl_parser REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(pluginlib REQUIRED)
find_package(sensor_msgs REQUIRED)
# find_package(spinal REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_kdl REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(urdf REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(orocos_kdl REQUIRED)
find_package(urdfdom_headers REQUIRED)
find_package(Eigen3 REQUIRED)

find_package(aerial_robot_msgs REQUIRED)
find_package(spinal_interfaces REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

# rosidl_generate_interfaces(${PROJECT_NAME}_msgs
#   "srv/AddExtraModule.srv"
#   DEPENDENCIES std_msgs geometry_msgs
# )
# rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME}_msgs "rosidl_typesupport_cpp")


ament_export_dependencies(
  tf2_eigen
  geometry_msgs
  interactive_markers
  robot_localization
  kdl_parser
  rosidl_default_runtime
  pluginlib
  sensor_msgs
  # spinal
  std_msgs
  tf2
  tf2_eigen
  tf2_geometry_msgs
  tf2_kdl
  tf2_ros
  urdf
  visualization_msgs
  orocos_kdl
  urdfdom_headers
  Eigen3
)

include_directories(
  include
  test
  ${EIGEN3_INCLUDE_DIRS}
)

add_library(aerial_robot_model
  src/model/base_model/robot_model.cpp
  src/model/transformable_model/robot_model.cpp
  src/model/transformable_model/jacobians.cpp
  src/model/transformable_model/kinematics.cpp
  src/model/transformable_model/stability.cpp
  src/model/transformable_model/statics.cpp
)

target_include_directories(aerial_robot_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(aerial_robot_model
  tf2_eigen
  geometry_msgs
  robot_localization
  kdl_parser
  sensor_msgs
  # spinal
  std_msgs
  tf2
  tf2_eigen
  tf2_geometry_msgs
  tf2_kdl
  tf2_ros
  urdf
)

target_link_libraries(aerial_robot_model
  ${orocos_kdl_LIBRARIES}
  ${EIGEN3_LIBRARIES}
)

add_library(aerial_robot_model_ros src/model/base_model/robot_model_ros.cpp)

target_include_directories(aerial_robot_model_ros PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(aerial_robot_model_ros
  tf2_eigen
  geometry_msgs
  robot_localization
  kdl_parser
  sensor_msgs
  # spinal
  std_msgs
  tf2
  tf2_eigen
  tf2_geometry_msgs
  tf2_kdl
  tf2_ros
  aerial_robot_msgs
  spinal_interfaces
)

target_link_libraries(aerial_robot_model_ros
  aerial_robot_model
  ${orocos_kdl_LIBRARIES}
  ${EIGEN3_LIBRARIES}
)

add_library(robot_model_pluginlib
  src/model/plugin/multirotor_robot_model.cpp
  src/model/plugin/underactuated_tilted_robot_model.cpp
)

target_include_directories(robot_model_pluginlib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(robot_model_pluginlib
  pluginlib
)

target_link_libraries(robot_model_pluginlib
  aerial_robot_model
)

# add_library(servo_bridge src/servo_bridge/servo_bridge.cpp)

# target_include_directories(servo_bridge PUBLIC
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#   $<INSTALL_INTERFACE:include>
# )

# ament_target_dependencies(
#   servo_bridge
#   rclcpp
#   sensor_msgs
#   # spinal
#   std_srvs
# )

# add_executable(servo_bridge_node src/servo_bridge/servo_bridge_node.cpp)

# target_link_libraries(servo_bridge_node servo_bridge)

add_executable(rotor_tf_publisher src/utils/rotor_tf_publisher.cpp)

target_include_directories(rotor_tf_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(rotor_tf_publisher
  kdl_parser
  rclcpp
  geometry_msgs
  tf2_ros
  tf2_kdl
  urdf
)

target_link_libraries(rotor_tf_publisher ${orocos_kdl_LIBRARIES})

add_executable(interactive_marker_tf_broadcaster src/utils/interactive_marker_tf_broadcaster.cpp)

target_include_directories(interactive_marker_tf_broadcaster PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(interactive_marker_tf_broadcaster
  interactive_markers
  rclcpp
  tf2_ros
)

### for rostest
# add_library(numerical_jacobians test/aerial_robot_model/numerical_jacobians.cpp)

# target_include_directories(numerical_jacobians PUBLIC
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>
#   $<INSTALL_INTERFACE:include>
# )

# target_link_libraries(numerical_jacobians  # ← add aerial_robot_model here
#   aerial_robot_model
# )

install(DIRECTORY include/${PROJECT_NAME}/ test/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

install(TARGETS aerial_robot_model aerial_robot_model_ros robot_model_pluginlib #numerical_jacobians #servo_bridge 
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS rotor_tf_publisher interactive_marker_tf_broadcaster #servo_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch script plugins
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
