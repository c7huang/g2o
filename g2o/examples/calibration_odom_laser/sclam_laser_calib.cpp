// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G.Grisetti, W. Burgard
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

#include <csignal>
#include <fstream>
#include <iostream>
#include <map>

#include "g2o/core/factory.h"
#include "g2o/core/hyper_dijkstra.h"
#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/stuff/color_macros.h"
#include "g2o/stuff/command_args.h"
#include "g2o/stuff/filesys_tools.h"
#include "g2o/stuff/macros.h"
#include "g2o/stuff/string_tools.h"
#include "g2o/stuff/timeutil.h"
#include "g2o/types/data/types_data.h"
#include "g2o/types/sclam2d/types_sclam2d.h"
#include "gm2dl_io.h"

static bool hasToStop = false;

G2O_USE_OPTIMIZATION_LIBRARY(eigen);
G2O_USE_TYPE_GROUP(slam2d);

using std::cerr;
using std::endl;
using std::string;

void sigquit_handler(int sig) {
  if (sig == SIGINT) {
    hasToStop = true;
    static int cnt = 0;
    if (cnt++ == 2) {
      cerr << __PRETTY_FUNCTION__ << " forcing exit" << endl;
      exit(1);
    }
  }
}

int main(int argc, char** argv) {
  int maxIterations;
  bool verbose;
  string inputFilename;
  string gnudump;
  string outputfilename;
  bool initialGuess;
  // command line parsing
  g2o::CommandArgs commandLineArguments;
  commandLineArguments.param("i", maxIterations, 10, "perform n iterations");
  commandLineArguments.param("v", verbose, false,
                             "verbose output of the optimization process");
  commandLineArguments.param("guess", initialGuess, false,
                             "initial guess based on spanning tree");
  commandLineArguments.param("gnudump", gnudump, "",
                             "dump to gnuplot data file");
  commandLineArguments.param("o", outputfilename, "",
                             "output final version of the graph");
  commandLineArguments.paramLeftOver("gm2dl-input", inputFilename, "",
                                     "gm2dl file which will be processed");

  commandLineArguments.parseArgs(argc, argv);

  g2o::OptimizationAlgorithmFactory* solverFactory =
      g2o::OptimizationAlgorithmFactory::instance();

  g2o::SparseOptimizer optimizer;
  optimizer.setVerbose(verbose);
  optimizer.setForceStopFlag(&hasToStop);

  g2o::OptimizationAlgorithmProperty solverProperty;
  optimizer.setAlgorithm(solverFactory->construct("lm_var", solverProperty));

  // loading
  if (!g2o::Gm2dlIO::readGm2dl(inputFilename, optimizer, false)) {
    cerr << "Error while loading gm2dl file" << endl;
  }

  auto laserOffset = std::dynamic_pointer_cast<g2o::VertexSE2>(
      optimizer.vertex(std::numeric_limits<int>::max()));
  // laserOffset->setEstimate(SE2()); // set to Identity
  if (laserOffset) {
    cerr << "Initial laser offset "
         << laserOffset->estimate().toVector().transpose() << endl;
  }
  bool gaugeFreedom = optimizer.gaugeFreedom();

  auto gauge = optimizer.findGauge();
  if (gaugeFreedom) {
    if (!gauge) {
      cerr << "# cannot find a vertex to fix in this thing" << endl;
      return 2;
    }
    cerr << "# graph is fixed by node " << gauge->id() << endl;
    gauge->setFixed(true);

  } else {
    cerr << "# graph is fixed by priors" << endl;
  }

  // sanity check
  auto pointerWrapper =
      std::shared_ptr<g2o::HyperGraph>(&optimizer, [](g2o::HyperGraph*) {});
  g2o::HyperDijkstra d(pointerWrapper);
  g2o::UniformCostFunction f;
  d.shortestPaths(gauge, f);
  // cerr << PVAR(d.visited().size()) << endl;

  if (d.visited().size() != optimizer.vertices().size()) {
    cerr << CL_RED("Warning: d.visited().size() != optimizer.vertices().size()")
         << endl;
    cerr << "visited: " << d.visited().size() << endl;
    cerr << "vertices: " << optimizer.vertices().size() << endl;
    for (auto& it : optimizer.vertices()) {
      if (d.visited().count(it.second) == 0) {
        auto* v = static_cast<g2o::OptimizableGraph::Vertex*>(it.second.get());
        cerr << "\t unvisited vertex " << it.first << " "
             << static_cast<void*>(v) << endl;
        v->setFixed(true);
      }
    }
  }

  optimizer.initializeOptimization();
  optimizer.computeActiveErrors();
  cerr << "Initial chi2 = " << FIXED(optimizer.chi2()) << endl;

  // if (guessCostFunction)
  // optimizer.computeInitialGuess(guessCostFunction);

  signal(SIGINT, sigquit_handler);

  int i = optimizer.optimize(maxIterations);
  if (maxIterations > 0 && !i) {
    cerr << "optimize failed, result might be invalid" << endl;
  }

  if (laserOffset) {
    cerr << "Calibrated laser offset "
         << laserOffset->estimate().toVector().transpose() << endl;
  }

  if (!outputfilename.empty()) {
    g2o::Gm2dlIO::updateLaserData(optimizer);
    cerr << "Writing " << outputfilename << " ... ";
    bool writeStatus = g2o::Gm2dlIO::writeGm2dl(outputfilename, optimizer);
    cerr << (writeStatus ? "done." : "failed") << endl;
  }

  if (!gnudump.empty()) {
    std::ofstream fout(gnudump.c_str());
    for (const auto& it : optimizer.vertices()) {
      auto* v = dynamic_cast<g2o::VertexSE2*>(it.second.get());
      fout << v->estimate().toVector().transpose() << endl;
    }
  }

  return 0;
}
