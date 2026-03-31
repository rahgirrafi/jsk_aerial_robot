# ROS2 Humble Conversion - Complete Implementation Guide

This document reflects the current migration status for `/home/rahgirrafi/jsk_aerial_robot`.

## Current Status

### Completed
- Package metadata conversion (`package.xml`) across workspace packages
- Package-level `CMakeLists.txt` conversion to ROS2 `ament_cmake`
- Removal of ROS1 `catkin` macros/variables from package-level CMake files
- `aerial_robot_base` C++ sources migrated to ROS2-style node/timer/executor patterns
- `aerial_robot_base` Python modules/scripts migrated to ROS2 runtime (`rclpy`) via a compatibility layer

### Verified
- No remaining package-level CMake usage of:
  - `find_package(catkin ...)`
  - `catkin_package(...)`
  - `catkin_metapackage()`
  - `CATKIN_PACKAGE_*`
  - `CATKIN_ENABLE_TESTING`

## Updated Conversion Scope

The CMake conversion phase is complete. Remaining work is now runtime/API migration:

1. C++ source migration (`roscpp` API to `rclcpp`)
2. Python node/script migration (`rospy` API to `rclpy`) for remaining packages
3. Launch migration (ROS1 XML launch to ROS2 Python launch)
4. Parameter migration (dynamic_reconfigure style to ROS2 parameters)
5. Build and test stabilization

## 1. C++ Source Migration

### Priority Files
1. `aerial_robot_estimation/src/state_estimation.cpp`
2. `aerial_robot_model/src/model/base_model/robot_model_ros.cpp`
3. `aerial_robot_model/src/servo_bridge/servo_bridge_node.cpp`
4. `aerial_robot_control/include/aerial_robot_control/control/base/base.h`
5. `aerial_robot_control/include/aerial_robot_control/flight_navigation.h`

Already migrated in this phase:
- `aerial_robot_base/src/aerial_robot_base_node.cpp`
- `aerial_robot_base/src/aerial_robot_base.cpp`
- `aerial_robot_base/include/aerial_robot_base/aerial_robot_base.h`

### API Replacements
- `#include <ros/ros.h>` -> `#include <rclcpp/rclcpp.hpp>`
- `ros::init(...)` -> `rclcpp::init(...)`
- `ros::NodeHandle` -> `std::shared_ptr<rclcpp::Node>`
- `ros::Publisher`/`ros::Subscriber` -> ROS2 publisher/subscription APIs
- `ros::ServiceServer`/`ros::ServiceClient` -> ROS2 service/client APIs
- `ROS_INFO/WARN/ERROR` -> `RCLCPP_INFO/WARN/ERROR`
- `boost::shared_ptr` -> `std::shared_ptr`
- `boost::bind` -> `std::bind`

### Example: Node Entrypoint

```cpp
#include <rclcpp/rclcpp.hpp>
#include <aerial_robot_base/aerial_robot_base.h>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<rclcpp::Node>("aerial_robot_base");
  auto aerial_robot_base = std::make_shared<AerialRobotBase>(node);
  (void)aerial_robot_base;

  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}
```

## 2. Python Migration

### Targets
- `aerial_robot_control/scripts/*.py`
- any remaining package scripts using `import rospy`

### Completed in `aerial_robot_base`
- `scripts/keyboard_command.py`
- `scripts/simple_demo.py`
- `scripts/rms.py`
- `src/aerial_robot_base/robot_interface.py`
- `src/aerial_robot_base/state_machine.py`
- `src/aerial_robot_base/hovering_check.py`
- `setup.py` migrated from catkin helper to setuptools

### Migration Notes
- Added `src/aerial_robot_base/ros_compat.py` to provide ROS2-backed wrappers (`init_node`, params, pub/sub, service calls, logging, timing) used by legacy logic.
- Replaced ROS1 master introspection in Python with ROS2 graph queries (`get_topic_names_and_types`).

### API Replacements
- `import rospy` -> `import rclpy`
- `rospy.init_node` -> `rclpy.init` + `Node` class
- `rospy.Publisher` -> `self.create_publisher`
- `rospy.Subscriber` -> `self.create_subscription`
- `rospy.Rate` loop -> timer callback (`self.create_timer`)

## 3. Launch Migration

### Targets
- Convert `.launch` XML files to Python launch files (`*.launch.py`)

### Pattern
- Use `launch.LaunchDescription`
- Use `launch_ros.actions.Node`
- Move `<param>` to `parameters=[{...}]`

## 4. Parameter Migration

Dynamic reconfigure patterns should be replaced with ROS2 parameters:

```cpp
node->declare_parameter("pid_kp", 1.0);
const auto kp = node->get_parameter("pid_kp").as_double();
```

For runtime updates, use parameter callbacks or parameter event subscriptions.

## 5. Build and Validation

Run these in order:

```bash
cd /home/rahgirrafi/jsk_aerial_robot
source /opt/ros/humble/setup.bash

# Install dependencies
rosdep install --from-paths . --ignore-src -r -y

# Build incrementally
colcon build --packages-select aerial_robot_msgs
colcon build --packages-select spinal
colcon build --packages-select aerial_robot_model
colcon build --packages-select aerial_robot_estimation
colcon build --packages-select aerial_robot_control
colcon build --packages-select aerial_robot_base

# Full build
colcon build --symlink-install --packages-skip-regex "gimbalrotor"
```

## Troubleshooting Notes

| Issue | Likely Cause | Action |
|---|---|---|
| `ros/ros.h not found` | Unmigrated C++ source | Replace with `rclcpp` includes and APIs |
| `rospy` import error | Unmigrated Python script | Port script to `rclpy` |
| unresolved symbol to ROS1 API | Mixed ROS1/ROS2 code in target | Complete API migration for that target |
| signature mismatch in `initialize(...)` calls | dependent package headers still use ROS1 `NodeHandle`/`boost` signatures | migrate corresponding control/estimation/model headers and methods to ROS2 API |
| plugin load fails | plugin XML/export mismatch | Verify plugin export path and target names |

## Summary

### Progress
- `package.xml`: complete
- package-level `CMakeLists.txt`: complete
- C++ API migration: in progress (`aerial_robot_base` done)
- Python migration: in progress (`aerial_robot_base` done)
- launch migration: pending
- integration tests: pending

### Immediate Next Step
Migrate C++ APIs in `aerial_robot_model`, `aerial_robot_estimation`, and `aerial_robot_control` to match the new ROS2 signatures already introduced in `aerial_robot_base`.

# ROS2 Migration Notes

## CMake Target and Dependency Migration

1. `rosidl_generate_interfaces(${PROJECT_NAME} ...)` creates a custom target with the same name as the project. If you also use `add_library(${PROJECT_NAME} ...)`, this causes a CMake target collision. **Solution:** Use a distinct name for the rosidl target, e.g. `rosidl_generate_interfaces(${PROJECT_NAME}_msgs ...)`.

2. `ament_target_dependencies(robot_model_pluginlib ... aerial_robot_model ...)` is incorrect because `aerial_robot_model` is a library target, not an ament package. **Solution:** Only ament/ROS packages go in `ament_target_dependencies()`. Link the local library with `target_link_libraries(robot_model_pluginlib aerial_robot_model)` instead.

3. Similarly, for `numerical_jacobians`, do not list `aerial_robot_model` in `ament_target_dependencies()`. Instead, link it with `target_link_libraries(numerical_jacobians aerial_robot_model)`.
4. Commented out spinal pkg in aerial_robot_model

These changes are required for correct ROS2/Humble CMake migration and to avoid build errors.