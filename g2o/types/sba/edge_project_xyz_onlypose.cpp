// g2o - General Graph Optimization
// Copyright (C) 2011 H. Strasdat
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

#include "edge_project_xyz_onlypose.h"

namespace g2o {

bool EdgeSE3ProjectXYZOnlyPose::read(std::istream &is) {
  internal::readVector(is, measurement_);
  return readInformationMatrix(is);
}

bool EdgeSE3ProjectXYZOnlyPose::write(std::ostream &os) const {
  internal::writeVector(os, measurement());
  return writeInformationMatrix(os);
}

void EdgeSE3ProjectXYZOnlyPose::linearizeOplus() {
  VertexSE3Expmap *vi = vertexXnRaw<0>();
  Vector3 xyz_trans = vi->estimate().map(Xw);

  number_t x = xyz_trans[0];
  number_t y = xyz_trans[1];
  number_t invz = 1.0 / xyz_trans[2];
  number_t invz_2 = invz * invz;

  jacobianOplusXi_(0, 0) = x * y * invz_2 * fx;
  jacobianOplusXi_(0, 1) = -(1 + (x * x * invz_2)) * fx;
  jacobianOplusXi_(0, 2) = y * invz * fx;
  jacobianOplusXi_(0, 3) = -invz * fx;
  jacobianOplusXi_(0, 4) = 0;
  jacobianOplusXi_(0, 5) = x * invz_2 * fx;

  jacobianOplusXi_(1, 0) = (1 + y * y * invz_2) * fy;
  jacobianOplusXi_(1, 1) = -x * y * invz_2 * fy;
  jacobianOplusXi_(1, 2) = -x * invz * fy;
  jacobianOplusXi_(1, 3) = 0;
  jacobianOplusXi_(1, 4) = -invz * fy;
  jacobianOplusXi_(1, 5) = y * invz_2 * fy;
}

Vector2 EdgeSE3ProjectXYZOnlyPose::cam_project(const Vector3 &trans_xyz) const {
  Vector2 proj = project(trans_xyz);
  Vector2 res;
  res[0] = proj[0] * fx + cx;
  res[1] = proj[1] * fy + cy;
  return res;
}

void EdgeSE3ProjectXYZOnlyPose::computeError() {
  const VertexSE3Expmap *v1 = vertexXnRaw<0>();
  Vector2 obs(measurement_);
  error_ = obs - cam_project(v1->estimate().map(Xw));
}

bool EdgeSE3ProjectXYZOnlyPose::isDepthPositive() {
  const VertexSE3Expmap *v1 = vertexXnRaw<0>();
  return (v1->estimate().map(Xw))(2) > 0;
}

}  // namespace g2o
