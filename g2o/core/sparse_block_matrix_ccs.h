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

#ifndef G2O_SPARSE_BLOCK_MATRIX_CCS_H
#define G2O_SPARSE_BLOCK_MATRIX_CCS_H

#include <Eigen/Core>
#include <cassert>
#include <unordered_map>
#include <vector>

#include "g2o/config.h"
#include "matrix_operations.h"

namespace g2o {

/**
 * \brief Sparse matrix which uses blocks
 *
 * This class is used as a const view on a SparseBlockMatrix
 * which allows a faster iteration over the elements of the
 * matrix.
 */
template <class MatrixType>
class SparseBlockMatrixCCS {
 public:
  //! this is the type of the elementary block, it is an Eigen::Matrix.
  using SparseMatrixBlock = MatrixType;

  //! columns of the matrix
  int cols() const {
    return !colBlockIndices_.empty() ? colBlockIndices_.back() : 0;
  }
  //! rows of the matrix
  int rows() const {
    return !rowBlockIndices_.empty() ? rowBlockIndices_.back() : 0;
  }

  /**
   * \brief A block within a column
   */
  struct RowBlock {
    int row{-1};        ///< row of the block
    MatrixType* block;  ///< matrix pointer for the block
    RowBlock() : block(0) {}
    RowBlock(int r, MatrixType* b) : row(r), block(b) {}
    bool operator<(const RowBlock& other) const { return row < other.row; }
  };
  using SparseColumn = std::vector<RowBlock>;

  SparseBlockMatrixCCS(const std::vector<int>& rowIndices,
                       const std::vector<int>& colIndices)
      : rowBlockIndices_(rowIndices), colBlockIndices_(colIndices) {}

  //! how many rows does the block at block-row r has?
  int rowsOfBlock(int r) const {
    return r ? rowBlockIndices_[r] - rowBlockIndices_[r - 1]
             : rowBlockIndices_[0];
  }

  //! how many cols does the block at block-col c has?
  int colsOfBlock(int c) const {
    return c ? colBlockIndices_[c] - colBlockIndices_[c - 1]
             : colBlockIndices_[0];
  }

  //! where does the row at block-row r start?
  int rowBaseOfBlock(int r) const { return r ? rowBlockIndices_[r - 1] : 0; }

  //! where does the col at block-col r start?
  int colBaseOfBlock(int c) const { return c ? colBlockIndices_[c - 1] : 0; }

  //! the block matrices per block-column
  const std::vector<SparseColumn>& blockCols() const { return blockCols_; }
  std::vector<SparseColumn>& blockCols() { return blockCols_; }

  //! indices of the row blocks
  const std::vector<int>& rowBlockIndices() const { return rowBlockIndices_; }

  //! indices of the column blocks
  const std::vector<int>& colBlockIndices() const { return colBlockIndices_; }

  void rightMultiply(number_t*& dest, const number_t* src) const {
    int destSize = cols();

    if (!dest) {
      dest = new number_t[destSize];
      memset(dest, 0, destSize * sizeof(number_t));
    }

    // map the memory by Eigen
    Eigen::Map<VectorX> destVec(dest, destSize);
    Eigen::Map<const VectorX> srcVec(src, rows());

#ifdef G2O_OPENMP
#pragma omp parallel for default(shared) schedule(dynamic, 10)
#endif
    for (int i = 0; i < static_cast<int>(blockCols_.size()); ++i) {
      int destOffset = colBaseOfBlock(i);
      for (auto it = blockCols_[i].begin(); it != blockCols_[i].end(); ++it) {
        const SparseMatrixBlock* a = it->block;
        int srcOffset = rowBaseOfBlock(it->row);
        // destVec += *a.transpose() * srcVec (according to the sub-vector
        // parts)
        internal::template atxpy<SparseMatrixBlock>(*a, srcVec, srcOffset,
                                                    destVec, destOffset);
      }
    }
  }

  /**
   * sort the blocks in each column
   */
  void sortColumns() {
    for (int i = 0; i < static_cast<int>(blockCols_.size()); ++i) {
      std::sort(blockCols_[i].begin(), blockCols_[i].end());
    }
  }

  /**
   * fill the CCS arrays of a matrix, arrays have to be allocated beforehand
   */
  int fillCCS(int* Cp, int* Ci, number_t* Cx,
              bool upperTriangle = false) const {
    assert(Cp && Ci && Cx && "Target destination is NULL");
    int nz = 0;
    for (size_t i = 0; i < blockCols_.size(); ++i) {
      int cstart = i ? colBlockIndices_[i - 1] : 0;
      int csize = colsOfBlock(i);
      for (int c = 0; c < csize; ++c) {
        *Cp = nz;
        for (auto it = blockCols_[i].begin(); it != blockCols_[i].end(); ++it) {
          const SparseMatrixBlock* b = it->block;
          int rstart = it->row ? rowBlockIndices_[it->row - 1] : 0;

          int elemsToCopy = b->rows();
          if (upperTriangle && rstart == cstart) elemsToCopy = c + 1;
          for (int r = 0; r < elemsToCopy; ++r) {
            *Cx++ = (*b)(r, c);
            *Ci++ = rstart++;
            ++nz;
          }
        }
        ++Cp;
      }
    }
    *Cp = nz;
    return nz;
  }

