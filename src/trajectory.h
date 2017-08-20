#ifndef TRAJECTORY_H_
#define TRAJECTORY_H_
#include <vector>

namespace pathplanner {
  using namespace std;

  class Trajectory
  {
    public:
      struct Coord {
        double x;
        double y;
      };

      Trajectory(vector<double> map_waypoints_s, vector<double> map_waypoints_x, vector<double> map_waypoints_y);
      
      virtual ~Trajectory() {}

      double ref_x;
      double ref_y;
      double ref_yaw;

      // actual (x, y) we use for the planner
      vector<double> next_x_vals;
      vector<double> next_y_vals;

      vector<double> previous_path_x;
      vector<double> previous_path_y;

      void set_previous_path(vector<double> previous_path_x, vector<double> previous_path_y);
      void generate_trajectory(double car_s, double car_x, double car_y, 
        double car_yaw, int lane, double ref_vel, bool normal_mode);

    private:

      double const INTERVAL = .02;
      double const DISTANCE = 30;
      double const LANE_WIDTH = 4;
      double const MIDDLE_LANE = LANE_WIDTH/2;
      double const MIN_SPEED = 0.3;

      vector<double> map_waypoints_x;
      vector<double> map_waypoints_y;
      vector<double> map_waypoints_s;

      double convert2mps(double mph) { return mph / 2.24; }

      void convert2Local(vector<double>& ptsx, vector<double>& ptsy);

      Coord convert2global(double x, double y);

      void update_trajectory(vector<double> ptsx, vector<double> ptsy, double ref_vel, bool normal_mode);

  };


}

#endif //TRAJECTORY_H_