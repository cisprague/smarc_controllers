//Node to mix roll compensation and velocity controller by sending RPM commands to the duoprop
//Sriharsha Bhat, 13.11.2019

#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <smarc_msgs/ThrusterRPM.h>
#include <std_msgs/Bool.h>

smarc_msgs::ThrusterRPM rpm1, rpm2;
//std_msgs::Float64 control_action;
double prev_control_msg1,prev_control_msg2,limit,freq, mean_prop_rpm, rpm_diff;
bool message_received, enable_state;
std::string topic_from_roll_controller_,topic_from_vel_controller_, topic_to_actuator_1_, topic_to_actuator_2_, pid_enable_topic_;


void PIDCallback1(const std_msgs::Float64& control_msg)
{
  //if(abs(prev_control_msg1-control_msg.data) > limit) {
    rpm_diff = control_msg.data;
    message_received = true;
    //}
    ROS_INFO_THROTTLE(1.0, "[ pid_actuator_prop ] RPM Difference : %f", control_msg.data);
}

void PIDCallback2(const std_msgs::Float64& control_msg)
{
   //if(abs(prev_control_msg2-control_msg.data) > limit) {
    message_received = true;
    mean_prop_rpm = control_msg.data;
    //}
ROS_INFO_THROTTLE(1.0, "[ pid_actuator_prop ]  Mean RPM: %f", control_msg.data);
}

void enableCB(const std_msgs::Bool enable_msg){
	if(enable_msg.data == false)
  {
    enable_state = false;
  }
  else enable_state = true;
}


int main(int argc, char** argv){
  ros::init(argc, argv, "pid_actuator_prop");

  ros::NodeHandle node;
  message_received = false;

  ros::NodeHandle node_priv("~");
  node_priv.param<std::string>("topic_from_roll_controller", topic_from_roll_controller_, "roll_prop_diff");
  node_priv.param<std::string>("topic_from_vel_controller", topic_from_vel_controller_, "mean_prop_rpm");
  node_priv.param<std::string>("topic_to_actuator_1", topic_to_actuator_1_, "uavcan_prop_command");
  node_priv.param<std::string>("topic_to_actuator_2", topic_to_actuator_2_, "uavcan_prop_command");
  node_priv.param<std::string>("pid_enable_topic", pid_enable_topic_, "pid_enable");
  node_priv.param<double>("limit_between_setpoints", limit, 5);
  node_priv.param<double>("loop_freq", freq, 50);
  //initiate subscribers
  ros::Subscriber pid_action_sub_prop1 = node.subscribe(topic_from_roll_controller_, 1, PIDCallback1);
  ros::Subscriber pid_action_sub_prop2 = node.subscribe(topic_from_vel_controller_, 1, PIDCallback2);
  ros::Subscriber enable_sub = node.subscribe(pid_enable_topic_, 10, enableCB);

  //initiate publishers
  ros::Publisher rpm1_pub = node.advertise<smarc_msgs::ThrusterRPM>(topic_to_actuator_1_, freq);
  ros::Publisher rpm2_pub = node.advertise<smarc_msgs::ThrusterRPM>(topic_to_actuator_2_, freq);
  //ros::Publisher control_action_pub = node.advertise<std_msgs::Float64>(topic_to_actuator_, 10);
  enable_state = true;

  ros::Rate rate(freq);

  while (node.ok()){

    if (message_received && enable_state) {
      rpm1.rpm = (mean_prop_rpm + 0.5*rpm_diff)*100;
      rpm2.rpm = (mean_prop_rpm - 0.5*rpm_diff)*100;
      rpm1_pub.publish(rpm1);
      rpm2_pub.publish(rpm2);
    }

    //prev_control_msg1 = control_action.thruster_front;
    //prev_control_msg2 = control_action.thruster_back;

    //prev_control_msg = control_action.data;
    ROS_INFO_THROTTLE(1.0, "[ pid_actuator ]  Control sent: Prop1:%i Prop2:%i", rpm1.rpm,rpm2.rpm);
  //  ROS_INFO_THROTTLE(1.0, "[ pid_actuator ]  Control forwarded: %f", control_action.data); //Gazebo

    rate.sleep();
    ros::spinOnce();

  }
  return 0;
};
