
#include <aerial_robot_model/model/aerial_robot_model_ros.h>

namespace aerial_robot_model {

  RobotModelRos::RobotModelRos(rclcpp::Node::SharedPtr node):
    node_(node),
    robot_model_loader_("aerial_robot_model", "aerial_robot_model::RobotModel")
  {
    // rosparam (tf_prefix)
    node_->declare_parameter<std::string>("tf_prefix", "");
    node_->get_parameter("tf_prefix", tf_prefix_);

    // tf2 broadcasters must be initialized with node
    br_ = std::make_shared<tf2_ros::TransformBroadcaster>(node_);
    static_br_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(node_);

    // load robot model plugin
    std::string plugin_name;
    if (node_->get_parameter("robot_model_plugin_name", plugin_name))
    {
      try
      {
        robot_model_ = robot_model_loader_.createSharedInstance(plugin_name);
      }
      catch(pluginlib::PluginlibException& ex)
      {
        RCLCPP_ERROR(node_->get_logger(), "The plugin failed to load for some reason. Error: %s", ex.what());
      }
    }
    else
    {
      RCLCPP_ERROR(node_->get_logger(), "can not find plugin parameter for robot model, use default class: aerial_robot_model::RobotModel");
      robot_model_ = std::make_shared<aerial_robot_model::RobotModel>();
    }

    if (robot_model_->isModelFixed()) {
      // broadcast static tf between root and CoG
      // refer: https://github.com/ros/robot_state_publisher/blob/rolling/src/robot_state_publisher.cpp#L130
      geometry_msgs::msg::TransformStamped tf = robot_model_->getCog<geometry_msgs::msg::TransformStamped>();
      tf.header.stamp = node_->now();
      tf.header.frame_id = tf_prefix_ + robot_model_->getRootFrameName();
      tf.child_frame_id = tf_prefix_ + std::string("cog");
      static_br_->sendTransform(tf);
    }
    else {
      joint_state_sub_ = node_->create_subscription<sensor_msgs::msg::JointState>(
        "joint_states", 10, std::bind(&RobotModelRos::jointStateCallback, this, std::placeholders::_1));
    }

    add_extra_module_service_ = node_->create_service<aerial_robot_msgs::srv::AddExtraModule>(
      "add_extra_module",
      std::bind(&RobotModelRos::addExtraModuleCallback, this, std::placeholders::_1, std::placeholders::_2));
  }


  void RobotModelRos::jointStateCallback(const sensor_msgs::msg::JointState::SharedPtr state)
  {
    joint_state_ = *state;
    robot_model_->updateRobotModel(*state);

    geometry_msgs::msg::TransformStamped tf = robot_model_->getCog<geometry_msgs::msg::TransformStamped>();
    tf.header = state->header;
    tf.header.frame_id = tf_prefix_ + robot_model_->getRootFrameName();
    tf.child_frame_id = tf_prefix_ + std::string("cog");
    br_->sendTransform(tf);
  }


  void RobotModelRos::addExtraModuleCallback(
    const aerial_robot_msgs::srv::AddExtraModule::Request::SharedPtr req,
    aerial_robot_msgs::srv::AddExtraModule::Response::SharedPtr res)
  {
    switch(req->action)
    {
      case aerial_robot_msgs::srv::AddExtraModule::Request::ADD:
      {
        geometry_msgs::msg::TransformStamped ts;
        ts.transform = req->transform;
        KDL::Frame f = tf2::transformToKDL(ts);
        KDL::RigidBodyInertia rigid_body_inertia(req->inertia.m, KDL::Vector(req->inertia.com.x, req->inertia.com.y, req->inertia.com.z),
                                                 KDL::RotationalInertia(req->inertia.ixx, req->inertia.iyy,
                                                                        req->inertia.izz, req->inertia.ixy,
                                                                        req->inertia.ixz, req->inertia.iyz));
        res->status = robot_model_->addExtraModule(req->module_name, req->parent_link_name, f, rigid_body_inertia);
        break;
      }
      case aerial_robot_msgs::srv::AddExtraModule::Request::REMOVE:
      {
        res->status = robot_model_->removeExtraModule(req->module_name);
        break;
      }
      default:
      {
        RCLCPP_WARN(node_->get_logger(), "[extra module]: wrong action %d", req->action);
        res->status = false;
        break;
      }
    }
  }
} //namespace aerial_robot_model
