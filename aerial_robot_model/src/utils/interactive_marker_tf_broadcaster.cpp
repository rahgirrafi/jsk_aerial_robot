#include <rclcpp/rclcpp.hpp>
#include <interactive_markers/interactive_marker_server.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <visualization_msgs/msg/interactive_marker.hpp>
#include <visualization_msgs/msg/interactive_marker_control.hpp>
#include <visualization_msgs/msg/interactive_marker_feedback.hpp>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.h>
#include <tf2_ros/transform_broadcaster.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <memory>
#include <mutex>



class TfPublisher : public rclcpp::Node
{
public:
  TfPublisher() : Node("tf_publisher")
  {
    // Use the ROS2 InteractiveMarkerServer constructor: (topic, node shared_ptr)
    server_ = std::make_shared<interactive_markers::InteractiveMarkerServer>("tf_publisher", shared_from_this());

    this->declare_parameter<std::string>("target_frame", "root");
    this->declare_parameter<std::string>("reference_frame", "fixed_frame");
    this->declare_parameter<double>("tf_loop_rate", 60.0);

    this->get_parameter("target_frame", target_frame_);
    this->get_parameter("reference_frame", reference_frame_);
    this->get_parameter("tf_loop_rate", tf_loop_rate_);

    int_marker_init();

    target_pose_.setIdentity();
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(shared_from_this());

    timer_ = this->create_wall_timer(
      std::chrono::duration<double>(1.0 / tf_loop_rate_),
      [this]() { this->tf_publish(); });

    server_->applyChanges();
  }

private:
  std::shared_ptr<interactive_markers::InteractiveMarkerServer> server_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::mutex tf_mutex_;
  tf2::Transform target_pose_;
  double tf_loop_rate_;
  std::string target_frame_;
  std::string reference_frame_;
  rclcpp::TimerBase::SharedPtr timer_;

  void tf_process_feedback(const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr &feedback)
  {
    if (feedback->event_type == visualization_msgs::msg::InteractiveMarkerFeedback::POSE_UPDATE)
    {
      tf2::Quaternion rotation(feedback->pose.orientation.x, feedback->pose.orientation.y, feedback->pose.orientation.z, feedback->pose.orientation.w);
      tf2::Vector3 origin(feedback->pose.position.x, feedback->pose.position.y, feedback->pose.position.z);
      set_transform(origin, rotation);
    }
    server_->applyChanges();
  }

  void tf_publish()
  {
    tf2::Transform t = get_transform();
    geometry_msgs::msg::TransformStamped transform_msg;
    transform_msg.header.stamp = this->now();
    transform_msg.header.frame_id = reference_frame_;
    transform_msg.child_frame_id = target_frame_;
    transform_msg.transform.translation.x = t.getOrigin().x();
    transform_msg.transform.translation.y = t.getOrigin().y();
    transform_msg.transform.translation.z = t.getOrigin().z();
    transform_msg.transform.rotation.x = t.getRotation().x();
    transform_msg.transform.rotation.y = t.getRotation().y();
    transform_msg.transform.rotation.z = t.getRotation().z();
    transform_msg.transform.rotation.w = t.getRotation().w();
    tf_broadcaster_->sendTransform(transform_msg);
  }

  void set_transform(const tf2::Vector3 &origin, const tf2::Quaternion &rotation)
  {
    std::lock_guard<std::mutex> lock(tf_mutex_);
    target_pose_.setOrigin(origin);
    target_pose_.setRotation(rotation);
  }

  tf2::Transform get_transform()
  {
    std::lock_guard<std::mutex> lock(tf_mutex_);
    return target_pose_;
  }

  void int_marker_init()
  {
    visualization_msgs::msg::InteractiveMarker int_marker;
    visualization_msgs::msg::InteractiveMarkerControl rotate_control;

    int_marker.controls.clear();
    int_marker.header.frame_id = reference_frame_;
    int_marker.name = target_frame_ + std::string("_control");
    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 1;
    rotate_control.orientation.y = 0;
    rotate_control.orientation.z = 0;
    rotate_control.name = "rotate_x";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);
    rotate_control.name = "move_x";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::MOVE_AXIS;
    int_marker.controls.push_back(rotate_control);

    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 0;
    rotate_control.orientation.y = 1;
    rotate_control.orientation.z = 0;
    rotate_control.name = "rotate_z";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);
    rotate_control.name = "move_z";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::MOVE_AXIS;
    int_marker.controls.push_back(rotate_control);
    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 0;
    rotate_control.orientation.y = 0;
    rotate_control.orientation.z = 1;
    rotate_control.name = "rotate_y";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);
    rotate_control.name = "move_y";
    rotate_control.interaction_mode = visualization_msgs::msg::InteractiveMarkerControl::MOVE_AXIS;
    int_marker.controls.push_back(rotate_control);

    // Use lambda for feedback callback
    server_->insert(int_marker, [this](const visualization_msgs::msg::InteractiveMarkerFeedback::ConstSharedPtr &feedback) {
      this->tf_process_feedback(feedback);
    });
  }
};





int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  // Use enable_shared_from_this pattern for node
  auto node = std::make_shared<TfPublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}

