# ROS1 to ROS2 Humble Conversion Guide

> Note
> This document is reference-only (API patterns and migration examples).
> For current project status, completed work, and next actionable steps, use `ROS2_IMPLEMENTATION_GUIDE.md` as the source of truth.

This document provides comprehensive guidance for converting the JSK Aerial Robot workspace from ROS1 to ROS2 Humble.

## Overview of Changes

### 1. Package Metadata (package.xml)

**ROS1:**
```xml
<?xml version="1.0"?>
<package>
  <buildtool_depend>catkin</buildtool_depend>
  <build_depend>roscpp</build_depend>
  <run_depend>roscpp</run_depend>
</package>
```

**ROS2:**
```xml
<?xml version="1.0"?>
<package format="3">
  <buildtool_depend>ament_cmake</buildtool_depend>
  <build_depend>rclcpp</build_depend>
  <exec_depend>rclcpp</exec_depend>
</package>
```

**Key Changes:**
- Add `format="3"` attribute to `<package>` tag
- Replace all `catkin` references with `ament_cmake`
- Replace `roscpp` with `rclcpp` (ROS2 C++ client library)
- Replace `rospy` with `rclpy` (ROS2 Python client library)
- Replace `build_depend`/`run_depend` with `build_depend`/`exec_depend`/`test_depend`
- For msg/srv packages: add `<member_of_group>rosidl_interface_packages</member_of_group>`
- For msg/srv packages: change `message_generation` to `rosidl_default_generators` (build) and `message_runtime` to `rosidl_default_runtime` (exec)

### 2. Build Configuration (CMakeLists.txt)

**ROS1 Pattern:**
```cmake
cmake_minimum_required(VERSION 3.0.2)
project(my_package)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  message_generation
)

add_message_files(FILES MyMessage.msg)
generate_messages(DEPENDENCIES std_msgs)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES my_library
  CATKIN_DEPENDS roscpp std_msgs message_runtime
)

add_library(my_library src/lib.cpp)
target_link_libraries(my_library ${catkin_LIBRARIES})
```

**ROS2 Pattern:**
```cmake
cmake_minimum_required(VERSION 3.8)
project(my_package)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MyMessage.msg"
  DEPENDENCIES std_msgs
)

add_library(my_library src/lib.cpp)
target_include_directories(my_library PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(my_library
  rclcpp
  std_msgs
)

ament_export_dependencies(rosidl_runtime_cpp std_msgs)

ament_package()
```

### 3. C++ Code Changes

#### Node Initialization

**ROS1:**
```cpp
#include <ros/ros.h>

int main(int argc, char** argv)
{
  ros::init(argc, argv, "my_node");
  ros::NodeHandle nh;
  ros::NodeHandle nh_private("~");
  
  double param;
  nh_private.param("my_param", param, 0.0);
  
  // ...
  
  ros::spin();
  return 0;
}
```

**ROS2:**
```cpp
#include <rclcpp/rclcpp.hpp>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("my_node");
  
  auto param = node->declare_parameter("my_param", 0.0);
  
  // ...
  
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
```

#### Publishers and Subscribers

**ROS1:**
```cpp
ros::Publisher pub = nh.advertise<std_msgs::Float64>("topic", 10);
ros::Subscriber sub = nh.subscribe("topic", 10, &callback);

std_msgs::Float64 msg;
msg.data = 1.5;
pub.publish(msg);
```

**ROS2:**
```cpp
auto pub = node->create_publisher<std_msgs::msg::Float64>("topic", 10);
auto sub = node->create_subscription<std_msgs::msg::Float64>(
  "topic", 10, 
  std::bind(&callback, this, std::placeholders::_1)
);

auto msg = std::make_shared<std_msgs::msg::Float64>();
msg->data = 1.5;
pub->publish(*msg);
```

#### Services

**ROS1:**
```cpp
ros::ServiceServer server = nh.advertiseService("service_name", &callback);
ros::ServiceClient client = nh.serviceClient<MyService>("service_name");

MyService srv;
client.call(srv);
```

**ROS2:**
```cpp
auto server = node->create_service<MyService>(
  "service_name",
  std::bind(&callback, this, std::placeholders::_1, std::placeholders::_2)
);

auto client = node->create_client<MyService>("service_name");
auto request = std::make_shared<MyService::Request>();
auto future = client->async_send_request(request);
```

#### Timers

**ROS1:**
```cpp
ros::TimerOptions ops(ros::Duration(0.1),
                      boost::bind(&MyClass::callback, this, _1));
timer_ = nh.createTimer(ops);
timer_.stop();
timer_.start();
```

**ROS2:**
```cpp
timer_ = node->create_wall_timer(
  std::chrono::milliseconds(100),
  std::bind(&MyClass::callback, this)
);
// No explicit stop/start, use timer destruction or separate enable/disable logic
```

#### Logging

**ROS1:**
```cpp
ROS_INFO("Message: %s", msg.c_str());
ROS_WARN("Warning: %d", value);
ROS_ERROR("Error occurred");
```

