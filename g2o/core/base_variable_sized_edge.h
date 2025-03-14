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

#ifndef G2O_BASE_VARIABLE_SIZED_EDGE_H
#define G2O_BASE_VARIABLE_SIZED_EDGE_H

#include <iomanip>
#include <iostream>
#include <limits>

#include "base_edge.h"
#include "g2o/autodiff/fixed_array.h"
#include "g2o/config.h"
#include "g2o/stuff/misc.h"
#include "robust_kernel.h"

namespace g2o {

/**
 * \brief base class to represent an edge connecting an arbitrary number of
 * nodes
 *
 * D - Dimension of the measurement
 * E - type to represent the measurement
 */
template <int D, typename E>
class BaseVariableSizedEdge : public BaseEdge<D, E> {
 public:
  /**
   * \brief helper for mapping the Hessian memory of the upper triangular block
   */
  struct HessianHelper {
    Eigen::Map<MatrixX> matrix;  ///< the mapped memory
    bool transposed = false;     ///< the block has to be transposed
    HessianHelper() : matrix(nullptr, 0, 0) {}
  };

  static constexpr int kDimension = BaseEdge<D, E>::kDimension;
  using Measurement = typename BaseEdge<D, E>::Measurement;
  using JacobianType = MatrixX::MapType;
  using ErrorVector = typename BaseEdge<D, E>::ErrorVector;
  using InformationType = typename BaseEdge<D, E>::InformationType;
  using HessianBlockType =
      Eigen::Map<MatrixX, MatrixX::Flags & Eigen::PacketAccessBit
                              ? Eigen::Aligned
                              : Eigen::Unaligned>;

  BaseVariableSizedEdge() : BaseEdge<D, E>() {}

  void linearizeOplus(JacobianWorkspace& jacobianWorkspace) override;

  /**
   * Linearizes the oplus operator in the vertex, and stores
   * the result in temporary variable vector _jacobianOplus
   */
  virtual void linearizeOplus();

  void resize(size_t size) override;

  bool allVerticesFixed() const override;

  void constructQuadraticForm() override;

  void mapHessianMemory(number_t* d, int i, int j, bool rowMajor) override;

  using BaseEdge<D, E>::computeError;

 protected:
  using BaseEdge<D, E>::measurement_;
  using BaseEdge<D, E>::information_;
  using BaseEdge<D, E>::error_;
  using BaseEdge<D, E>::vertices_;
  using BaseEdge<D, E>::dimension_;

  std::vector<HessianHelper> hessian_;
  std::vector<JacobianType, Eigen::aligned_allocator<JacobianType> >
      jacobianOplus_;  ///< jacobians of the edge (w.r.t. oplus)

  void computeQuadraticForm(const InformationType& omega,
                            const ErrorVector& weightedError);

  OptimizableGraph::Vertex* vertexRaw(size_t n) const {
    assert(n < vertices_.size() && "Index out of bounds");
    return static_cast<OptimizableGraph::Vertex*>(vertices_[n].get());
  }

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#include "base_variable_sized_edge.hpp"

}  // end namespace g2o

#endif
