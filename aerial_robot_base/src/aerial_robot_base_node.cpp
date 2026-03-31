#include "aerial_robot_base/aerial_robot_base.h"

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