  /**
   * fill the CCS arrays of a matrix, arrays have to be allocated beforehand.
   * This function only writes the values and assumes that column and row
   * structures have already been written.
   */
  int fillCCS(number_t* Cx, bool upperTriangle = false) const {
    assert(Cx && "Target destination is NULL");
    number_t* CxStart = Cx;
    int cstart = 0;
    for (size_t i = 0; i < blockCols_.size(); ++i) {
      int csize = colBlockIndices_[i] - cstart;
      for (int c = 0; c < csize; ++c) {
        for (auto it = blockCols_[i].begin(); it != blockCols_[i].end(); ++it) {
          const SparseMatrixBlock* b = it->block;
          int rstart = it->row ? rowBlockIndices_[it->row - 1] : 0;

          int elemsToCopy = b->rows();
          if (upperTriangle && rstart == cstart) elemsToCopy = c + 1;
          memcpy(Cx, b->data() + c * b->rows(), elemsToCopy * sizeof(number_t));
          Cx += elemsToCopy;
        }
      }
      cstart = colBlockIndices_[i];
    }
    return Cx - CxStart;
  }

 protected:
  const std::vector<int>& rowBlockIndices_;  ///< vector of the indices of the
                                             ///< blocks along the rows.
  const std::vector<int>&
      colBlockIndices_;  ///< vector of the indices of the blocks along the cols
  std::vector<SparseColumn> blockCols_;  ///< the matrices stored in CCS order
};

/**
 * \brief Sparse matrix which uses blocks based on hash structures
 *
 * This class is used to construct the pattern of a sparse block matrix
 */
template <class MatrixType>
class SparseBlockMatrixHashMap {
 public:
  //! this is the type of the elementary block, it is an Eigen::Matrix.
  using SparseMatrixBlock = MatrixType;

  //! columns of the matrix
  int cols() const {
    return !colBlockIndices_.empty() ? colBlockIndices_.back() : 0;
  }
  //! rows of the matrix
  int rows() const {
    return !rowBlockIndices_.empty() ? rowBlockIndices_.back() : 0;
  }

  using SparseColumn = std::unordered_map<int, MatrixType*>;

  SparseBlockMatrixHashMap(const std::vector<int>& rowIndices,
                           const std::vector<int>& colIndices)
      : rowBlockIndices_(rowIndices), colBlockIndices_(colIndices) {}

  //! how many rows does the block at block-row r has?
  int rowsOfBlock(int r) const {
    return r ? rowBlockIndices_[r] - rowBlockIndices_[r - 1]
             : rowBlockIndices_[0];
  }

  //! how many cols does the block at block-col c has?
  int colsOfBlock(int c) const {
    return c ? colBlockIndices_[c] - colBlockIndices_[c - 1]
             : colBlockIndices_[0];
  }

  //! where does the row at block-row r start?
  int rowBaseOfBlock(int r) const { return r ? rowBlockIndices_[r - 1] : 0; }

  //! where does the col at block-col r start?
  int colBaseOfBlock(int c) const { return c ? colBlockIndices_[c - 1] : 0; }

  //! the block matrices per block-column
  const std::vector<SparseColumn>& blockCols() const { return blockCols_; }
  std::vector<SparseColumn>& blockCols() { return blockCols_; }

  //! indices of the row blocks
  const std::vector<int>& rowBlockIndices() const { return rowBlockIndices_; }

  //! indices of the column blocks
  const std::vector<int>& colBlockIndices() const { return colBlockIndices_; }

  /**
   * add a block to the pattern, return a pointer to the added block
   */
  MatrixType* addBlock(int r, int c, bool zeroBlock = false) {
    assert(c < static_cast<int>(blockCols_.size()) &&
           "accessing column which is not available");
    SparseColumn& sparseColumn = blockCols_[c];
    auto foundIt = sparseColumn.find(r);
    if (foundIt == sparseColumn.end()) {
      int rb = rowsOfBlock(r);
      int cb = colsOfBlock(c);
      auto* m = new MatrixType(rb, cb);
      if (zeroBlock) m->setZero();
      sparseColumn[r] = m;
      return m;
    }
    return foundIt->second;
  }

 protected:
  const std::vector<int>& rowBlockIndices_;  ///< vector of the indices of the
                                             ///< blocks along the rows.
  const std::vector<int>&
      colBlockIndices_;  ///< vector of the indices of the blocks along the cols
  std::vector<SparseColumn> blockCols_;  ///< the matrices stored in CCS order
};

}  // namespace g2o

#endif
