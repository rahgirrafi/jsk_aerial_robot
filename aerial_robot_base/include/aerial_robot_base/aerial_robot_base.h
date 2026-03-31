#pragma once

#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>

namespace aerial_robot_control
{
class ControlBase;
}

namespace aerial_robot_navigation
{
class BaseNavigator;
}

namespace aerial_robot_estimation
{
class StateEstimator;
}

namespace aerial_robot_model
{
class RobotModelRos;
}

class AerialRobotBase
{
 public:
  explicit AerialRobotBase(const rclcpp::Node::SharedPtr& node);
  ~AerialRobotBase() = default;

  void mainFunc();

 private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr main_loop_group_;
  rclcpp::TimerBase::SharedPtr main_timer_;

  std::shared_ptr<aerial_robot_model::RobotModelRos> robot_model_ros_;
  std::shared_ptr<aerial_robot_estimation::StateEstimator> estimator_;

  pluginlib::ClassLoader<aerial_robot_navigation::BaseNavigator> navigator_loader_;
  std::shared_ptr<aerial_robot_navigation::BaseNavigator> navigator_;

  pluginlib::ClassLoader<aerial_robot_control::ControlBase> controller_loader_;
  std::shared_ptr<aerial_robot_control::ControlBase> controller_;
};
