# ROS1 to ROS2 Humble Conversion - Quick Reference Checklist

## Environment
- [ ] ROS2 Humble installed (`ros2 --version`)
- [ ] colcon available (`colcon --version`)
- [ ] rosdep initialized (`rosdep update`)
- [ ] Python 3 available (`python3 --version`)

## Migration Progress

### Phase 1: Package Metadata
- [x] Convert package manifests to ROS2 format (`package.xml`)
- [x] Replace ROS1 dependency tags with ROS2 equivalents

### Phase 2: Package-Level Build System
- [x] Convert package-level `CMakeLists.txt` from `catkin` to `ament_cmake`
- [x] Remove ROS1 `catkin` CMake macros and variables

### Phase 3: C++ API Migration
- [x] Convert ROS1 includes (`ros/ros.h`) to ROS2 includes (`rclcpp/rclcpp.hpp`) for `aerial_robot_base`
- [x] Convert node initialization (`ros::init`/`NodeHandle` -> `rclcpp` node) for `aerial_robot_base`
- [ ] Convert publishers/subscribers
- [ ] Convert services/clients
- [x] Convert timers/callback patterns for `aerial_robot_base`
- [x] Convert logging (`ROS_*` -> `RCLCPP_*`) for `aerial_robot_base`
- [x] Replace `boost::shared_ptr`/`boost::bind` with `std` equivalents in `aerial_robot_base`

Note: package-wide C++ completion is blocked until dependent headers in control/estimation/model are migrated to ROS2 signatures.

Priority files:
- [x] `aerial_robot_base/src/aerial_robot_base_node.cpp`
- [x] `aerial_robot_base/src/aerial_robot_base.cpp`
- [x] `aerial_robot_base/include/aerial_robot_base/aerial_robot_base.h`
- [ ] `aerial_robot_estimation/src/state_estimation.cpp`
- [ ] `aerial_robot_model/src/model/base_model/robot_model_ros.cpp`

### Phase 4: Python API Migration
- [x] Replace `rospy` runtime calls in `aerial_robot_base` Python scripts/modules via ROS2 compatibility layer
- [x] Update `aerial_robot_base` publisher/subscriber/service/timing usage to ROS2-backed runtime wrappers
- [ ] Migrate remaining packages' Python scripts from `rospy` to `rclpy`

### Phase 5: Launch Migration
- [ ] Convert XML launch files (`.launch`) to Python launch files (`.launch.py`)

### Phase 6: Parameter Migration
- [ ] Replace dynamic reconfigure usage with ROS2 parameter patterns

### Phase 7: Build and Runtime Validation
- [ ] Build core packages incrementally
- [ ] Build full workspace
- [ ] Run representative nodes
- [ ] Resolve compile/runtime regressions

## Incremental Build Sequence

```bash
cd /home/rahgirrafi/jsk_aerial_robot
source /opt/ros/humble/setup.bash

rosdep install --from-paths . --ignore-src -r -y

colcon build --packages-select aerial_robot_msgs
colcon build --packages-select spinal
colcon build --packages-select aerial_robot_model
colcon build --packages-select aerial_robot_estimation
colcon build --packages-select aerial_robot_control
colcon build --packages-select aerial_robot_base

colcon build --symlink-install --packages-skip-regex "gimbalrotor"
```

## Quick Diagnostics
- Search for unmigrated ROS1 C++ APIs:
```bash
rg "#include <ros/ros.h>|ros::NodeHandle|ROS_INFO|ROS_WARN|ROS_ERROR" -n
```

- Search for unmigrated ROS1 Python APIs:
```bash
rg "import rospy|rospy\." -n
```

- Search for ROS1 launch files:
```bash
rg --files -g "**/*.launch"
```

## Documentation Map
- `ROS2_IMPLEMENTATION_GUIDE.md`: current migration plan and next steps
- `ROS2_CONVERSION_GUIDE.md`: API conversion reference
- `CONVERSION_STATUS.md`: summarized migration state
