#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "ramp_msgs/TrajectoryRequest.h"
#include "utility.h"
#include "segment.h"

class Trajectory {
  public:

    Trajectory();
    Trajectory(const ramp_msgs::TrajectoryRequest::Request trajec_req);
    ~Trajectory();

    // Data Members
    std::vector<ramp_msgs::KnotPoint>   knot_points_;
    std::vector<Segment>                segments_;
    std::vector<MotionState>            points_;
    unsigned int                        resolutionRate_;  // in Hz

    // Methods
    const std::vector<MotionState> generate();
    const ramp_msgs::Trajectory buildTrajectoryMsg() const;
    const std::string toString() const;
    
    
  private:
    void  buildSegments();
    const MotionState getMotionState(const unsigned int ind_segment, const float t);
    Utility u;
    const std::vector<MotionState> getStopStates(int i);
    const unsigned int k_dof_;
};

#endif
