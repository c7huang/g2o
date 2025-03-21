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

#include <iostream>

#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/stuff/command_args.h"

// we use the 2D and 3D SLAM types here
G2O_USE_TYPE_GROUP(slam2d);
G2O_USE_TYPE_GROUP(slam3d);
G2O_USE_OPTIMIZATION_LIBRARY(eigen);

int main(int argc, char** argv) {
  // Command line parsing
  int maxIterations;
  std::string outputFilename;
  std::string inputFilename;
  g2o::CommandArgs arg;
  arg.param("i", maxIterations, 10,
            "perform n iterations, if negative consider the gain");
  arg.param("o", outputFilename, "", "output final version of the graph");
  arg.paramLeftOver("graph-input", inputFilename, "",
                    "graph file which will be processed");
  arg.parseArgs(argc, argv);

  // NOTE: We skip to fix a vertex here, either this is stored in the file
  // itself or Levenberg will handle it.

  // create the optimizer to load the data and carry out the optimization
  g2o::SparseOptimizer optimizer;
  optimizer.setVerbose(true);

  // allocate the solver
  g2o::OptimizationAlgorithmProperty solverProperty;
  optimizer.setAlgorithm(
      g2o::OptimizationAlgorithmFactory::instance()->construct("lm_var",
                                                               solverProperty));

  std::ifstream ifs(inputFilename.c_str());
  if (!ifs) {
    std::cerr << "unable to open " << inputFilename << std::endl;
    return 1;
  }
  optimizer.load(ifs);
  optimizer.initializeOptimization();
  optimizer.optimize(maxIterations);

  if (!outputFilename.empty()) {
    if (outputFilename == "-") {
      std::cerr << "saving to stdout";
      optimizer.save(std::cout);
    } else {
      std::cerr << "saving " << outputFilename << " ... ";
      optimizer.save(outputFilename.c_str());
    }
    std::cerr << "done." << std::endl;
  }
  return 0;
}
