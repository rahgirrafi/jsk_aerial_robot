#include <aerial_robot_base/aerial_robot_base.h>

#include <aerial_robot_control/control/base/base.h>
#include <aerial_robot_control/flight_navigation.h>
#include <aerial_robot_estimation/state_estimation.h>
#include <aerial_robot_model/model/aerial_robot_model_ros.h>

#include <algorithm>
#include <chrono>
#include <functional>

AerialRobotBase::AerialRobotBase(const rclcpp::Node::SharedPtr& node)
  : node_(node),
    main_loop_group_(node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive)),
    controller_loader_("aerial_robot_control", "aerial_robot_control::ControlBase"),
    navigator_loader_("aerial_robot_control", "aerial_robot_navigation::BaseNavigator")
{

  const bool param_verbose = node_->declare_parameter<bool>("param_verbose", true);
  const double main_rate = node_->declare_parameter<double>("main_rate", 0.0);

  // robot model
  // TODO: dependent packages still need full ROS2 API migration.
  robot_model_ros_ = std::make_shared<aerial_robot_model::RobotModelRos>(node_, node_);
  auto robot_model = robot_model_ros_->getRobotModel();

  // estimator
  estimator_ = std::make_shared<aerial_robot_estimation::StateEstimator>();
  estimator_->initialize(node_, node_, robot_model);

  // navigation
  std::string navi_plugin_name;
  if(node_->get_parameter("flight_navigation_plugin_name", navi_plugin_name))
    {
      try
        {
          navigator_ = navigator_loader_.createSharedInstance(navi_plugin_name);
        }
      catch (const pluginlib::PluginlibException& ex)
        {
          RCLCPP_ERROR(node_->get_logger(), "Failed to load navigation plugin: %s", ex.what());
        }
    }
  else
    {
      RCLCPP_DEBUG(node_->get_logger(), "Use default class for flight navigation: aerial_robot_navigation::BaseNavigator");
      navigator_ = std::make_shared<aerial_robot_navigation::BaseNavigator>();
    }

  if (navigator_) navigator_->initialize(node_, node_, robot_model, estimator_, 1.0 / std::max(main_rate, 1e-6));

  //  controller
  try
    {
      const std::string aerial_robot_control_name = node_->declare_parameter<std::string>(
        "aerial_robot_control_name", "aerial_robot_control/flatness_pid");
      controller_ = controller_loader_.createSharedInstance(aerial_robot_control_name);
      if (controller_) controller_->initialize(node_, node_, robot_model, estimator_, navigator_, 1.0 / std::max(main_rate, 1e-6));
    }
  catch (const pluginlib::PluginlibException& ex)
    {
      RCLCPP_ERROR(node_->get_logger(), "Failed to load controller plugin: %s", ex.what());
    }

  if (param_verbose)
    {
      RCLCPP_INFO(node_->get_logger(), "%s: main_rate is %.3f", node_->get_fully_qualified_name(), main_rate);
    }

  if (main_rate <= 0)
    {
      RCLCPP_ERROR(node_->get_logger(), "main_rate is non-positive, cannot run the main timer");
    }
  else
    {
      main_timer_ = node_->create_wall_timer(
        std::chrono::duration<double>(1.0 / main_rate),
        std::bind(&AerialRobotBase::mainFunc, this),
        main_loop_group_);
    }
}

void AerialRobotBase::mainFunc()
{
  if (navigator_) navigator_->update();
  if (controller_) controller_->update();
}
