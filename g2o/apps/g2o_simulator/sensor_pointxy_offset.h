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

#ifndef G2O_SENSOR_POINTXY_OFFSET_H_
#define G2O_SENSOR_POINTXY_OFFSET_H_

#include "g2o_simulator_api.h"
#include "pointsensorparameters.h"
#include "simulator2d_base.h"

namespace g2o {

class G2O_SIMULATOR_API SensorPointXYOffset
    : public PointSensorParameters,
      public BinarySensor<Robot2D, EdgeSE2PointXYOffset, WorldObjectPointXY> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using RobotPoseType = PoseVertexType::EstimateType;
  explicit SensorPointXYOffset(const std::string& name);
  void sense() override;
  void addNoise(EdgeType* e) override;
  void addParameters() override;

 protected:
  bool isVisible(WorldObjectType* to);
  std::shared_ptr<ParameterSE2Offset> offsetParam_;
  RobotPoseType sensorPose_;
};

}  // namespace g2o

#endif
