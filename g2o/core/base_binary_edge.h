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

#ifndef G2O_BASE_BINARY_EDGE_H
#define G2O_BASE_BINARY_EDGE_H

#include "base_fixed_sized_edge.h"

namespace g2o {

// This could be a simple using statement, but in multiple places
// jacobianOplusXi_ and jacobianOplusXj are used.
template <int D, typename E, typename VertexXi, typename VertexXj>
class BaseBinaryEdge : public BaseFixedSizedEdge<D, E, VertexXi, VertexXj> {
 public:
  using VertexXiType = VertexXi;
  using VertexXjType = VertexXj;
  BaseBinaryEdge() : BaseFixedSizedEdge<D, E, VertexXi, VertexXj>(){};

 protected:
  typename BaseFixedSizedEdge<D, E, VertexXi, VertexXj>::template JacobianType<
      D, VertexXi::kDimension>& jacobianOplusXi_ =
      std::get<0>(this->jacobianOplus_);
  typename BaseFixedSizedEdge<D, E, VertexXi, VertexXj>::template JacobianType<
      D, VertexXj::kDimension>& jacobianOplusXj_ =
      std::get<1>(this->jacobianOplus_);
};

}  // namespace g2o

#endif