**ROS2:**
```cpp
RCLCPP_INFO(node->get_logger(), "Message: %s", msg.c_str());
RCLCPP_WARN(node->get_logger(), "Warning: %d", value);
RCLCPP_ERROR(node->get_logger(), "Error occurred");
```

#### Dynamic Parameters (replacing dynamic_reconfigure)

**ROS1 (dynamic_reconfigure):**
```cpp
#include <dynamic_reconfigure/server.h>
#include <my_package/MyConfig.h>

dynamic_reconfigure::Server<my_package::MyConfig> server;
dynamic_reconfigure::Server<my_package::MyConfig>::CallbackType f;
f = boost::bind(&callback, _1, _2);
server.setCallback(f);
```

**ROS2 (parameters):**
```cpp
auto param_callback = [this](const rcl_interfaces::msg::ParameterEvent &event) {
  for (auto & changed_param : event.changed_parameters) {
    if (changed_param.name == "my_param") {
      // Handle param change
    }
  }
};
param_subscription_ = node->create_subscription<rcl_interfaces::msg::ParameterEvent>(
  "/parameter_events", 
  rclcpp::QoS(rclcpp::KeepLast(1)).transient_local(),
  param_callback
);
```

Or use parameter client:
```cpp
node->declare_parameter("my_param", 1.0);
auto& param = node->get_parameter("my_param");
```

#### AsyncSpinner (Callback threads)

**ROS1:**
```cpp
ros::AsyncSpinner spinner(4); // 4 threads
spinner.start();
ros::waitForShutdown();
spinner.stop();
```

**ROS2:**
```cpp
rclcpp::executors::MultiThreadedExecutor executor(
  rclcpp::ExecutorOptions(), 4 // 4 threads
);
executor.add_node(node);
executor.spin();
```

### 4. Boost to Standard Library Conversions

Replace `boost::shared_ptr` with `std::shared_ptr`:
```cpp
// ROS1
boost::shared_ptr<MyClass> obj = boost::make_shared<MyClass>();

// ROS2
std::shared_ptr<MyClass> obj = std::make_shared<MyClass>();
```

Replace `boost::bind` with `std::bind`:
```cpp
// ROS1
boost::bind(&MyClass::method, this, _1)

// ROS2
std::bind(&MyClass::method, this, std::placeholders::_1)
```

### 5. Plugin System Migration

**PluginLib remains compatible in ROS2**, but package.xml needs updating:

**ROS1:**
```xml
<build_depend>pluginlib</build_depend>
<run_depend>pluginlib</run_depend>
<export>
  <aerial_robot_control plugin="${prefix}/plugins/control_plugins.xml"/>
</export>
```

**ROS2:**
```xml
<build_depend>pluginlib</build_depend>
<exec_depend>pluginlib</exec_depend>
<export>
  <aerial_robot_control plugin="${prefix}/plugins/control_plugins.xml"/>
</export>
```

Plugin XML files (.xml) remain mostly the same.

### 6. Messages and Services

**Message Files (.msg) are compatible** - mostly no changes needed except:
- ROS2 message field names must be lowercase with underscores (not camelCase)
- No changes needed if already following convention

**Service Files (.srv) are compatible** - no changes needed

### 7. Launch Files

**ROS1:**
```xml
<launch>
  <node pkg="my_package" type="my_node" name="my_node" output="screen">
    <param name="param_name" value="1.0"/>
  </node>
</launch>
```

**ROS2 (Python):**
```python
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
  return LaunchDescription([
    Node(
      package='my_package',
      executable='my_node',
      name='my_node',
      output='screen',
      parameters=[
        {'param_name': 1.0}
      ]
    )
  ])
```

## Conversion Checklist

- [ ] Convert all `package.xml` files
- [ ] Convert all `CMakeLists.txt` files
- [ ] Update C++ includes (`ros/ros.h` → `rclcpp/rclcpp.hpp`)
- [ ] Update node initialization patterns
- [ ] Update publishers/subscribers
- [ ] Update services/clients
- [ ] Update timers
- [ ] Update logging calls
- [ ] Replace dynamic_reconfigure with parameters
- [ ] Update AsyncSpinner patterns
- [ ] Replace boost with std equivalents
- [ ] Update Python scripts (rospy → rclpy)
- [ ] Convert launch files (XML → Python)
- [ ] Test plugin loading with pluginlib
- [ ] Update build/run commands

## Testing

After conversion, test with:
```bash
colcon build
colcon test
```

## Known Issues and Workarounds

### Issue: `message_generation` not found
**Solution:** Use `rosidl_default_generators` instead

### Issue: Services not generating correctly
**Solution:** Ensure service files are listed in `rosidl_generate_interfaces()`

### Issue: Plugin loading fails
**Solution:** Verify plugin XML paths and ensure package exports are correct

### Issue: Parameter callbacks not triggered
**Solution:** Subscribe to `/parameter_events` topic properly or use parameter client

## References

- [ROS2 Migration Guide](https://docs.ros.org/en/humble/Contributing/Migration-Guide.html)
- [ROS2 API Documentation](https://docs.ros.org/en/humble/index.html)
- [colcon Documentation](https://colcon.readthedocs.io/)
