// g2o - General Graph Optimization
// Copyright (C) 2011 G. Grisetti, R. Kuemmerle, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef G2O_SENSOR_POINTXYZ_H_
#define G2O_SENSOR_POINTXYZ_H_
#include "g2o_simulator_api.h"
#include "pointsensorparameters.h"
#include "simulator3d_base.h"

namespace g2o {

class G2O_SIMULATOR_API SensorPointXYZ
    : public PointSensorParameters,
      public BinarySensor<Robot3D, EdgeSE3PointXYZ, WorldObjectTrackXYZ> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using RobotPoseType = PoseVertexType::EstimateType;
  explicit SensorPointXYZ(const std::string& name);
  void sense() override;
  void addParameters() override;
  std::shared_ptr<ParameterSE3Offset> offsetParam() { return offsetParam_; };
  void addNoise(EdgeType* e) override;

 protected:
  bool isVisible(WorldObjectType* to);
  RobotPoseType sensorPose_;
  std::shared_ptr<ParameterSE3Offset> offsetParam_;
};

}  // namespace g2o

#endif
