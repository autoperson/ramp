#include "collision_detection.h"


CollisionDetection::CollisionDetection() : predictionTime_(ros::Duration(5)), h_traj_req_(0) {}

CollisionDetection::~CollisionDetection() 
{
  if(h_traj_req_ != 0) {
    delete h_traj_req_;
    h_traj_req_ = 0;
  }
}

void CollisionDetection::init(ros::NodeHandle& h) {
  h_traj_req_ = new TrajectoryRequestHandler((const ros::NodeHandle&)h);
  //pub_population = h.advertise<ramp_msgs::Population>("population", 1000);
  setOb_T_w_b(id_);
}




/** Returns true if trajectory_ is in collision with any of the objects */
const CollisionDetection::QueryResult CollisionDetection::perform() const 
{
  ROS_INFO("In CollisionDetection::perform()");

  CollisionDetection::QueryResult result;
  

  // Duration for predicting the object trajectories
  ros::Duration d(5);
  
  // Predict the obstacle's trajectory
  ramp_msgs::RampTrajectory ob_trajectory = getPredictedTrajectory(obstacle_); 


  /*ramp_msgs::Population pop;
  pop.best_id  = 0;
  pop.robot_id = 1;
  pop.population.push_back(ob_trajectory);
  pub_population.publish(pop);*/


  
  // Query for collision
  CollisionDetection::QueryResult q = query(ob_trajectory);
  
  // If collision, set result to q
  if(q.collision_) {
    result = q;
  }

  ROS_INFO("Exiting CollisionDetection::perform()");
  return result;  
} //End perform



//TODO: Get this from parameters...
/** Transformation matrix of obstacle robot from base frame to world frame*/
void CollisionDetection::setOb_T_w_b(int id) {

  // robot 1 needs robot 0's pose
  if(id == 1) {
    tf::Vector3 pos(0., 1.5, 0);
    ob_T_w_b_.setOrigin(pos);
    ob_T_w_b_.setRotation(tf::createQuaternionFromYaw(0));
  }

  // robot 0 needs robot 1's pose
  else {
    tf::Vector3 pos(3.5f, 0.f, 0.f);
    ob_T_w_b_.setRotation(tf::createQuaternionFromYaw(2.35f));
    ob_T_w_b_.setOrigin(pos);
  }
} // End setOb_T_w_b




/*const int getIndexOb(const ramp_msgs::RampTrajectory ob_trajectory, const uint16_t i) {

  int j;
  if(ob_trajectory.trajectory.points.size() == 1) {
    return 0; 
  }

  else if(i < 5) {
    return 0;
  }



}*/

/** 
 * This method returns true if there is collision between trajectory_ and the obstacle's trajectory, false otherwise 
 * The robots are treated as circles for simple collision detection
 */
