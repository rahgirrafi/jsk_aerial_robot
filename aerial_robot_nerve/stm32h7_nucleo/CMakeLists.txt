cmake_minimum_required(VERSION 3.8)
project(stm32h7_nucleo)

find_package(ament_cmake REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_target(${PROJECT_NAME}_generate_ros_lib ALL
  COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/make_libraries.py
    --save_path ${PROJECT_SOURCE_DIR}
    --support_rtos
    --support_ethernet
)

install(DIRECTORY bin
  DESTINATION lib/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(DIRECTORY scripts
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
