#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <math.h>
#include "ramp_msgs/Path.h"
#include "ramp_msgs/Range.h"
#include <ros/console.h>
#include <vector>
#include <chrono>
#include <ros/console.h>
#include <ros/package.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "circle_filter.h" // Needs to be included AFTER opencv
using namespace std::chrono;

#define PI 3.14159f

struct Edge
{
  cv::Point start;
  cv::Point end;
};

struct Normal
{
  double a, b, c;
};

struct Triangle
{
  std::vector<Edge> edges;
};


struct Polygon
{
  std::vector<Edge> edges;
  std::vector<Normal> normals;
};

struct Cell
{
  cv::Point p;
  double dist;
};

struct Point
{
  double x;
  double y;
};

struct Circle
{
  Point center;
  double radius;
};

struct Velocity
{
  double v, vx, vy, w;
};

struct CompareDist
{
  bool operator()(const Cell& c1, const Cell& c2)
  {
    return c1.dist < c2.dist;
  }
};


struct CircleGroup
{
  Circle fitCir;
  std::vector<Circle> packedCirs;
};

struct CircleOb
{
  CircleOb() {}
  ~CircleOb()
  {
    delete kf;
  }
  CircleGroup cirs;
  Circle cir;
  CircleFilter* kf;

  std::vector<Circle> prevCirs;
  std::vector<double> prevTheta;
  Velocity vel;
  std::vector<Velocity> vels;
  double vx, vy, v;
  double theta, w;
  int i_prevThetaCir;

  int static_count=0;
  bool moving=true;
  bool hasMoved=false;
};



struct CircleMatch
{
  int i_cirs;
  int i_prevCir;
  double dist;
  double delta_r;

};


class Utility {
  public:
    Utility();

    std::vector<ramp_msgs::Range> standardRanges_;

    bool checkAngleBetweenAngles(const double angle, const double min, const double max) const;

    const double positionDistance(const std::vector<double> a, const std::vector<double> b) const;
    const double positionDistance(const double ax, const double ay, const double bx, const double by) const;
    
    const double findAngleFromAToB(const std::vector<double> a, const std::vector<double> b) const;

    const double findDistanceBetweenAngles(const double a1, const double a2) const;

    const double findAngleBetweenAngles(const double a1, const double a2) const;
    
    const double displaceAngle(const double a1, double a2) const;

    // Needs to be static to use in std::sort()
    static bool compareCircleMatches(const CircleMatch& a, const CircleMatch& b);

    const std::string toString(const ramp_msgs::Path p) const;    
    const std::string toString(const ramp_msgs::MotionState mp) const;    
};
#endif