const CollisionDetection::QueryResult CollisionDetection::query(const ramp_msgs::RampTrajectory ob_trajectory) const {
  /*ROS_INFO("In CollisionDetection::query"); 
  ROS_INFO("trajectory.points.size(): %i", (int)trajectory_.trajectory.points.size());
  ROS_INFO("ob_trajectory.points.size(): %i", (int)ob_trajectory.trajectory.points.size());
  ROS_INFO("ob_trajectory: %s", utility_.toString(ob_trajectory).c_str());*/

  CollisionDetection::QueryResult result;
  uint8_t t_checkColl = 6;

  /*if(ob_trajectory.trajectory.points.size() <= 2) {
    if(id == 0)
      std::cout<<"\nRobot 1 has no trajectory!\n";
    else  
      std::cout<<"\nRobot 0 has no trajectory!\n";
  }*/
  
  uint16_t i_stop;
  if(trajectory_.i_knotPoints.size() == 0) {
    ROS_INFO("i_stop=0");
    i_stop = 0;
  }
  else if(trajectory_.i_knotPoints.size() == 1) {
    ROS_INFO("i_stop=1");
    i_stop = 1;
  }
  else if(trajectory_.i_knotPoints.size() <= 2) {
    ROS_INFO("i_stop=kp 1");
    i_stop = trajectory_.i_knotPoints.at(1);
  }
  else {
    ROS_INFO("i_stop=kp 2");
    i_stop = trajectory_.i_knotPoints.at(2);
  }
  
  ROS_INFO("i_stop: %i", i_stop);
  
  // For every point, check circle detection on a subset of the obstacle's trajectory
  float radius = 0.33f;
  for(uint16_t i=0;i<i_stop;i++) 
  {
    
    // Get the ith point on the trajectory
    trajectory_msgs::JointTrajectoryPoint p_i = trajectory_.trajectory.points.at(i);


    // *** Test position i for collision against some points on obstacle's trajectory ***
    // Obstacle trajectory should already be in world coordinates!
    for(int j = (ob_trajectory.trajectory.points.size() == 1 || i<=t_checkColl) ? 0 : i-t_checkColl ; j<(i+t_checkColl) && j<ob_trajectory.trajectory.points.size(); j++) 
    {
     

      // Get the jth point of the obstacle's trajectory
      trajectory_msgs::JointTrajectoryPoint p_ob  = ob_trajectory.trajectory.points.at(j);

      // Get the distance between the centers
      float dist = sqrt( pow(p_i.positions.at(0) - p_ob.positions.at(0),2) + pow(p_i.positions.at(1) - p_ob.positions.at(1),2) );

      /*ROS_INFO("Robot id: %i - Comparing trajectory point (%f,%f) and obstacle point (%f,%f): dist = %f", 
          id_, 
          p_i.positions.at(0), p_i.positions.at(1), 
          p_ob.positions.at(0), p_ob.positions.at(1), 
          dist);*/
      
        

      // If the distance between the two centers is less than the sum of the two radii, 
      // there is collision
      if( dist <= radius*2 ) 
      {
        /*ROS_INFO("Points in collision: (%f,%f), and (%f,%f), dist: %f i: %i j: %i",
            p_i.positions.at(0),
            p_i.positions.at(1),
            p_ob.positions.at(0),
            p_ob.positions.at(1),
            dist,
            (int)i,
            (int)j);*/
        result.collision_ = true;
        result.t_firstCollision_ = p_i.time_from_start.toSec();
        i = i_stop;
        break;
      } // end if
    } // end for
  } // end for


  ROS_INFO("Exiting CollisionDetection::query");
  return result;
} //End query







/** This method determines what type of motion an obstacle has */
const MotionType CollisionDetection::findMotionType(const ramp_msgs::Obstacle ob) const {
  MotionType result;

  // Find the linear and angular velocities
  tf::Vector3 v_linear;
  tf::vector3MsgToTF(ob.odom_t.twist.twist.linear, v_linear);

  tf::Vector3 v_angular;
  tf::vector3MsgToTF(ob.odom_t.twist.twist.angular, v_angular);

  // Find magnitude of velocity vectors
  float mag_linear_t  = sqrt( tf::tfDot(v_linear, v_linear)   );
  float mag_angular_t = sqrt( tf::tfDot(v_angular, v_angular) );


  // Translation only
  // normally 0.0066 when idle
  if(mag_linear_t >= 0.0001 && mag_angular_t < 0.1) {
    result = MotionType::Translation;
    //std::cout<<"\nMotion type = Translation\n";
  }

  // Self-Rotation
  // normally 0.053 when idle
  else if(mag_linear_t < 0.15 && mag_angular_t >= 0.1) {
    result = MotionType::Rotation;
    //std::cout<<"\nMotion Type = Rotation\n";
  }

  // Either translation+self-rotation or global rotation
  else if(mag_linear_t >= 0.15 && mag_angular_t >= 0.1) {
    result = MotionType::TranslationAndRotation;
    //std::cout<<"\nMotion Type = TranslationAndRotation\n";
  } //end else if

  // Else, there is no motion
  else {
    result = MotionType::None;
    //std::cout<<"\nMotion Type = None\n";
  }

  return result;
} // End findMotionType




/** This method returns the predicted trajectory for an obstacle for the future duration d 
 * TODO: Remove Duration parameter and make the predicted trajectory be computed until robot reaches bounds of environment */
const ramp_msgs::RampTrajectory CollisionDetection::getPredictedTrajectory(const ramp_msgs::Obstacle ob) const 
{
  ramp_msgs::RampTrajectory result;

  // First, identify which type of trajectory it is
  // translations only, self-rotation, translation and self-rotation, or global rotation
  MotionType motion_type = findMotionType(ob);
  

  // Now build a Trajectory Request 
  ramp_msgs::TrajectoryRequest tr;
    tr.request.path = getObstaclePath(ob, motion_type);
    tr.request.type = PREDICTION;  // Prediction


  // Get trajectory
  if(h_traj_req_->request(tr)) 
  {
    result = tr.response.trajectory;
  }

  /*ramp_msgs::Population pop;
  pop.population.push_back(result);
  pop.best_id = 0;
  pop.robot_id = 2;
  pub_population.publish(pop);*/

  return result;
} // End getPredictedTrajectory






