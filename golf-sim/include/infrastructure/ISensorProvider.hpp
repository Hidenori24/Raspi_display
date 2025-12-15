#pragma once

namespace infrastructure {

// Raw sensor data frame
struct SensorFrame {
  double t_sec = 0.0;
  float ax = 0.0f;
  float ay = 0.0f;
  float az = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;
  
  SensorFrame() = default;
  SensorFrame(double t, float ax_, float ay_, float az_, float gx_, float gy_, float gz_)
    : t_sec(t), ax(ax_), ay(ay_), az(az_), gx(gx_), gy(gy_), gz(gz_) {}
};

// Strategy interface for sensor input
class ISensorProvider {
public:
  virtual ~ISensorProvider() = default;
  
  // Non-blocking poll: returns true if data available
  virtual bool poll(SensorFrame& out) = 0;
  
  // Reset provider state
  virtual void reset() = 0;
};

} // namespace infrastructure
