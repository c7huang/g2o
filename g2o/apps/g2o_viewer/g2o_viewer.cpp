// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
//
// This file is part of g2o.
//
// g2o is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// g2o is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with g2o.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <iostream>

#include "g2o/apps/g2o_cli/dl_wrapper.h"
#include "g2o/apps/g2o_cli/g2o_common.h"
#include "g2o/core/optimizable_graph.h"
#include "g2o/stuff/command_args.h"
#include "run_g2o_viewer.h"

int main(int argc, char** argv) {
  g2o::OptimizableGraph::initMultiThreading();
  QApplication qapp(argc, argv);

  g2o::CommandArgs arg;
#ifndef G2O_DISABLE_DYNAMIC_LOADING_OF_LIBRARIES
  std::string dummy;
  arg.param("solverlib", dummy, "",
            "specify a solver library which will be loaded");
  arg.param("typeslib", dummy, "",
            "specify a types library which will be loaded");
  // loading the standard solver /  types
  g2o::DlWrapper dlTypesWrapper;
  g2o::loadStandardTypes(dlTypesWrapper, argc, argv);
  // register all the solvers
  g2o::DlWrapper dlSolverWrapper;
  g2o::loadStandardSolver(dlSolverWrapper, argc, argv);
#endif

  // run the viewer
  return g2o::RunG2OViewer::run(argc, argv, arg);
}