/** 
 *  This method returns a prediction for the obstacle's path. 
 *  The path is based on 1) the type of motion the obstacle currently has
 *  2) the duration that we should predict the motion for 
 */
const ramp_msgs::Path CollisionDetection::getObstaclePath(const ramp_msgs::Obstacle ob, const MotionType mt) const 
{
  ramp_msgs::Path result;

  std::vector<ramp_msgs::KnotPoint> path;

  /***********************************************************************
   Create and initialize the first point in the path
   ***********************************************************************/
  ramp_msgs::KnotPoint start;
  start.motionState.positions.push_back(ob.odom_t.pose.pose.position.x);
  start.motionState.positions.push_back(ob.odom_t.pose.pose.position.y);
  start.motionState.positions.push_back(tf::getYaw(ob.odom_t.pose.pose.orientation));

  start.motionState.velocities.push_back(ob.odom_t.twist.twist.linear.x);
  start.motionState.velocities.push_back(ob.odom_t.twist.twist.linear.y);
  start.motionState.velocities.push_back(ob.odom_t.twist.twist.angular.z);

  /** Transform point based on the obstacle's odometry frame */
  // Transform the position
  tf::Vector3 p_st(start.motionState.positions.at(0), start.motionState.positions.at(1), 0); 
  tf::Vector3 p_st_tf = ob_T_w_b_ * p_st;
  
  start.motionState.positions.at(0) = p_st_tf.getX();
  start.motionState.positions.at(1) = p_st_tf.getY();
  start.motionState.positions.at(2) = utility_.displaceAngle(
      tf::getYaw(ob_T_w_b_.getRotation()), start.motionState.positions.at(2));
  
  
  // Transform the velocity
  std::vector<double> zero; zero.push_back(0); zero.push_back(0); 
  double teta = utility_.findAngleFromAToB(zero, start.motionState.positions);
  double phi = start.motionState.positions.at(2);
  double v = start.motionState.velocities.at(0);

  start.motionState.velocities.at(0) = v*cos(phi);
  start.motionState.velocities.at(1) = v*sin(phi);

  ROS_INFO("start: %s", utility_.toString(start).c_str());


  if(v < 0) 
  {
    start.motionState.positions.at(2) = utility_.displaceAngle(start.motionState.positions.at(2), PI);
  }
  
  /***********************************************************************
   ***********************************************************************
   ***********************************************************************/

  /** Adjust the position based on the starting time of the trajectory passed in */
  start.motionState.positions.at(0) += start.motionState.velocities.at(0) * t_start_;
  start.motionState.positions.at(1) += start.motionState.velocities.at(1) * t_start_;
  start.motionState.positions.at(2) += start.motionState.velocities.at(2) * t_start_;

  // Push the first point onto the path
  path.push_back(start);

  /** Find the ending configuration for the predicted trajectory based on motion type */
  // If translation
  if(mt == MotionType::Translation) 
  {

    // Create the Goal Knotpoint
    ramp_msgs::KnotPoint goal;


    double theta = start.motionState.positions.at(2);
    double delta_x = cos(phi)*ob.odom_t.twist.twist.linear.x;
    double delta_y = sin(phi)*ob.odom_t.twist.twist.linear.x;
    //std::cout<<"\ntheta: "<<theta<<" delta_x: "<<delta_x<<" delta_y: "<<delta_y<<"\n";
   

    // Get the goal position in the base frame
    tf::Vector3 ob_goal_b(start.motionState.positions.at(0) + (delta_x * predictionTime_.toSec()), 
                          start.motionState.positions.at(1) + (delta_y * predictionTime_.toSec()),
                          0);

    goal.motionState.positions.push_back(ob_goal_b.getX());
    goal.motionState.positions.push_back(ob_goal_b.getY());
    goal.motionState.positions.push_back(start.motionState.positions.at(2));
    goal.motionState.velocities.push_back(start.motionState.velocities.at(0));
    goal.motionState.velocities.push_back(start.motionState.velocities.at(1));
    goal.motionState.velocities.push_back(start.motionState.velocities.at(2));


    // Push goal onto the path
    path.push_back(goal);
  } // end if translation


  //std::cout<<"\nPath: "<<utility_.toString(utility_.getPath(path));
  result = utility_.getPath(path);
  return result; 
}



