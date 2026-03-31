cmake_minimum_required(VERSION 3.8)
project(motor_test)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(spinal REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(motor_test_node src/motor_test.cpp)
ament_target_dependencies(motor_test_node
  rclcpp
  std_msgs
  std_srvs
  geometry_msgs
  spinal
)

install(TARGETS motor_test_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY scripts
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
