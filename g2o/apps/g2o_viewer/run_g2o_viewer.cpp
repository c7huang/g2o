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

#include "run_g2o_viewer.h"

#include <QApplication>
#include <QThread>

#include "g2o/apps/g2o_cli/dl_wrapper.h"
#include "g2o/config.h"
#include "g2o/core/factory.h"
#include "g2o/core/hyper_graph_action.h"
#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/stuff/command_args.h"
#include "g2o/stuff/opengl_wrapper.h"
#include "gui_hyper_graph_action.h"
#include "main_window.h"
#include "stream_redirect.h"

namespace g2o {

/**
 * \brief helper for calling usleep on any system using Qt
 */
class SleepThread : public QThread {
 public:  // make the protected methods publicly available
  using QThread::msleep;
  using QThread::usleep;
};

int RunG2OViewer::run(int argc, char** argv, CommandArgs& arg) {
  std::string inputFilename;
  std::string loadLookup;
  arg.param("renameTypes", loadLookup, "",
            "create a lookup for loading types into other types,\n\t "
            "TAG_IN_FILE=INTERNAL_TAG_FOR_TYPE,TAG2=INTERNAL2\n\t e.g., "
            "VERTEX_CAM=VERTEX_SE3:EXPMAP");
  arg.paramLeftOver("graph-input", inputFilename, "",
                    "graph file which will be processed", true);
  arg.parseArgs(argc, argv);

  MainWindow mw;
  mw.updateDisplayedSolvers();
  mw.updateRobustKernels();
  mw.show();

  // redirect the output that normally goes to cerr to the textedit in the
  // viewer
  StreamRedirect redirect(std::cerr, mw.plainTextEdit);

  // setting up the optimizer
  auto* optimizer = new SparseOptimizer();
  // Loading the input data
  if (!loadLookup.empty()) {
    optimizer->setRenamedTypesFromString(loadLookup);
  }
  mw.viewer->graph = optimizer;

  // set up the GUI action
  auto guiHyperGraphAction = std::make_shared<GuiHyperGraphAction>();
  guiHyperGraphAction->viewer = mw.viewer;
  // optimizer->addPostIterationAction(&guiHyperGraphAction);
  optimizer->addPreIterationAction(guiHyperGraphAction);

  if (!inputFilename.empty()) {
    mw.loadFromFile(QString::fromStdString(inputFilename));
  }

  QCoreApplication* myapp = QApplication::instance();
  while (mw.isVisible()) {
    guiHyperGraphAction->dumpScreenshots = mw.actionDump_Images->isChecked();
    if (myapp) QCoreApplication::processEvents();
    SleepThread::msleep(10);
  }

  delete optimizer;
  return 0;
}

}  // namespace g2o
