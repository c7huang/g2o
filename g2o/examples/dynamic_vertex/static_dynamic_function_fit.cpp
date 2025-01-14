// This example illustrates how to mix static and dynamic vertices in
// a graph.

// The goal is to fit a function of the form y(x) = f(x) + x^3 * p(x)
// to a set of data:
// * f(x) is a quadratic
// * p(x) is a polynomial whose dimensions are established at runtime.

// The ith observation consists of m_i pairs of values
// Z_i={(x_1,z_1),(x_2,z_2),(x_m_i,z_m_i)}, where z_i=y(x_i)+w_i, where
// w_i is additive white noise with information matrix Omega.

// In this example both the measurement edges and one vertex can
// changed dynamically.

#include <random>
#include <unsupported/Eigen/Polynomials>

#include "g2o/core/base_binary_edge.h"
#include "g2o/core/base_dynamic_vertex.h"
#include "g2o/core/base_unary_edge.h"
#include "g2o/core/base_vertex.h"
#include "g2o/core/block_solver.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/solvers/eigen/linear_solver_eigen.h"
#include "g2o/stuff/sampler.h"

// Declare the custom types used in the graph

// This vertex stores the coefficients of the f(x) polynomial. This is
// quadratic, and always has a degree of three.

class FPolynomialCoefficientVertex
    : public g2o::BaseVertex<3, Eigen::Vector3d> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  // Create the vertex
  FPolynomialCoefficientVertex() { setToOrigin(); }

  // Read the vertex
  bool read(std::istream& is) override {
    // Read the state
    return g2o::internal::readVector(is, estimate_);
  }

  // Write the vertex
  bool write(std::ostream& os) const override {
    return g2o::internal::writeVector(os, estimate_);
  }

  // Reset to zero
  void setToOriginImpl() override { estimate_.setZero(); }

  // Direct linear add
  void oplusImpl(const g2o::VectorX::MapType& update) override {
    estimate_ += update;
  }
};

// This vertex stores the coefficients of the p(x) polynomial. It is dynamic
// because we can change it at runtime.

class PPolynomialCoefficientVertex
    : public g2o::BaseDynamicVertex<Eigen::VectorXd> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  // Create the vertex
  PPolynomialCoefficientVertex() = default;

  // Read the vertex
  bool read(std::istream& is) override {
    // Read the dimension
    int dimension;
    is >> dimension;
    if (!is.good()) {
      return false;
    }

    // Set the dimension; we call the method here to ensure stuff like
    // cache and the workspace is setup
    setDimension(dimension);

    // Read the state
    return g2o::internal::readVector(is, estimate_);
  }

  // Write the vertex
  bool write(std::ostream& os) const override {
    os << estimate_.size() << " ";
    return g2o::internal::writeVector(os, estimate_);
  }

  // Reset to zero
  void setToOriginImpl() override { estimate_.setZero(); }

  // Direct linear add
  void oplusImpl(const g2o::VectorX::MapType& update) override {
    estimate_ += update;
  }

  // Resize the vertex state. In this case, we simply trash whatever
  // was there before.
  bool setDimensionImpl(int newDimension) override {
    estimate_.resize(newDimension);
    estimate_.setZero();
    return true;
  }
};

// Helper structure

struct FunctionObservation {
  Eigen::VectorXd x;
  Eigen::VectorXd z;
};

// The edge which encodes the observations

class MultipleValueEdge
    : public g2o::BaseBinaryEdge<Eigen::Dynamic, Eigen::VectorXd,
                                 FPolynomialCoefficientVertex,
                                 PPolynomialCoefficientVertex> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  MultipleValueEdge(const FunctionObservation& obs, double omega) : x_(obs.x) {
    setDimension(obs.z.size());
    setMeasurement(obs.z);
    const InformationType I = Eigen::MatrixXd::Identity(x_.size(), x_.size()) * omega;
    setInformation(I);
  }

  bool read(std::istream& is) override {
    Eigen::VectorXd z;
    g2o::internal::readVector(is, x_);
    g2o::internal::readVector(is, z);
    setMeasurement(z);

    return readInformationMatrix(is);
  }

  bool write(std::ostream& os) const override {
    g2o::internal::writeVector(os, x_);
    g2o::internal::writeVector(os, measurement_);
    return writeInformationMatrix(os);
  }

  // Compute the measurement from the eigen polynomial module
  void computeError() override {
    const FPolynomialCoefficientVertex* fvertex = vertexXnRaw<0>();
    const PPolynomialCoefficientVertex* pvertex = vertexXnRaw<1>();
    for (int i = 0; i < measurement_.size(); ++i) {
      const double x3 = pow(x_[i], 3);
      error_[i] = measurement_[i] -
                  Eigen::poly_eval(fvertex->estimate(), x_[i]) -
                  x3 * (Eigen::poly_eval(pvertex->estimate(), x_[i]));
    }
  }

 private:
  // The points that the polynomial is computed at
  Eigen::VectorXd x_;
};

