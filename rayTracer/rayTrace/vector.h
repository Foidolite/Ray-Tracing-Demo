#pragma once
#include <stdexcept>
#include <vector>
#include <cmath>

template <typename component>
class Vector {
public:
	//constructors
	Vector() {}
	Vector(std::initializer_list<component> vals) : values(vals) {
		dimension = vals.size();
	}
	Vector(std::vector<component> vals);
	Vector(unsigned int dim, component vals[], bool delVals = false);
	Vector(unsigned int dim);
	
	//getters
	component getValue(unsigned int i) const {
		if (i < dimension) { return values[i]; }
		else { throw std::out_of_range("Vector: Attempt to index vector out of range."); }
	}
	unsigned int getDimension() const { return dimension; }
	//setters
	void setValue(component value, unsigned int i) { if (i < dimension) { values[i] = value; } }
	void setDimension(unsigned int d) { 
		dimension = d; 
		while (values.size > dimension) 
			values.pop_back();
		while (values.size < dimension)
			values.push_back(0);
	};

	//vector operations
	double magnitude();
	double magSquared();
	Vector unit();
	
	//member function overloads
	void operator += (const Vector b) {
		for (int i = 0; i < dimension; ++i)
			values[i] += b.values[i];
	}
	void operator -= (const Vector b) {
		for (int i = 0; i < dimension; ++i)
			values[i] -= b.values[i];
	}
	Vector operator-() const {
		std::vector<component> newValues;
		for (int i = 0; i < dimension; ++i) {
			newValues.push_back(-values[i]);
		}
		return Vector<component>(newValues);
	}
	void operator *= (double b) {
		for (int i = 0; i < dimension; ++i) {
			values[i] *= b;
		}
	};
	void operator /= (double b) {
		for (int i = 0; i < dimension; ++i) {
			values[i] /= b;
		}
	};
	component operator [] (int i) const { return getValue(i); }
	
private:
	std::vector<component> values;
	unsigned int dimension = 0;
};

//some constructors

template <typename component>
Vector<component>::Vector(std::vector<component> vals) {
	values = vals;
	dimension = vals.size();
}

template <typename component>
Vector<component>::Vector(unsigned int dim, component vals[], bool delVals) {
	dimension = dim;
	for (unsigned int i = 0; i < dim; ++i) {
		values.push_back(vals[i]);
	}
	if (delVals)
		delete[] vals;
}

template <typename component>
Vector<component>::Vector(unsigned int dim) {
	dimension = dim;
	while (values.size() < dim)
		values.push_back(0);
}

//vector functions

template <typename component>
double dot(const Vector<component> a, const Vector<component> b) {
	double product = 0;
	for (int i = 0; i < a.getDimension() && i < b.getDimension(); ++i)
		product += a.getValue(i) * b.getValue(i);
	return product;
}

template <typename component>
Vector<component> cross(const Vector<component> a, const Vector<component> b) {
	if (a.getDimension() != 3 || b.getDimension() != 3)
		throw std::logic_error("Cross product only well defined for 3 dimensions");

	std::vector<component> newValues;
	newValues.push_back(a.getValue(1)*b.getValue(2) - a.getValue(2)*b.getValue(1));
	newValues.push_back(a.getValue(2)*b.getValue(0) - a.getValue(0)*b.getValue(2));
	newValues.push_back(a.getValue(0)*b.getValue(1) - a.getValue(1)*b.getValue(0));

	return Vector<component>(newValues);
}

template <typename component>
double Vector<component>::magnitude() {
	double product = 0;
	for (int i = 0; i < dimension; ++i)
		product += values[i] * values[i];
	return sqrt(product);
}

template <typename component>
double Vector <component>::magSquared() {
	double product = 0;
	for (int i = 0; i < dimension; ++i)
		product += values[i] * values[i];
	return product;
}

template <typename component>
Vector<component> Vector<component>::unit() {
	return *this / magnitude();
}

//operator overloads

template <typename component>
Vector<component> operator + (const Vector<component> &a, const Vector<component> &b) {
	int dims = a.getDimension() < b.getDimension() ? a.getDimension() : b.getDimension();
	std::vector<component> newValues;
	for (int i = 0; i < dims; ++i)
		newValues.push_back(a[i] + b[i]);

	return Vector<component>(newValues);
}

template <typename component>
Vector<component> operator - (const Vector<component> &a, const Vector<component> &b) {
	int dims = a.getDimension() < b.getDimension() ? a.getDimension() : b.getDimension();
	std::vector<component> newValues;
	for (int i = 0; i < dims; ++i)
		newValues.push_back(a[i] - b[i]);

	return Vector<component>(newValues);
}

template <typename component>
Vector<component> operator * (double a, const Vector<component> &b) {
	std::vector<component> newValues;
	for (int i = 0; i < b.getDimension(); ++i)
		newValues.push_back(a * b[i]);

	return Vector<component>(newValues);
}

template <typename component>
Vector<component> operator * (const Vector<component> &a, double b) {
	std::vector<component> newValues;
	for (int i = 0; i < a.getDimension(); ++i)
		newValues.push_back(a[i] * b);

	return Vector<component>(newValues);
}

template <typename component>
Vector<component> operator / (const Vector<component> &a, double b) {
	std::vector<component> newValues;
	for (int i = 0; i < a.getDimension(); ++i)
		newValues.push_back(a[i] / b);

	return Vector<component>(newValues);
}

template <typename component>
bool operator == (const Vector<component> &a, const Vector<component> &b) {
	int dims = a.getDimension() < b.getDimension() ? a.getDimension() : b.getDimension();
	for (int i = 0; i < dims; ++i) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}

//print function

template <typename component>
std::ostream& operator << (std::ostream &out, const Vector<component> &data) {
	for (int i = 0; i < data.getDimension(); ++i)
		out << (float)data[i] << " ";
	return out;
}