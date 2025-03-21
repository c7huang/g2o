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

#ifndef G2O_SENSOR_SEGMENT2D_POINTLINE_H_
#define G2O_SENSOR_SEGMENT2D_POINTLINE_H_

#include "g2o/apps/g2o_simulator/pointsensorparameters.h"
#include "g2o/types/slam2d_addons/types_slam2d_addons.h"
#include "simulator2d.h"

namespace g2o {

// sensor that senses segments, only if the extremas are visible
class G2O_SIMULATOR_API SensorSegment2DPointLine
    : public PointSensorParameters,
      public BinarySensor<Robot2D, EdgeSE2Segment2DPointLine,
                          WorldObjectSegment2D> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit SensorSegment2DPointLine(const std::string& name);
  void sense() override;
  void addNoise(EdgeType* e) override;

 protected:
  bool isVisible(WorldObjectType* to);
  int visiblePoint_ = 0;
};

}  // namespace g2o

#endif
