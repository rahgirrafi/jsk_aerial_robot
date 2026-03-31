cmake_minimum_required(VERSION 3.8)

project(spinal_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

# Message files
set(msg_files
  "msg/Barometer.msg"
  "msg/BoardInfo.msg"
  "msg/DesireCoord.msg"
  "msg/ESCTelemetry.msg"
  "msg/ESCTelemetryArray.msg"
  "msg/FlightConfigCmd.msg"
  "msg/FourAxisCommand.msg"
  "msg/Gps.msg"
  "msg/GpsFull.msg"
  "msg/Gyro.msg"
  "msg/Imu.msg"
  "msg/JointProfile.msg"
  "msg/JointProfiles.msg"
  "msg/MotorInfo.msg"
  "msg/PMatrixPseudoInverseUnit.msg"
  "msg/PMatrixPseudoInverseWithInertia.msg"
  "msg/PwmInfo.msg"
  "msg/PwmTest.msg"
  "msg/Pwms.msg"
  "msg/RollPitchYawTerm.msg"
  "msg/RollPitchYawTerms.msg"
  "msg/ServoControlCmd.msg"
  "msg/ServoInfo.msg"
  "msg/ServoState.msg"
  "msg/ServoStates.msg"
  "msg/ServoTorqueCmd.msg"
  "msg/ServoTorqueStates.msg"
  "msg/SimpleImu.msg"
  "msg/TorqueAllocationMatrixInv.msg"
  "msg/UavInfo.msg"
  "msg/Vector3Int16.msg"
)

# Service files
set(srv_files
  "srv/GetBoardInfo.srv"
  "srv/ImuCalib.srv"
  "srv/MagDeclination.srv"
  "srv/SetAttitudeGains.srv"
  "srv/SetBoardConfig.srv"
  "srv/SetDirectServoConfig.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES builtin_interfaces
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
