#include "ros/ros.h"
#include "trajectory.h"
#include "ramp_msgs/TrajectoryRequest.h"

Utility u;

bool handleRequest(ramp_msgs::TrajectoryRequest::Request& req, 
                   ramp_msgs::TrajectoryRequest::Response& res) 
{
  //std::cout<<"\nRequest received:"<<u.toString(req)<<"\n";
  
  Trajectory traj(req);
  traj.generate(); 
  res.trajectory = traj.buildTrajectoryMsg();
  
  //std::cout<<"\nSending back:"<<u.toString(res.trajectory);
  return true;
}


int main(int argc, char** argv) {

  ros::init(argc, argv, "trajectory_generator");
  ros::NodeHandle handle;

  ros::ServiceServer service = handle.advertiseService("trajectory_generator", handleRequest);

  /* *** Test generating a trajectory *** */

  // Build a Path
  ramp_msgs::Configuration c1;
  c1.K.push_back(1);
  c1.K.push_back(3);
  c1.K.push_back(0);

  ramp_msgs::Configuration c2;
  c2.K.push_back(2.65712);
  c2.K.push_back(2.89299);
  c2.K.push_back(0.110012);

  ramp_msgs::Configuration c3;
  c3.K.push_back(3);
  c3.K.push_back(3);
  c3.K.push_back(0);

  /*ramp_msgs::Configuration c4;
  c4.K.push_back(5);
  c4.K.push_back(0);
  c4.K.push_back(-M_PI / 2);*/

  std::vector<float> v_s;
  v_s.push_back(0.25);
  //v_s.push_back(0.25);
  //v_s.push_back(0.25);

  std::vector<float> v_g;
  v_g.push_back(0.25);
  //v_g.push_back(0.25);
  //v_g.push_back(0.25);

  ramp_msgs::Path p;
  p.configurations.push_back(c1);
  //p.configurations.push_back(c2);
  p.configurations.push_back(c3);
  //p.configurations.push_back(c4);
  
  ramp_msgs::TrajectoryRequest tr;
  tr.request.path = p;
  
  tr.request.v_start = v_s;
  tr.request.v_end = v_g;
  tr.request.resolutionRate = 5;

  std::cout<<"\nConstructing Trajectory\n";
  Trajectory traj(tr.request);
  std::cout<<"\nDone constructing\n";
  std::cout<<"\nGenerating Trajectory\n";
  traj.generate(); 
  tr.response.trajectory = traj.buildTrajectoryMsg();
  
  std::cout<<"\n"<<u.toString(tr.response.trajectory);
  
  std::cout<<"\nSpinning...\n";  
  ros::spin();
  std::cout<<"\nExiting Normally\n";
  return 0;
}