int main(int argc, const char* argv[]) {
  // Random number generator
  std::default_random_engine generator;

  // Create the coefficients for the f-polynomial (all drawn randomly)
  Eigen::Vector3d f;
  for (int i = 0; i < 3; ++i) {
    f[i] = g2o::sampleUniform(-1, 1);
  }

  // Number of dimensions of the polynomial; the default is 4
  int polynomialDimension = 4;
  if (argc > 1) {
    polynomialDimension = atoi(argv[1]);
  }

  // Create the coefficients for the polynomial (all drawn randomly)
  Eigen::VectorXd p(polynomialDimension);
  for (int i = 0; i < polynomialDimension; ++i) {
    p[i] = g2o::sampleUniform(-1, 1);
  }

  std::cout << "Ground truth vectors f=" << f.transpose()
            << "; p=" << p.transpose() << std::endl;

  // The number of observations in the polynomial; the default is 6
  int obs = 6;
  if (argc > 2) {
    obs = atoi(argv[2]);
  }

  // The number of observations will be randomly sampled each time.

  // Sample the observations. This is a set of function
  // observations. The cardinality of each observation set is random.
  const double sigmaZ = 0.1;
  std::vector<FunctionObservation> observations(obs);
  std::uniform_int_distribution<int> cardinalitySampler(1, 5);

  for (int i = 0; i < obs; ++i) {
    FunctionObservation& fo = observations[i];
    const int numObs = cardinalitySampler(generator);
    fo.x.resize(numObs);
    fo.z.resize(numObs);
    for (int o = 0; o < numObs; ++o) {
      fo.x[o] = g2o::sampleUniform(-5, 5);
      const double x3 = pow(fo.x[o], 3);
      fo.z[o] = Eigen::poly_eval(f, fo.x[o]) +
                x3 * (Eigen::poly_eval(p, fo.x[o])) +
                sigmaZ * g2o::sampleGaussian();
    }
  }

  // Construct the graph and set up the solver and optimiser
  std::unique_ptr<g2o::BlockSolverX::LinearSolverType> linearSolver =
      g2o::make_unique<
          g2o::LinearSolverEigen<g2o::BlockSolverX::PoseMatrixType>>();

  // Set up the solver
  std::unique_ptr<g2o::BlockSolverX> blockSolver =
      g2o::make_unique<g2o::BlockSolverX>(std::move(linearSolver));

  // Set up the optimisation algorithm
  std::unique_ptr<g2o::OptimizationAlgorithm> optimisationAlgorithm(
      new g2o::OptimizationAlgorithmLevenberg(std::move(blockSolver)));

  // Create the graph and configure it
  std::unique_ptr<g2o::SparseOptimizer> optimizer =
      g2o::make_unique<g2o::SparseOptimizer>();
  optimizer->setVerbose(true);
  optimizer->setAlgorithm(std::move(optimisationAlgorithm));

  // Create the f vertex; its dimensions are known
  auto pf = std::make_shared<FPolynomialCoefficientVertex>();
  pf->setId(0);
  optimizer->addVertex(pf);

  // Create the vertex; note its dimension is currently is undefined
  auto pv = std::make_shared<PPolynomialCoefficientVertex>();
  pv->setId(1);
  optimizer->addVertex(pv);

  // Create the information matrix
  double omega = 1 / (sigmaZ * sigmaZ);

  // Create the edges
  for (int i = 0; i < obs; ++i) {
    auto mve = std::make_shared<MultipleValueEdge>(observations[i], omega);
    mve->setVertex(0, pf);
    mve->setVertex(1, pv);
    optimizer->addEdge(mve);
  }

  // Now run the same optimization problem for different choices of
  // dimension of the polynomial vertex. This shows how we can
  // dynamically change the vertex dimensions in an alreacy
  // constructed graph. Note that you must call initializeOptimization
  // before you can optimize after a state dimension has changed.
  for (int testDimension = 1; testDimension <= polynomialDimension;
       ++testDimension) {
    pv->setDimension(testDimension);
    optimizer->initializeOptimization();
    optimizer->optimize(10);
    std::cout << "Computed parameters: f=" << pf->estimate().transpose()
              << "; p=" << pv->estimate().transpose() << std::endl;
  }
  for (int testDimension = polynomialDimension - 1; testDimension >= 1;
       --testDimension) {
    pv->setDimension(testDimension);
    optimizer->initializeOptimization();
    optimizer->optimize(10);
    std::cout << "Computed parameters: f= " << pf->estimate().transpose()
              << "; p=" << pv->estimate().transpose() << std::endl;
  }
}
