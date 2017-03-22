#include "squirrel_people_tracker/follow_child.h"
#include <actionlib/client/simple_action_client.h>
#include <string>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/tf.h>
#include <actionlib/client/terminal_state.h>


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

ChildFollowingAction::~ChildFollowingAction(void)
{
  if (move_base_ac_)
    delete move_base_ac_;
}

ChildFollowingAction::ChildFollowingAction(std::string name) : as_(nh_, name, false), action_name_(name)
{
  init_ = ros::Time::now();
  move_base_ac_ = new MoveBaseClient("move_base", true);
  while (!move_base_ac_->waitForServer(ros::Duration(5.0)))
  {
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  distance_ = 0.8;
  // register the goal and feeback callbacks
  as_.registerGoalCallback(boost::bind(&ChildFollowingAction::goalCB, this));
  as_.registerPreemptCallback(boost::bind(&ChildFollowingAction::preemptCB, this));

  // subscribe to the data topic of interest
  sub_ = nh_.subscribe("people_tracker_measurements", 1, &ChildFollowingAction::analysisCB, this);
  as_.start();

  // publishers
  pub_ = nh_.advertise<geometry_msgs::PoseStamped>("/published_topic", 1);
  vis_pub_ = nh_.advertise<visualization_msgs::Marker>( "visualization_marker", 0 );
}

void ChildFollowingAction::goalCB()
{
  goal_ = as_.acceptNewGoal();
}

void ChildFollowingAction::preemptCB()
{
  ROS_INFO("%s: Preempted", action_name_.c_str());
  // set the action state to preempted
  as_.setPreempted();
}

void ChildFollowingAction::analysisCB(const people_msgs::PositionMeasurementArray::ConstPtr &msg)
{
  ROS_INFO("Data received. processing ...");
  // make sure that the action hasn't been canceled
  if (!as_.isActive())
  {
    ROS_INFO("Action server is no longer active. Exiting.");
    return;
  }
  if (msg->people.size() == 0)
  {
    ROS_INFO("Exit. no people in message"); 
    return;
  }
  actionlib::SimpleClientGoalState state = move_base_ac_->getState();
  ROS_INFO("Action in state: %s",state.toString().c_str());
  

  geometry_msgs::PoseStamped robot_pose, child_pose, tmp_pose, out_pose;
  move_base_msgs::MoveBaseGoal move_base_goal_;
  double min_distance = 1000.0;
  int index = 0;
  double time_diff = (ros::Time::now() - init_).toSec();
  ROS_INFO("time diff: %f", time_diff);
  if (time_diff < 1.5)
  {
    return;
  }
  // calculate distance to select the closest personCB
  for (size_t i = 0; i < msg->people.size(); ++i)
  {
    double distance = (sqrt(msg->people[i].pos.x*msg->people[i].pos.x + msg->people[i].pos.y*msg->people[i].pos.y));
    if (distance < min_distance)
    {
      index = i;
      min_distance = distance;
    }
  }

  double alpha = 0.0;
  tmp_pose.header.stamp = ros::Time(0);
  tmp_pose.header.frame_id = "hokuyo_link";
  tmp_pose.pose.position.x = msg->people[index].pos.x;
  tmp_pose.pose.position.y = msg->people[index].pos.y;
  tmp_pose.pose.orientation =  tf::createQuaternionMsgFromYaw(alpha);
  try{
    ros::Time now = ros::Time(0);
    tfl_.waitForTransform("hokuyo_link", "map", now, ros::Duration(3.0));
    tfl_.transformPose("map", tmp_pose, child_pose);
  }
  catch (tf::TransformException ex){
    ROS_ERROR("%s",ex.what());
    ros::Duration(1.0).sleep();
    return;
  }
  ROS_INFO("Person detected at (x, y): (%f, %f) hokuyo_link", tmp_pose.pose.position.x, tmp_pose.pose.position.y);
  ROS_INFO("Person detected at (x, y): (%f, %f) map", child_pose.pose.position.x, child_pose.pose.position.y);

  // check if the child is in one of the target areas
  for (size_t i=0; i < goal_->target_locations.size(); ++i)
  {
    if ((fabs(child_pose.pose.position.x - goal_->target_locations[i].x) < 0.25) &&
        (fabs(child_pose.pose.position.y - goal_->target_locations[i].y) < 0.25))
    {
      // make sure we stop now
      ROS_INFO("%s: Succeeded", action_name_.c_str());
      result_.final_location = child_pose;
      as_.setSucceeded(result_);
    }
  }
  
  // calculate a point between the child and the robot
  alpha = atan2(tmp_pose.pose.position.y, tmp_pose.pose.position.x);
  double k = sqrt(tmp_pose.pose.position.x * tmp_pose.pose.position.x + tmp_pose.pose.position.y * tmp_pose.pose.position.y);
  ROS_DEBUG("k: %f, alpha: %f", k, alpha);
  ROS_DEBUG("sin(alpha): %f, cos(alpha): %f", sin(alpha), cos(alpha));
  
  tmp_pose.header.stamp = ros::Time(0);
  tmp_pose.header.frame_id = "hokuyo_link";
  tmp_pose.pose.position.x = (k - distance_)*cos(alpha);
  tmp_pose.pose.position.y = (k - distance_)*sin(alpha);
  tmp_pose.pose.orientation = tf::createQuaternionMsgFromYaw(alpha);
  
  pub_.publish(tmp_pose);
  try{
    ros::Time now = ros::Time(0);
    tfl_.waitForTransform("hokuyo_link", "map",
                            now, ros::Duration(3.0));
    tfl_.transformPose("map", tmp_pose, out_pose);
  }
  catch (tf::TransformException ex){
    ROS_ERROR("%s",ex.what());
    ros::Duration(1.0).sleep();
    return;
  }
  if (fabs(last_goal_.position.x - out_pose.pose.position.x) < 0.25 && 
      fabs(last_goal_.position.y - out_pose.pose.position.y) < 0.25 &&
      fabs(tf::getYaw(last_goal_.orientation) - tf::getYaw(out_pose.pose.orientation) < 0.26)) //~15 degree
  {
  ROS_INFO("Last goal was (x, y): (%f, %f) map", last_goal_.position.x, last_goal_.position.y);
  ROS_INFO("Current nav goal would be (x, y): (%f, %f) map", out_pose.pose.position.x, out_pose.pose.position.y);
    ROS_INFO("current goal and last goal are close to each other. Do not send new goal");
    return;
  }

  publishGoalMarker(out_pose.pose.position.x, out_pose.pose.position.y, out_pose.pose.position.z);
  ROS_INFO("Setting nav goal to (x, y): (%f, %f) hokuyo_link", tmp_pose.pose.position.x, tmp_pose.pose.position.y);
  ROS_INFO("Setting nav goal to (x, y): (%f, %f) map", out_pose.pose.position.x, out_pose.pose.position.y);

  last_goal_.position = out_pose.pose.position;
  last_goal_.orientation = out_pose.pose.orientation;

  move_base_goal_.target_pose.header.frame_id = out_pose.header.frame_id;
  move_base_goal_.target_pose.header.stamp = out_pose.header.stamp;
  move_base_goal_.target_pose.pose.position.x = out_pose.pose.position.x;
  move_base_goal_.target_pose.pose.position.y = out_pose.pose.position.y;
  move_base_goal_.target_pose.pose.orientation = out_pose.pose.orientation;

  ROS_INFO("Sending goal");
  move_base_ac_->sendGoal(move_base_goal_);
  init_ = ros::Time::now();
  ros::Duration(0.1).sleep();
}

void ChildFollowingAction::publishGoalMarker(float x, float y, float z)
{
  visualization_msgs::Marker marker;
  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time(0);
  marker.ns = "goal";
  marker.id = 0;
  marker.type = visualization_msgs::Marker::SPHERE;
  marker.action = visualization_msgs::Marker::ADD;
  marker.pose.position.x = x;
  marker.pose.position.y = y;
  marker.pose.position.z = z;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;
  marker.scale.x = 0.2;
  marker.scale.y = 0.2;
  marker.scale.z = 0.2;
  marker.color.a = 1.0; // Don't forget to set the alpha!
  marker.color.r = 0.0;
  marker.color.g = 1.0;
  marker.color.b = 0.0; 
  vis_pub_.publish(marker);
  ros::Duration(0.01).sleep();
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "squirrel_child_follower");
  ChildFollowingAction follow_child(ros::this_node::getName());
  ros::spin();
  return 0;
}
