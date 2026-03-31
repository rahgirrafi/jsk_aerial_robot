cmake_minimum_required(VERSION 3.8)
project(hydrus_xi)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(angles REQUIRED)
find_package(hydrus REQUIRED)
find_package(rclcpp REQUIRED)
find_package(NLopt REQUIRED)
find_package(OsqpEigen REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

ament_export_dependencies(angles hydrus rclcpp NLopt OsqpEigen)

include_directories(include ${OsqpEigen_INCLUDE_DIRS} ${NLOPT_INCLUDE_DIRS})

add_library(hydrus_xi_under_actuated_navigation src/hydrus_xi_under_actuated_navigation.cpp)
target_include_directories(hydrus_xi_under_actuated_navigation PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(hydrus_xi_under_actuated_navigation hydrus rclcpp)
target_link_libraries(hydrus_xi_under_actuated_navigation ${OsqpEigen_LIBRARIES} ${NLOPT_LIBRARIES})

add_library(hydrus_xi_fully_actuated_robot_model src/hydrus_xi_fully_actuated_robot_model.cpp)
target_include_directories(hydrus_xi_fully_actuated_robot_model PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(hydrus_xi_fully_actuated_robot_model hydrus rclcpp)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

install(TARGETS hydrus_xi_under_actuated_navigation hydrus_xi_fully_actuated_robot_model
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY config scripts plugins launch
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(FILES flight_navigation_plugin.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
