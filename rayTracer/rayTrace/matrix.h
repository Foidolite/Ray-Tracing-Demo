#pragma once
#include <stdexcept>
#include <vector>
#include <cmath>

template <typename component>
class Matrix {
public:
	//constructors
	Matrix() {}
	Matrix(std::initializer_list < Vector<component>> cols) : columns(cols) { nRows = this->columns[0].getDimension(); nCols = cols.size(); verifyDims(); }
	Matrix(std::initializer_list <component *> cols) : columns(cols) { nRows = this->columns[0].getDimension(); nCols = cols.size(); verifyDims(); }
	Matrix(component * vals, unsigned int numCols, unsigned int numRows);
	Matrix(std::vector<std::vector<component>> vals);
	Matrix(std::vector<Vector<component>> vals);
	void verifyDims() {
		unsigned int colDim = columns[0].getDimension();
		for (unsigned int i = 1; i < nCols; ++i) {
			if (colDim != columns[i].getDimension()) { throw std::invalid_argument("All matrix columns must be of the same length"); }
		}
	}
	Matrix(unsigned int numRows, unsigned int numCols);

	//getters
	component getValueRC(unsigned int row, unsigned int column) const;
	component getValueCR(unsigned int column, unsigned int row) const;
	Vector<component> getColumn(unsigned int column) const;
	Vector<component> getRow(unsigned int row) const;
	unsigned int getNumRows() const { return nRows; };
	unsigned int getNumCols() const { return nCols; };

	//setters
	void setValueRC(component value, unsigned int row, unsigned int column);
	void setValueCR(component value, unsigned int column, unsigned int row);
	void setColumn(Vector<component> value, unsigned int column);

	//matrix operations
	double determinant();
	Matrix transpose();
	Matrix inverse();
	
	//row/column major conversions
	void toColArray(component * valArray);
	void toRowArray(component * valArray);
	static void toColArray(component * vals, component * retVals, unsigned int numCols, unsigned int numRows);
	static void toRowArray(component * vals, component * retVals, unsigned int numCols, unsigned int numRows);
	
	//member function overloads
	void operator +=(const Matrix b) {
		for (int i = 0; i < nCols; ++i)
			columns[i] += b[i];
	}
	void operator -=(const Matrix b) {
		for (int i = 0; i < nCols; ++i)
			columns[i] -= b[i];
	}
	Matrix operator-() const {
		std::vector<Vector<component>> newVals;
		for (int i = 0; i < nCols; ++i)
			newVals.push_back(-columns[i]);
		return Matrix<component>(newVals);
	}
	Matrix operator *=(double b) {
		for (int i = 0; i < nCols; ++i)
			columns[i] *= b;
	}
	Matrix operator /=(double b) {
		for (int i = 0; i < nCols; ++i)
			columns[i] /= b;
	}
	Vector<component> operator [] (int i) const { return getColumn(i); }

private:
	std::vector<Vector<component>> columns;
	unsigned int nRows, nCols;
};

//some constructors
template <typename component>
Matrix<component>::Matrix(component * vals, unsigned int numCols, unsigned int numRows) {
	nRows = numRows;
	nCols = numCols;
	for (int c = 0; c < nCols; ++c) {
		Vector<component> column(numRows);
		for (int r = 0; r < nRows; ++r)
			column.setValue(vals[c*numRows + r], r);

		columns.push_back(column);
	}
	this->verifyDims();
}

template <typename component>
Matrix<component>::Matrix(std::vector<std::vector<component>> vals) {
	if (vals.size() == 0 || vals[0].size() == 0)
		throw std::invalid_argument("Attempt to construct Matrix with empty vector.");

	nRows = vals[0].size();
	nCols = vals.size();
	for (unsigned int c = 0; c < vals.size(); ++c) {
		columns.push_back(Vector<component>(vals[c]));
	}

	verifyDims();
}

template <typename component>
Matrix<component>::Matrix(std::vector<Vector<component>> vals) {
	if (vals.size() == 0 || vals[0].getDimension() == 0)
		throw std::invalid_argument("Attempt to construct Matrix with empty vector.");

	nRows = vals[0].getDimension();
	nCols = vals.size();
	for (unsigned int c = 0; c < vals.size(); ++c) {
		columns.push_back(vals[c]);
	}

	verifyDims();
}

