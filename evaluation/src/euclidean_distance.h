#ifndef EUCLIDEAN_DISTANCE_H
#define EUCLIDEAN_DISTANCE_H
#include "utility.h"
#include "ramp_msgs/Trajectory.h"


class EuclideanDistance {
  public:
    EuclideanDistance() {}


    const double perform(obstacle_struct obstacle);

    ramp_msgs::Trajectory trajectory_;

};

#endif
