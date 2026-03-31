cmake_minimum_required(VERSION 3.8)
project(spinal)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
# find_package(rqt_gui REQUIRED)
# find_package(rqt_gui_py REQUIRED)

# Message and service files
set(msg_files
  "msg/PMatrixPseudoInverseUnit.msg"
  "msg/PMatrixPseudoInverseWithInertia.msg"
  "msg/ServoInfo.msg"
  "msg/BoardInfo.msg"
  "msg/ServoState.msg"
  "msg/ServoStates.msg"
  "msg/ServoControlCmd.msg"
  "msg/ServoTorqueCmd.msg"
  "msg/ServoTorqueStates.msg"
  "msg/Gyro.msg"
  "msg/SimpleImu.msg"
  "msg/FourAxisCommand.msg"
  "msg/Imu.msg"
  "msg/Barometer.msg"
  "msg/Gps.msg"
  "msg/GpsFull.msg"
  "msg/RollPitchYawTerm.msg"
  "msg/RollPitchYawTerms.msg"
  "msg/MotorInfo.msg"
  "msg/PwmInfo.msg"
  "msg/Pwms.msg"
  "msg/PwmTest.msg"
  "msg/UavInfo.msg"
  "msg/DesireCoord.msg"
  "msg/FlightConfigCmd.msg"
  "msg/Vector3Int16.msg"
  "msg/TorqueAllocationMatrixInv.msg"
  "msg/ESCTelemetry.msg"
  "msg/ESCTelemetryArray.msg"
  "msg/JointProfile.msg"
  "msg/JointProfiles.msg"
)

set(srv_files
  "srv/GetBoardInfo.srv"
  "srv/SetBoardConfig.srv"
  "srv/SetAttitudeGains.srv"
  "srv/ImuCalib.srv"
  "srv/MagDeclination.srv"
  "srv/SetDirectServoConfig.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES std_msgs builtin_interfaces
)

ament_export_dependencies(rosidl_runtime_cpp std_msgs builtin_interfaces)

set(SPINAL_DIRS mcu_project/lib/Jsk_Lib)

add_definitions(-DSIMULATION)

add_library(spinal_math
  ${SPINAL_DIRS}/math/AP_Math.cpp
  ${SPINAL_DIRS}/math/matrix3.cpp
  ${SPINAL_DIRS}/math/vector2.cpp
  ${SPINAL_DIRS}/math/vector3.cpp
  ${SPINAL_DIRS}/math/quaternion.cpp
  ${SPINAL_DIRS}/math/location.cpp
)

target_include_directories(spinal_math PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SPINAL_DIRS}>
  $<INSTALL_INTERFACE:include>
)

add_library(spinal_flight_controller
  ${SPINAL_DIRS}/flight_control/attitude/attitude_control.cpp
)

target_link_libraries(spinal_flight_controller PUBLIC spinal_math)
target_include_directories(spinal_flight_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SPINAL_DIRS}>
  $<INSTALL_INTERFACE:include>
)

# # Generate ros_lib for STM32F7
# # Note: This custom target may need adjustment based on your Python environment setup
# add_custom_target(${PROJECT_NAME}_generate_stm32f7_ros_lib ALL
#   COMMAND ${CMAKE_COMMAND} -E echo "Note: STM32F7 ros_lib generation script requires manual configuration for ROS2"
# )

# # Generate ros_lib for STM32H7 targets.
# add_custom_target(${PROJECT_NAME}_generate_stm32h7_ros_lib ALL
#   COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/make_libraries.py
#     --save_path ${PROJECT_SOURCE_DIR}/mcu_project/boards/stm32H7
#     --support_rtos
#     --support_ethernet
# )

# add_custom_target(${PROJECT_NAME}_generate_stm32h7_v2_ros_lib ALL
#   COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/make_libraries.py
#     --save_path ${PROJECT_SOURCE_DIR}/mcu_project/boards/stm32H7_v2
#     --support_rtos
#     --support_ethernet
# )

install(DIRECTORY ${SPINAL_DIRS}/
  DESTINATION include/${PROJECT_NAME}
)

install(DIRECTORY bin
  DESTINATION lib/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(DIRECTORY scripts resource launch
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

install(FILES rqt_gui_plugin.xml
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS spinal_math spinal_flight_controller
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()
