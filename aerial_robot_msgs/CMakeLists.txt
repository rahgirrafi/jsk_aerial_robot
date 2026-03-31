cmake_minimum_required(VERSION 3.8)
project(aerial_robot_msgs)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Message files
set(msg_files
  "msg/DynamicReconfigureLevels.msg"
  "msg/FourAxisGain.msg"
  "msg/AerialRobotStatus.msg"
  "msg/FlightNav.msg"
  "msg/PoseControlPid.msg"
  "msg/Pid.msg"
  "msg/State.msg"
  "msg/States.msg"
  "msg/Acc.msg"
  "msg/WrenchAllocationMatrix.msg"
  "msg/ApplyWrench.msg"
  "msg/ForceList.msg"
)

# Service files
set(srv_files
  "srv/AddExtraModule.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES std_msgs geometry_msgs
)

ament_package()



