#include "path.h"

Path::Path() {}

Path::Path(Configuration start, Configuration goal) : start_(start), goal_(goal) {
  all_.push_back(start);
  all_.push_back(goal); 
}

Path::Path(std::vector<Configuration> all) {
  start_ = all.at(0);
  goal_ = all.at(all.size()-1);
  

  for(unsigned int i=0;i<all.size();i++) {
    all_.push_back(all.at(i));
  }
}

Path::Path(ramp_msgs::Path p) {

  Configuration s(p.configurations.at(0));
  start_ = s;

  Configuration g(p.configurations.at(p.configurations.size()-1));
  goal_ = g;

  for(unsigned int i=0;i<p.configurations.size();i++) {
    Configuration c(p.configurations.at(i));
    all_.push_back(c);
  }

  stop_points_ = p.stop_points;
  stop_times_  = p.stop_times;
}

Path::~Path() {}

/** This method inserts the configuration c into the path at location path_size-1 */
void Path::Add(const Configuration c) {
  all_.insert(all_.end()-1, c); 
}


const unsigned int Path::size() const { return all_.size(); }


const ramp_msgs::Path Path::buildPathMsg() const {
  ramp_msgs::Path result;

  //Push all of the configurations onto the Path msg
  for(unsigned int i=0;i<all_.size();i++) {

    //Build the configuration msg
    ramp_msgs::Configuration c = all_.at(i).buildConfigurationMsg();
    
    //Push the msg onto K
    result.configurations.push_back(c);
  }
  
  result.stop_points = stop_points_;
  result.stop_times  = stop_times_;

  return result;
}

const std::string Path::toString() const {
  std::ostringstream result;

  result<<"Path:";
  for(unsigned int i=0;i<all_.size();i++) {
    result<<"\n  "<<i<<": "<<all_.at(i).toString();
  }
  result<<"\n  Stop points: ";
  for(unsigned int i=0;i<stop_points_.size();i++) {
    result<<stop_points_.at(i)<<" ("<<stop_times_.at(i)<<"s), ";
  }
  
  return result.str();
}
