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

#include "robust_kernel_impl.h"

#include <cmath>
#include <utility>

#include "robust_kernel_factory.h"

namespace g2o {

RobustKernelScaleDelta::RobustKernelScaleDelta(RobustKernelPtr kernel,
                                               number_t delta)
    : RobustKernel(delta), kernel_(std::move(kernel)) {}

RobustKernelScaleDelta::RobustKernelScaleDelta(number_t delta)
    : RobustKernel(delta) {}

void RobustKernelScaleDelta::setKernel(const RobustKernelPtr& ptr) {
  kernel_ = ptr;
}

void RobustKernelScaleDelta::robustify(number_t error, Vector3& rho) const {
  if (kernel_.get()) {
    const number_t dsqr = delta_ * delta_;
    const number_t dsqrReci = 1. / dsqr;
    kernel_->robustify(dsqrReci * error, rho);
    rho[0] *= dsqr;
    rho[2] *= dsqrReci;
  } else {  // no robustification
    rho[0] = error;
    rho[1] = 1.;
    rho[2] = 0.;
  }
}

void RobustKernelHuber::robustify(number_t e, Vector3& rho) const {
  const number_t dsqr = delta_ * delta_;
  if (e <= dsqr) {  // inlier
    rho[0] = e;
    rho[1] = 1.;
    rho[2] = 0.;
  } else {                           // outlier
    const number_t sqrte = sqrt(e);  // absolute value of the error
    rho[0] =
        2 * sqrte * delta_ - dsqr;  // rho(e)   = 2 * delta * e^(1/2) - delta^2
    rho[1] = delta_ / sqrte;        // rho'(e)  = delta / sqrt(e)
    rho[2] = -0.5 * rho[1] /
             e;  // rho''(e) = -1 / (2*e^(3/2)) = -1/2 * (delta/e) / e
  }
}

void RobustKernelPseudoHuber::robustify(number_t e2, Vector3& rho) const {
  const number_t dsqr = delta_ * delta_;
  const number_t dsqrReci = 1. / dsqr;
  const number_t aux1 = dsqrReci * e2 + 1.0;
  const number_t aux2 = sqrt(aux1);
  rho[0] = 2 * dsqr * (aux2 - 1);
  rho[1] = 1. / aux2;
  rho[2] = -0.5 * dsqrReci * rho[1] / aux1;
}

void RobustKernelCauchy::robustify(number_t e2, Vector3& rho) const {
  const number_t dsqr = delta_ * delta_;
  const number_t dsqrReci = 1. / dsqr;
  const number_t aux = dsqrReci * e2 + 1.0;
  rho[0] = dsqr * log(aux);
  rho[1] = 1. / aux;
  rho[2] = -dsqrReci * std::pow(rho[1], 2);
}

void RobustKernelGemanMcClure::robustify(number_t e2, Vector3& rho) const {
  const number_t aux = delta_ / (delta_ + e2);
  rho[0] = e2 * aux;
  rho[1] = aux * aux;
  rho[2] = -2. * rho[1] * aux;
}

void RobustKernelWelsch::robustify(number_t e2, Vector3& rho) const {
  const number_t dsqr = delta_ * delta_;
  const number_t aux = e2 / dsqr;
  const number_t aux2 = exp(-aux);
  rho[0] = dsqr * (1. - aux2);
  rho[1] = aux2;
  rho[2] = -aux2 / dsqr;
}

void RobustKernelFair::robustify(number_t e2, Vector3& rho) const {
  const number_t sqrte = sqrt(e2);
  const number_t aux = sqrte / delta_;
  rho[0] = 2. * delta_ * delta_ * (aux - log1p(aux));
  rho[1] = 1. / (1. + aux);
  rho[2] = -0.5 / (sqrte * (1. + aux));
}

void RobustKernelTukey::robustify(number_t e2, Vector3& rho) const {
  const number_t e = sqrt(e2);
  const number_t delta2 = delta_ * delta_;
  if (e <= delta_) {
    const number_t aux = e2 / delta2;
    rho[0] = delta2 * (1. - std::pow((1. - aux), 3)) / 3.;
    rho[1] = std::pow((1. - aux), 2);
    rho[2] = -2. * (1. - aux) / delta2;
  } else {
    rho[0] = delta2 / 3.;
    rho[1] = 0;
    rho[2] = 0;
  }
}

void RobustKernelSaturated::robustify(number_t e2, Vector3& rho) const {
  const number_t dsqr = delta_ * delta_;
  if (e2 <= dsqr) {  // inlier
    rho[0] = e2;
    rho[1] = 1.;
    rho[2] = 0.;
  } else {  // outlier
    rho[0] = dsqr;
    rho[1] = 0.;
    rho[2] = 0.;
  }
}

// delta is used as $phi$
void RobustKernelDCS::robustify(number_t e2, Vector3& rho) const {
  const number_t& phi = delta_;
  const number_t scale = (2.0 * phi) / (phi + e2);
  if (scale >= 1.0) {  // limit scale to max of 1 and return this
    rho[0] = e2;
    rho[1] = 1.;
    rho[2] = 0;
  } else {
    const number_t phi_sqr = phi * phi;
    rho[0] = scale * e2 * scale;
    rho[1] = (4 * phi_sqr * (phi - e2)) / std::pow(phi + e2, 3);
    rho[2] = -(8 * phi_sqr * (2 * phi - e2)) / std::pow(phi + e2, 4);
  }
}

// register the kernel to their factory
G2O_REGISTER_ROBUST_KERNEL(Huber, RobustKernelHuber)
G2O_REGISTER_ROBUST_KERNEL(PseudoHuber, RobustKernelPseudoHuber)
G2O_REGISTER_ROBUST_KERNEL(Cauchy, RobustKernelCauchy)
G2O_REGISTER_ROBUST_KERNEL(GemanMcClure, RobustKernelGemanMcClure)
G2O_REGISTER_ROBUST_KERNEL(Welsch, RobustKernelWelsch)
G2O_REGISTER_ROBUST_KERNEL(Fair, RobustKernelFair)
G2O_REGISTER_ROBUST_KERNEL(Tukey, RobustKernelTukey)
G2O_REGISTER_ROBUST_KERNEL(Saturated, RobustKernelSaturated)
G2O_REGISTER_ROBUST_KERNEL(DCS, RobustKernelDCS)
}  // end namespace g2o
