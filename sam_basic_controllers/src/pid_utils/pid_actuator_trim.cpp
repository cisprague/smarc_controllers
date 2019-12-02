//Node to forward PID control action to UAVCAN by publishing to sam_msgs.
//Sriharsha Bhat, 05.12.2018

#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <sam_msgs/PercentStamped.h>

sam_msgs::PercentStamped control_action;
double prev_control_msg,limit, freq;
std::string topic_from_controller_, topic_to_actuator_;


void PIDCallback(const std_msgs::Float64& control_msg)
{
  if(abs(prev_control_msg-control_msg.data) > limit) {
    control_action.value = control_msg.data + 50.;//transforms.transform.rotation.x;//data;
    }
ROS_INFO_THROTTLE(1.0, "[ pid_actuator ]  Control action heard: %f", control_msg.data);
}

int main(int argc, char** argv){
  ros::init(argc, argv, "pid_actuator");

  ros::NodeHandle node;

  ros::NodeHandle node_priv("~");
  node_priv.param<std::string>("topic_from_controller", topic_from_controller_, "control_action");
  node_priv.param<std::string>("topic_to_actuator", topic_to_actuator_, "uavcan_lcg_command");
  node_priv.param<double>("limit_between_setpoints", limit, 5);
  node_priv.param<double>("loop_freq", freq, 50);

  //initiate subscribers
  ros::Subscriber pid_action_sub = node.subscribe(topic_from_controller_, 1, PIDCallback);

  //initiate publishers
  ros::Publisher control_action_pub = node.advertise<sam_msgs::PercentStamped>(topic_to_actuator_, freq);

  ros::Rate rate(freq);

  while (node.ok()){

    control_action_pub.publish(control_action);
    prev_control_msg = control_action.value;
    ROS_INFO_THROTTLE(1.0, "[ pid_actuator ]  Control forwarded: %f", control_action.value);

    rate.sleep();
    ros::spinOnce();

  }
  return 0;
};