template <typename component>
Matrix<component>::Matrix(unsigned int numRows, unsigned int numCols) {
	nRows = numRows;
	nCols = numCols;
	while (columns.size() < numCols)
		columns.push_back(Vector<component>(numRows));
	verifyDims();
}

//some getters
template <typename component>
component Matrix<component>::getValueRC(unsigned int row, unsigned int column) const{
	if (row < nRows && column < nCols)
		return columns[column][row];
	else
		throw std::out_of_range("Attempt to index matrix out of range.");
}
template <typename component>
component Matrix<component>::getValueCR(unsigned int column, unsigned int row) const {
	return this->getValueRC(row, column);
}

template <typename component>
Vector<component> Matrix<component>::getColumn(unsigned int column) const {
	if (column < nCols)
		return columns[column];
	else
		throw std::out_of_range("Attempt to index matrix out of range.");
}

template <typename component>
Vector<component> Matrix<component>::getRow(unsigned int row) const {
	if (row < nRows) {
		std::vector<component> newRow;
		for (int c = 0; c < nCols; ++c) {
			newRow.push_back(columns[row]);
		}
		return Vector<component>(newRow);
	}
	else
		throw std::out_of_range("Attempt to index matrix out of range.");
}

//setters
template <typename component>
void Matrix<component>::setValueRC(component value, unsigned int row, unsigned int column) {
	if (row < nRows && column < nCols)
		columns[column].setValue(value, row);
	else
		throw std::out_of_range("Attempt to index matrix out of range.");
}
template <typename component>
void Matrix<component>::setValueCR(component value, unsigned int column, unsigned int row) {
	this->setValueRC(value, row, column);
}

template <typename component>
void Matrix<component>::setColumn(Vector<component> value, unsigned int column) {
	if (column < nCols)
		columns[column] = value;
	else
		throw std::out_of_range("Attempt to index matrix out of range.");
}

//matrix operations
template <typename component>
double Matrix<component>::determinant() {
	if (nRows != nCols || nRows == 1)
		std::invalid_argument("Determinant only defined for square matrices.");
	else {
		if (nRows == 2)
			return (double)columns[0][0] * (double)columns[1][1] - (double)columns[1][0] * (double)columns[0][1];
		else {
			double result = 0;
			for (unsigned int i = 0; i < nRows; ++i) {
				std::vector<std::vector<component>> conjugate;
				for (unsigned int c = 1; c < nCols; ++c) {
					std::vector<component> column;
					for (unsigned int r = 0; r < nRows; ++r) {
						if (r != i)
							column.push_back(columns[c][r]);
					}
					conjugate.push_back(column);
				}
				result += pow(-1,i)*columns[0][i]*Matrix(conjugate).determinant();
			}
			return result;
		}
	}
}

template <typename component>
Matrix<component> Matrix<component>::transpose() {
	std::vector<std::vector<component>> transposed;
	for (unsigned int r = 0; r < nRows; ++r) {
		std::vector<component> column;
		for (unsigned int c = 0; c < nCols; ++c) {
			column.push_back(columns[c][r]);
		}
		transposed.push_back(column);
	}
	return Matrix(transposed);
}

template <typename component>
Matrix<component> Matrix<component>::inverse() {
	if (this->determinant() == 0)
		throw std::invalid_argument("Matrix is not invertible.");
	std::vector<std::vector<component>> cof;
	for (unsigned int c = 0; c < nCols; ++c) {
		std::vector<component> column;
		for (unsigned int r = 0; r < nRows; ++r) {
			std::vector<std::vector<component>> det;
			for (unsigned int C = 0; C < nCols; ++C) {
				if (C != c) {
					std::vector<component> detCol;
					for (unsigned int R = 0; R < nRows; ++R) {
						if (R != r)
							detCol.push_back(columns[C][R]);
					}
					det.push_back(detCol);
				}
			}
			column.push_back(pow(-1, r + c)*Matrix<component>(det).determinant());
		}
		cof.push_back(column);
	}

	Matrix<component> adj = Matrix<component>(cof).transpose();
	return (1 / this->determinant())*adj;
}

//operator overloads
template <typename component>
Matrix<component> operator + (const Matrix<component> &a, const Matrix<component> &b) {
	std::vector<Vector<component>> newValues;
	int numCols = a.getNumCols() < b.getNumCols() ? a.getNumCols() : b.getNumCols();
	for (int c = 0; c < numCols; ++c)
		newValues.push_back(a[c] + b[c]);
	return Matrix<component>(newValues);
}

