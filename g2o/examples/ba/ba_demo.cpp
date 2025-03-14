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

#include <cstdint>
#include <iostream>
#include <unordered_set>

#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/robust_kernel_impl.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/solvers/structure_only/structure_only_solver.h"
#include "g2o/stuff/sampler.h"
#include "g2o/types/sba/types_six_dof_expmap.h"

#if defined G2O_HAVE_CHOLMOD
G2O_USE_OPTIMIZATION_LIBRARY(cholmod);
#else
G2O_USE_OPTIMIZATION_LIBRARY(eigen);
#endif

G2O_USE_OPTIMIZATION_LIBRARY(dense);

using std::cout;
using std::endl;

class Sample {
 public:
  static int uniform(int from, int to) {
    return static_cast<int>(g2o::Sampler::uniformRand(from, to));
  }
};

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    cout << endl;
    cout << "Please type: " << endl;
    cout << "ba_demo [PIXEL_NOISE] [OUTLIER RATIO] [ROBUST_KERNEL] "
            "[STRUCTURE_ONLY] [DENSE]"
         << endl;
    cout << endl;
    cout << "PIXEL_NOISE: noise in image space (E.g.: 1)" << endl;
    cout << "OUTLIER_RATIO: probability of spurious observation  (default: 0.0)"
         << endl;
    cout << "ROBUST_KERNEL: use robust kernel (0 or 1; default: 0==false)"
         << endl;
    cout << "STRUCTURE_ONLY: performed structure-only BA to get better point "
            "initializations (0 or "
            "1; default: 0==false)"
         << endl;
    cout << "DENSE: Use dense solver (0 or 1; default: 0==false)" << endl;
    cout << endl;
    cout << "Note, if OUTLIER_RATIO is above 0, ROBUST_KERNEL should be set to "
            "1==true."
         << endl;
    cout << endl;
    exit(0);
  }

  double PIXEL_NOISE = atof(argv[1]);
  double OUTLIER_RATIO = 0.0;

  if (argc > 2) {
    OUTLIER_RATIO = atof(argv[2]);
  }

  bool ROBUST_KERNEL = false;
  if (argc > 3) {
    ROBUST_KERNEL = atoi(argv[3]) != 0;
  }
  bool STRUCTURE_ONLY = false;
  if (argc > 4) {
    STRUCTURE_ONLY = atoi(argv[4]) != 0;
  }

  bool DENSE = false;
  if (argc > 5) {
    DENSE = atoi(argv[5]) != 0;
  }

  cout << "PIXEL_NOISE: " << PIXEL_NOISE << endl;
  cout << "OUTLIER_RATIO: " << OUTLIER_RATIO << endl;
  cout << "ROBUST_KERNEL: " << ROBUST_KERNEL << endl;
  cout << "STRUCTURE_ONLY: " << STRUCTURE_ONLY << endl;
  cout << "DENSE: " << DENSE << endl;

  g2o::SparseOptimizer optimizer;
  optimizer.setVerbose(false);
  std::string solverName = "lm_fix6_3";
  if (DENSE) {
    solverName = "lm_dense6_3";
  } else {
#ifdef G2O_HAVE_CHOLMOD
    solverName = "lm_fix6_3_cholmod";
#else
    solverName = "lm_fix6_3";
#endif
  }

  g2o::OptimizationAlgorithmProperty solverProperty;
  optimizer.setAlgorithm(
      g2o::OptimizationAlgorithmFactory::instance()->construct(solverName,
                                                               solverProperty));

  std::vector<g2o::Vector3> true_points;
  for (size_t i = 0; i < 500; ++i) {
    true_points.emplace_back((g2o::Sampler::uniformRand(0., 1.) - 0.5) * 3,
                             g2o::Sampler::uniformRand(0., 1.) - 0.5,
                             g2o::Sampler::uniformRand(0., 1.) + 3);
  }

  double focal_length = 1000.;
  g2o::Vector2 principal_point(320., 240.);

  std::vector<g2o::SE3Quat, Eigen::aligned_allocator<g2o::SE3Quat> > true_poses;
  auto cam_params = std::make_shared<g2o::CameraParameters>(
      focal_length, principal_point, 0.);
  cam_params->setId(0);

  if (!optimizer.addParameter(cam_params)) {
    assert(false);
  }

  int vertex_id = 0;
  for (size_t i = 0; i < 15; ++i) {
    g2o::Vector3 trans(i * 0.04 - 1., 0, 0);

    Eigen::Quaterniond q;
    q.setIdentity();
    g2o::SE3Quat pose(q, trans);
    auto v_se3 = std::make_shared<g2o::VertexSE3Expmap>();
    v_se3->setId(vertex_id);
    if (i < 2) {
      v_se3->setFixed(true);
    }
    v_se3->setEstimate(pose);
    optimizer.addVertex(v_se3);
    true_poses.push_back(pose);
    vertex_id++;
  }
  int point_id = vertex_id;
  double sum_diff2 = 0;

  cout << endl;
  std::unordered_map<int, int> pointid_2_trueid;
  std::unordered_set<int> inliers;

  for (size_t i = 0; i < true_points.size(); ++i) {
    auto v_p = std::make_shared<g2o::VertexPointXYZ>();
    v_p->setId(point_id);
    v_p->setMarginalized(true);
    v_p->setEstimate(true_points.at(i) +
                     g2o::Vector3(g2o::Sampler::gaussRand(0., 1),
                                  g2o::Sampler::gaussRand(0., 1),
                                  g2o::Sampler::gaussRand(0., 1)));
    int num_obs = 0;
    for (auto& true_pose : true_poses) {
      g2o::Vector2 z = cam_params->cam_map(true_pose.map(true_points.at(i)));
      if (z[0] >= 0 && z[1] >= 0 && z[0] < 640 && z[1] < 480) {
        ++num_obs;
      }
    }
    if (num_obs >= 2) {
      optimizer.addVertex(v_p);
      bool inlier = true;
      for (size_t j = 0; j < true_poses.size(); ++j) {
        g2o::Vector2 z =
            cam_params->cam_map(true_poses.at(j).map(true_points.at(i)));

        if (z[0] >= 0 && z[1] >= 0 && z[0] < 640 && z[1] < 480) {
          double sam = g2o::Sampler::uniformRand(0., 1.);
          if (sam < OUTLIER_RATIO) {
            z = g2o::Vector2(Sample::uniform(0, 640), Sample::uniform(0, 480));
            inlier = false;
          }
          z += g2o::Vector2(g2o::Sampler::gaussRand(0., PIXEL_NOISE),
                            g2o::Sampler::gaussRand(0., PIXEL_NOISE));
          auto e = std::make_shared<g2o::EdgeProjectXYZ2UV>();
          e->setVertex(0, v_p);
          e->setVertex(1, optimizer.vertices().find(j)->second);
          e->setMeasurement(z);
          e->information() = g2o::Matrix2::Identity();
          if (ROBUST_KERNEL) {
            e->setRobustKernel(std::make_shared<g2o::RobustKernelHuber>());
          }
          e->setParameterId(0, 0);
          optimizer.addEdge(e);
        }
      }

      if (inlier) {
        inliers.insert(point_id);
        g2o::Vector3 diff = v_p->estimate() - true_points[i];

        sum_diff2 += diff.dot(diff);
      }
      pointid_2_trueid.insert(std::make_pair(point_id, i));
      ++point_id;
    }
  }
  cout << endl;
  optimizer.initializeOptimization();
  optimizer.setVerbose(true);
  if (STRUCTURE_ONLY) {
    g2o::StructureOnlySolver<3> structure_only_ba;
    cout << "Performing structure-only BA:" << endl;
    g2o::OptimizableGraph::VertexContainer points;
    for (const auto& it : optimizer.vertices()) {
      auto v =
          std::static_pointer_cast<g2o::OptimizableGraph::Vertex>(it.second);
      if (v->dimension() == 3) points.push_back(v);
    }
    structure_only_ba.calc(points, 10);
  }
  // optimizer.save("test.g2o");
  cout << endl;
  cout << "Performing full BA:" << endl;
  optimizer.optimize(10);
  cout << endl;
  cout << "Point error before optimisation (inliers only): "
       << sqrt(sum_diff2 / inliers.size()) << endl;
  sum_diff2 = 0;
  for (auto& it : pointid_2_trueid) {
    auto v_it = optimizer.vertices().find(it.first);
    if (v_it == optimizer.vertices().end()) {
      std::cerr << "Vertex " << it.first << " not in graph!" << endl;
      exit(-1);
    }
    auto* v_p = dynamic_cast<g2o::VertexPointXYZ*>(v_it->second.get());
    if (v_p == nullptr) {
      std::cerr << "Vertex " << it.first << "is not a PointXYZ!" << endl;
      exit(-1);
    }
    g2o::Vector3 diff = v_p->estimate() - true_points[it.second];
    if (inliers.find(it.first) == inliers.end()) continue;
    sum_diff2 += diff.dot(diff);
  }
  cout << "Point error after optimisation (inliers only): "
       << sqrt(sum_diff2 / inliers.size()) << endl;
  cout << endl;
}
