#include "insert.h"


Insert::Insert(const ramp_msgs::Path p) : path_(p) {}


const ramp_msgs::Path Insert::perform() {

  // Randomly choose the two adjacent knot points
  // One index is generated randomly and the second will be the next knot point,
  // unless the generated index is the last knot point index
  // The new knot point will be inserted between those two knot points
  unsigned int i_knotPoint1 = rand() % path_.points.size(); 
  unsigned int i_knotPoint2 = (i_knotPoint1 == path_.points.size()-1) ? i_knotPoint1-1 : i_knotPoint1+1;

  // If the last index was chosen, swap the values so i_knotPoint1 < i_knotPoint2
  // this just makes it simpler 
  if(i_knotPoint2 < i_knotPoint1) {
    unsigned int swap = i_knotPoint1;
    i_knotPoint1 = i_knotPoint2;
    i_knotPoint2 = swap;
  }

  // Generate a new, random configuration (gets stored in motion state position)
  ramp_msgs::KnotPoint kp;
  for(unsigned int i=0;i<path_.points.at(0).motionState.positions.size();i++) {
    
    // Generate a random value for each K in the specified range
    double  min = utility.standardRanges_.at(i).min;
    double  max = utility.standardRanges_.at(i).max;
    float   temp = (min == 0 && max == 0) ? 0 :      
           ( min + (float)rand() / ((float)RAND_MAX / (max - min)) );

    // Set the position
    kp.motionState.positions.push_back(temp);

    // Set the velocity
    if(i_knotPoint1 == 0)
      kp.motionState.velocities.push_back(path_.points.at(i_knotPoint2).motionState.velocities.at(i));
    else
      kp.motionState.velocities.push_back(path_.points.at(i_knotPoint1).motionState.velocities.at(i));
  }

  // Insert the configuration 
  path_.points.insert(path_.points.begin()+i_knotPoint2, kp);
  
  return path_; 
}