template <typename component>
Matrix<component> operator - (const Matrix<component> &a, const Matrix<component> &b) {
	std::vector<Vector<component>> newValues;
	int numCols = a.getNumCols() < b.getNumCols() ? a.getNumCols() : b.getNumCols();
	for (int c = 0; c < numCols; ++c)
		newValues.push_back(a[c] - b[c]);
	return Matrix<component>(newValues);
}

template <typename component>
Matrix<component> operator * (double a, const Matrix<component> &b) {
	std::vector<Vector<component>> newValues;
	for (unsigned int c = 0; c < b.getNumCols(); ++c) {
		newValues.push_back(a*b[c]);
	}
	return Matrix<component>(newValues);
}

template <typename component>
Matrix<component> operator * (const Matrix<component> &a, double b) {
	std::vector<Vector<component>> newValues;
	for (int c = 0; c < a.getNumCols(); ++c) {
		newValues.push_back(b*a[c]);
	}
	return Matrix<component>(newValues);
}

template <typename component>
Matrix<component> operator * (const Matrix<component>& a, const Matrix<component>& b) {
	if (b.getNumRows() != a.getNumCols())
		throw std::invalid_argument("Attempt to multiply matrices of incompatible dimension.");
	std::vector<Vector<component>> newValues;
	for (int c = 0; c < b.getNumCols(); ++c) {
		Vector<component> newCol(a[0].getDimension());
		for (int r = 0; r < b.getNumRows(); ++r) {
			newCol += a[r] * b[c][r];
		}
		newValues.push_back(newCol);
	}
	return Matrix<component>(newValues);
}

template <typename component>
Vector<component> operator * (const Matrix<component>& a, const Vector<component>& b) {
	if (b.getDimension() != a.getNumRows())
		throw std::invalid_argument("Attempt to multiply vector with matrix of incompatible dimension.");
	Vector<component> newVec(b.getDimension());
	for (int c = 0; c < a.getNumCols(); ++c) {
		newVec += a[c] * b[c];
	}
	return newVec;
}

template <typename component>
Vector<component> operator * (const Vector<component>& a, const Matrix<component>& b) {
	if (a.getDimension() != b.getNumRows())
		throw std::invalid_argument("Attempt to multiply vector with matrix of incompatible dimension.");
	Vector<component> newVec(a.getDimension());
	for (int c = 0; c < b.getNumCols(); ++c) {
		newVec += b[c] * a[c];
	}
	return newVec;
}

template <typename component>
Matrix<component> operator / (const Matrix<component> &a, double b) {
	std::vector<Vector<component>> newValues;
	for (int c = 0; c < a.getNumCols(); ++c) {
		newValues.push_back(a[c]/b);
	}
	return Matrix<component>(newValues);
}

//row/column major conversions
template <typename component>
void Matrix<component>::toColArray(component * valArray) {
	for (unsigned int c = 0; c < nCols; ++c) {
		for (unsigned int r = 0; r < nRows; ++r) {
			valArray[c*nCols + r] = columns[c][r];
		}
	}
}
template <typename component>
void Matrix<component>::toRowArray(component * valArray) {
	for (unsigned int c = 0; c < nCols; ++c) {
		for (unsigned int r = 0; r < nRows; ++r) {
			valArray[r*nRows + c] = columns[c][r];
		}
	}
}
template <typename component>
static void Matrix<component>::toColArray(component * vals, component * retVals, unsigned int numCols, unsigned int numRows) {
	for (unsigned int c = 0; c < numCols; ++c) {
		for (unsigned int r = 0; r < numRows; ++r) {
			retVals[c*numCols + r] = vals[r*numRows, + c];
		}
	}
}
template <typename component>
static void Matrix<component>::toRowArray(component * vals, component * retVals, unsigned int numCols, unsigned int numRows) {
	for (unsigned int c = 0; c < numCols; ++c) {
		for (unsigned int r = 0; r < numRows; ++r) {
			retVals[r*numRows, + c] = vals[c*numCols + r];
		}
	}
}

//print function

template <typename component>
std::ostream& operator << (std::ostream& out, const Matrix<component>& data) {
	for (int i = 0; i < data.getNumCols(); ++i)
		out << data[i] << std::endl;
	return out;
}