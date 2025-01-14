// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
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

#ifndef G2O_TYPES_SLAM2D_ONLINE_H
#define G2O_TYPES_SLAM2D_ONLINE_H

#include <iostream>

#include "g2o/types/slam2d/edge_se2.h"
#include "g2o_interactive_api.h"

namespace g2o {

class G2O_INTERACTIVE_API OnlineVertexSE2 : public VertexSE2 {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  OnlineVertexSE2() = default;

  void oplusImpl(const VectorX::MapType& update) override {
    VertexSE2::oplusImpl(update);
    updatedEstimate = estimate_;
  }

  void oplusUpdatedEstimate(double* update) {
    Eigen::Vector3d p = estimate_.toVector();
    p += Eigen::Map<Eigen::Vector3d>(update);
    p[2] = normalize_theta(p[2]);
    updatedEstimate.fromVector(p);
    // std::cerr << PVAR(updatedEstimate.toVector()) << " " <<
    // PVAR(estimate_.toVector()) << std::endl;
  }

  VertexSE2::EstimateType updatedEstimate;
};

class G2O_INTERACTIVE_API OnlineEdgeSE2 : public EdgeSE2 {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  OnlineEdgeSE2() = default;

  void initialEstimate(const OptimizableGraph::VertexSet& from,
                       OptimizableGraph::Vertex* /* to */) override {
    auto fromEdge = std::static_pointer_cast<OnlineVertexSE2>(vertexXn<0>());
    auto toEdge = std::static_pointer_cast<OnlineVertexSE2>(vertexXn<1>());
    if (from.count(fromEdge) > 0) {
      toEdge->updatedEstimate = fromEdge->updatedEstimate * measurement_;
      toEdge->setEstimate(toEdge->updatedEstimate);
    } else {
      fromEdge->updatedEstimate = toEdge->updatedEstimate * inverseMeasurement_;
      fromEdge->setEstimate(fromEdge->updatedEstimate);
    }
  }

  double chi2() const override {
    const auto* v1 = static_cast<const OnlineVertexSE2*>(vertexXnRaw<0>());
    const auto* v2 = static_cast<const OnlineVertexSE2*>(vertexXnRaw<1>());
    SE2 delta = inverseMeasurement_ *
                (v1->updatedEstimate.inverse() * v2->updatedEstimate);
    Eigen::Vector3d error = delta.toVector();
    return error.dot(information() * error);
  }
};

}  // namespace g2o

#endif
