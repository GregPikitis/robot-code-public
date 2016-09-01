#ifndef MUAN_CONTROL_TRAPEZOIDALMOTIONPROFILE_H_
#define MUAN_CONTROL_TRAPEZOIDALMOTIONPROFILE_H_

#include "motion_profile.h"
#include <cmath>
#include <type_traits>

namespace muan {

namespace control {

struct MotionProfileConstraints {
  double max_velocity;
  double max_acceleration;
};

/*
 * A trapezoidal-shaped velocity profile.
 * Example:
 *    MotionProfileConstraints constraints{ 1 * m / s, 1 * m / s / s };
 *    TrapezoidalMotionProfile<Length> profile{ constraints,
 *                                              current_position,
 *                                              goal_position };
 *    ...
 *    desired_position = profile.Calculate(t - profile_start_time);
 */
class TrapezoidalMotionProfile : public MotionProfile {
 public:
  TrapezoidalMotionProfile(MotionProfileConstraints constraints,
                           MotionProfilePosition goal)
      : TrapezoidalMotionProfile{constraints, goal,
                                 MotionProfilePosition{0, 0}} {}

  TrapezoidalMotionProfile(MotionProfileConstraints constraints,
                           MotionProfilePosition goal,
                           MotionProfilePosition initial);

  virtual ~TrapezoidalMotionProfile() = default;

  // Calculate the correct position and velocity for the profile at a time t
  // where the beginning of the profile was at time t=0
  MotionProfilePosition Calculate(Time t) const override;

  Time total_time() const override { return end_deccel_; }

  MotionProfileConstraints& constraints() { return constraints_; }

 private:
  // Is the profile inverted? In other words, does it need to increase or
  // decrease the velocity to get to the peak from the initial velocity?
  bool ShouldFlipAcceleration(
      const MotionProfilePosition& initial, const MotionProfilePosition& goal,
      const MotionProfileConstraints& constraints) const {
    // Calculate the distance travelled by a linear velocity ramp
    // from the initial to the final velocity and compare it to the desired
    // distance. If it is smaller, invert the profile.
    double velocity_change = goal.velocity - initial.velocity;

    double distance_change = goal.position - initial.position;

    Time t = muan::abs(velocity_change) / constraints.max_acceleration;
    bool is_acceleration_flipped =
        t * (velocity_change / 2 + initial.velocity) > distance_change;
    return is_acceleration_flipped;
  }

  // Flip the sign of the velocity and position if the profile is inverted
  MotionProfilePosition Direct(const MotionProfilePosition& in) const {
    MotionProfilePosition result = in;
    result.position *= direction_;
    result.velocity *= direction_;
    return result;
  }

  // The direction of the profile, either 1 for forwards or -1 for inverted
  int direction_;

  MotionProfileConstraints constraints_;
  MotionProfilePosition initial_, goal_;

  Time end_accel_, end_full_speed_, end_deccel_;
};

} /* control */

} /* muan */

#include "trapezoidal_motion_profile.hpp"

#endif /* SRC_ROBOTCODE_TRAPEZOIDALMOTIONPROFILE_H_ */
