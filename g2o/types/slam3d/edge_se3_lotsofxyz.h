// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, H. Strasdat, W. Burgard
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

#ifndef G2O_SE3_LOTSOF_XYZ
#define G2O_SE3_LOTSOF_XYZ

#include "g2o/config.h"
#include "g2o/core/base_variable_sized_edge.h"
#include "g2o_types_slam3d_api.h"
#include "vertex_pointxyz.h"
#include "vertex_se3.h"

namespace g2o {

class G2O_TYPES_SLAM3D_API EdgeSE3LotsOfXYZ
    : public BaseVariableSizedEdge<-1, VectorX> {
 protected:
  unsigned int observedPoints_ = 0;

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  EdgeSE3LotsOfXYZ();

  void setSize(int vertices) {
    resize(vertices);
    observedPoints_ = vertices - 1;
    measurement_.resize(observedPoints_ * 3L, 1);
    setDimension(observedPoints_ * 3);
  }

  void computeError() override;

  bool read(std::istream& is) override;
  bool write(std::ostream& os) const override;

  bool setMeasurementFromState() override;

  void initialEstimate(const OptimizableGraph::VertexSet&,
                       OptimizableGraph::Vertex*) override;
  number_t initialEstimatePossible(const OptimizableGraph::VertexSet&,
                                   OptimizableGraph::Vertex*) override;

  void linearizeOplus() override;
};

}  // namespace g2o

#endif  // G2O_SE3_LOTSOF_XYZ
