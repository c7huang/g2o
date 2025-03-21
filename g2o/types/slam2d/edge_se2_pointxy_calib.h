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

#ifndef G2O_EDGE_SE2_XY_CALIB_H
#define G2O_EDGE_SE2_XY_CALIB_H

#include "g2o/core/base_variable_sized_edge.h"
#include "g2o_types_slam2d_api.h"
#include "vertex_point_xy.h"
#include "vertex_se2.h"

namespace g2o {

/**
 * \brief Landmark measurement that also calibrates an offset for the landmark
 * measurement
 */
class G2O_TYPES_SLAM2D_API EdgeSE2PointXYCalib
    : public BaseVariableSizedEdge<2, Vector2> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  EdgeSE2PointXYCalib();

  void computeError() override {
    const auto* v1 = static_cast<const VertexSE2*>(vertexRaw(0));
    const auto* l2 = static_cast<const VertexPointXY*>(vertexRaw(1));
    const auto* calib = static_cast<const VertexSE2*>(vertexRaw(2));
    error_ = ((v1->estimate() * calib->estimate()).inverse() * l2->estimate()) -
             measurement_;
  }

  bool read(std::istream& is) override;
  bool write(std::ostream& os) const override;

  number_t initialEstimatePossible(const OptimizableGraph::VertexSet& from,
                                   OptimizableGraph::Vertex* to) override {
    (void)to;
    return (from.count(vertices_[0]) == 1 ? 1.0 : -1.0);
  }
  void initialEstimate(const OptimizableGraph::VertexSet& from,
                       OptimizableGraph::Vertex* to) override;
};

}  // namespace g2o

#endif
