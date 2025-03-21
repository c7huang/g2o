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

#ifndef JACOBIAN_WORKSPACE_H
#define JACOBIAN_WORKSPACE_H

#include <Eigen/Core>
#include <cassert>
#include <vector>

#include "g2o/core/eigen_types.h"
#include "g2o_core_api.h"
#include "hyper_graph.h"

namespace g2o {

struct OptimizableGraph;

/**
 * \brief provide memory workspace for computing the Jacobians
 *
 * The workspace is used by an OptimizableGraph to provide temporary memory
 * for computing the Jacobian of the error functions.
 * Before calling linearizeOplus on an edge, the workspace needs to be allocated
 * by calling allocate().
 *
 * By default, the sizes are updated incrementally with each call. If the reset
 * flag is set to true, the counts are set back to
 */
class G2O_CORE_API JacobianWorkspace {
 public:
  using WorkspaceVector =
      std::vector<VectorX, Eigen::aligned_allocator<VectorX>>;
  JacobianWorkspace() = default;
  ~JacobianWorkspace() = default;

  /**
   * allocate the workspace
   */
  bool allocate();

  /**
   * update the maximum required workspace needed by taking into account this
   * edge
   */
  void updateSize(const HyperGraph::Edge* e, bool reset = false);

  /**
   * update the required workspace by looking at a full graph
   */
  void updateSize(const OptimizableGraph& graph, bool reset = false);

  /**
   * manually update with the given parameters
   */
  void updateSize(int numVertices, int dimension, bool reset = false);

  /**
   * set the full workspace to zero
   */
  void setZero();

  /**
   * return the workspace for a vertex in an edge
   */
  number_t* workspaceForVertex(int vertexIndex) {
    assert(vertexIndex >= 0 && (size_t)vertexIndex < workspace_.size() &&
           "Index out of bounds");
    return workspace_[vertexIndex].data();
  }

 protected:
  WorkspaceVector
      workspace_;  ///< the memory pre-allocated for computing the Jacobians
  int maxNumVertices_{
      -1};  ///< the maximum number of vertices connected by a hyper-edge
  int maxDimension_{
      -1};  ///< the maximum dimension (number of elements) for a Jacobian
};

}  // namespace g2o

#endif
