#ifndef ESTIMATOR_H_
#define ESTIMATOR_H_
#include <vector>
#include <math.h>
#include <functional>
#include "vehicle.h"


namespace pathplanner {
  using namespace std;

  class Estimator
  {
    public:

      Estimator() {}

      virtual ~Estimator() {}

      double calculate_cost(vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>>predictions, string state, bool verbose = false);


    private:
      struct collision {
        bool hasCollision;
        int step;
      };

      struct TrajectoryData
      {
        int proposed_lane;
        double avg_speed;
        double max_acceleration;
        double rms_acceleration;
        double closest_approach;
        collision collides;
      };

      typedef std::function<double(Vehicle vehicle, vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions, TrajectoryData data)> DelegateType;

      // priority levels for costs
      int const COLLISION = pow(10, 6);
      int const DANGER = pow(10, 5);
      int const REACH_GOAL = pow(10, 5);
      int const COMFORT = pow(10, 4);
      int const EFFICIENCY = pow(10, 2);

      double const DESIRED_BUFFER = 1.5; // timesteps
      int const PLANNING_HORIZON = 2;

      double const INTERVAL = .02;
      double const DISTANCE = 30;
      double const LANE_WIDTH = 4.0;
      double const MIDDLE_LANE = LANE_WIDTH / 2;

      double change_lane_cost(Vehicle vehicle, vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions, TrajectoryData data);

      double inefficiency_cost(Vehicle vehicle, vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions, TrajectoryData data);

      double collision_cost(Vehicle vehicle, vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions, TrajectoryData data);

      double buffer_cost(Vehicle vehicle, vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions, TrajectoryData data);

      TrajectoryData get_helper_data(vector<Vehicle::snapshot> trajectory,
        map<int, vector<Vehicle::prediction>> predictions);

      bool check_collision(Vehicle::snapshot snap, double s_previous, double s_now);

      map<int, vector<Vehicle::prediction>> filter_predictions_by_lane(
        map<int, vector<Vehicle::prediction>> predictions, int lane);
  };

}
#endif //ESTIMATOR_H_