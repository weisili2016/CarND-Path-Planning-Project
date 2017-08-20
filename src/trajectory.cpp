#include "trajectory.h"
#include <iostream>
#include <vector>
#include "libs/spline.h"
#include "helper_functions.h"

namespace pathplanner {
  using namespace std;
  using namespace helpers;

  Trajectory::Trajectory(vector<double> map_waypoints_s, vector<double> map_waypoints_x, vector<double> map_waypoints_y) {
    this->map_waypoints_s = map_waypoints_s;
    this->map_waypoints_x = map_waypoints_x;
    this->map_waypoints_y = map_waypoints_y;
  }

  void Trajectory::set_previous_path(vector<double> previous_path_x, vector<double> previous_path_y) {
    this->previous_path_x = previous_path_x;
    this->previous_path_y = previous_path_y;
  }

  void Trajectory::convert2Local(vector<double>& ptsx, vector<double>& ptsy) {

    for (int i = 0; i < ptsx.size(); ++i) {
      double shift_x = ptsx[i] - ref_x;
      double shift_y = ptsy[i] - ref_y;
      ptsx[i] = (shift_x*cos(0 - ref_yaw) - shift_y*sin(0 - ref_yaw));
      ptsy[i] = (shift_x*sin(0 - ref_yaw) + shift_y*cos(0 - ref_yaw));
    }
  }

  Trajectory::Coord Trajectory::convert2global(double x, double y) {

    Coord coord;
    coord.x = (x*cos(ref_yaw) - y*sin(ref_yaw));
    coord.y = (x*sin(ref_yaw) + y*cos(ref_yaw));

    coord.x += ref_x;
    coord.y += ref_y;

    return coord;
  }

  void Trajectory::update_trajectory(vector<double> ptsx, vector<double> ptsy, double ref_vel) {
    convert2Local(ptsx, ptsy);

    /*for (double item : ptsx) {
      cout << "xl: " << item << " " << endl;
    }
    for (double item : ptsy) {
      cout << "yl: " << item << " " << endl;
    }*/

    // create a spline
    tk::spline s;
    // set x, y points to the spline
    s.set_points(ptsx, ptsy);

    // fill with the previous points from the last time
    for (int i = 0; i < previous_path_x.size(); ++i) {
      next_x_vals.push_back(previous_path_x[i]);
      next_y_vals.push_back(previous_path_y[i]);
    }

    // calculate how to break up spline points so that we travel at our ddesired reference velocity
    // we have local coordinates here
    double target_x = DISTANCE;
    double target_y = s(target_x);
    double target_dist = sqrt((target_x)*(target_x)+(target_y)*(target_y));

    double x_add_on = 0;

    // fill up the rest planner after filling it with the previous points, here we will always output 50 points
    //
    double N = target_dist / (INTERVAL*convert2mps(ref_vel));
    for (int i = 1; i <= 50 - previous_path_x.size(); ++i) {
      double x_point = x_add_on + (target_x) / N;
      double y_point = s(x_point);

      x_add_on = x_point;

      // rotate back to normal after rotating it earlier
      Coord point = convert2global(x_point, y_point);

      next_x_vals.push_back(point.x);
      next_y_vals.push_back(point.y);
    }

    /*for (double item : next_x_vals) {
      cout << "xl: " << item << " ";
    }
    for (double item : next_y_vals) {
      cout << "yl: " << item << " ";
    }*/
  }

  void Trajectory::generate_trajectory(double car_s, double car_x, double car_y, double car_yaw, int lane, double ref_vel) {
    // Create a list of widly spaced waypoints (x,y), evenly spaced at 30 m
    // Later we will interpolate these waypoints with a spline and fill it in with more points tha control speed
    cout << "generate vel:" << ref_vel << endl;
    next_x_vals.clear();
    next_y_vals.clear();

    vector<double> ptsx;
    vector<double> ptsy;
    //cout << "ref_vel: " << ref_vel << endl;
    if (abs(ref_vel) < 0.1) {
      cout << "car stopped" << endl;
      return;
    }

    ref_x = car_x;
    ref_y = car_y;
    ref_yaw = deg2rad(car_yaw);

    int prev_size = previous_path_x.size();

    if (prev_size < 2 || ref_vel < MIN_SPEED) {
      car_yaw = 0;
      // Use two points that make the path tangent to the car
      double prev_car_x = car_x - cos(car_yaw);
      double prev_car_y = car_y - sin(car_yaw);
      ref_yaw = deg2rad(car_yaw);

      ptsx.push_back(prev_car_x);
      ptsx.push_back(car_x);

      ptsy.push_back(prev_car_y);
      ptsy.push_back(car_y);
    }
    // use the previous path's end point as starting reference
    else {
      ref_x = previous_path_x[prev_size - 1];
      ref_y = previous_path_y[prev_size - 1];

      double ref_x_prev = previous_path_x[prev_size - 2];
      double ref_y_prev = previous_path_y[prev_size - 2];
      ref_yaw = atan2(ref_y - ref_y_prev, ref_x - ref_x_prev);

      // Use two points that make the path tangent to the previous path's end point
      ptsx.push_back(ref_x_prev);
      ptsx.push_back(ref_x);

      ptsy.push_back(ref_y_prev);
      ptsy.push_back(ref_y);
    }

    // In Frenet add evenly 30m spaced points ahead of the starting reference
    vector<double> next_wp0 = getXY(car_s + DISTANCE, MIDDLE_LANE + LANE_WIDTH * lane, map_waypoints_s, map_waypoints_x, map_waypoints_y);
    vector<double> next_wp1 = getXY(car_s + 2*DISTANCE, MIDDLE_LANE + LANE_WIDTH * lane, map_waypoints_s, map_waypoints_x, map_waypoints_y);
    vector<double> next_wp2 = getXY(car_s + 3*DISTANCE, MIDDLE_LANE + LANE_WIDTH * lane, map_waypoints_s, map_waypoints_x, map_waypoints_y);

    ptsx.push_back(next_wp0[0]);
    ptsx.push_back(next_wp1[0]);
    ptsx.push_back(next_wp2[0]);

    ptsy.push_back(next_wp0[1]);
    ptsy.push_back(next_wp1[1]);
    ptsy.push_back(next_wp2[1]);

    /*for (double item : ptsx) {
      cout << "xl: " << item << " " << endl;
    }
    for (double item : ptsy) {
      cout << "yl: " << item << " " << endl;
    }*/

    update_trajectory(ptsx, ptsy, ref_vel);
  }
}