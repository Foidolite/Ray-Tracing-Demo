#pragma once
#include <stdexcept>
#include <vector>
#include <cmath>

template <typename component>
class Quaternion {
public:
	//constructors
	Quaternion() { re = 0; im = Vector<component>(3); }
	Quaternion(std::initializer_list<component> vals);
	Quaternion(std::vector<component> vals);
	Quaternion(component real, Vector<component> imaginary);

	//getters
	component Re() const { return re; }
	Vector<component> Im() const { return im; }

	//setters
	void setValues(component* vals) { re = vals[0]; im = Vector<component>(vals[1], vals[2], vals[3]); };
	void setValues(Vector<component> vals) {re = vals[0]; im = Vector<component>(vals[1], vals[2], vals[3]);};
	void setReal(component val) { re = val; };
	void setImag(Vector<component> vals) { im = vals; };

	//quaternion operations
	Quaternion<component> conjugate();
	double norm();
	Quaternion<component> unit();

	//member function overloads
	void operator += (const Quaternion b) {
		re += b.re;
		im += b.im;
	}
	void operator -= (const Quaternion b) {
		re -= b.re;
		im -= b.im;
	}
	Quaternion operator - () const {
		return Quaternion(-re, -im);
	}
	void operator *= (double b) {
		re *= b;
		im *= b;
	}
	void operator /= (double b) {
		re /= b;
		im /= b;
	}
	component operator [] (int i) {
		if (i == 0)
			return re;
		else
			return im[i - 1];
	}

private:
	component re;
	Vector<component> im;
};

//some constructors
template <typename component>
Quaternion<component>::Quaternion(std::initializer_list<component> vals) {
	component* start = vals.begin();
	re = start[0];
	im = { start[1],start[2],start[3] };
}

template <typename component>
Quaternion<component>::Quaternion(component real, Vector<component> imaginary) {
	re = real;
	im = imaginary;
}

template <typename component>
Quaternion<component>::Quaternion(std::vector<component> vals) {
	re = vals[0];
	im = Vector<component>(vals[1], vals[2], vals[3]);
}

//quaternion operations
template <typename component>
Quaternion<component> Quaternion<component>::conjugate() {
	return Quaternion(re, -im);
}

template <typename component>
double Quaternion<component>::norm() {
	Vector<component> tmp = { re, im[0],im[1],im[2] };
	return tmp.magnitude();
}

template <typename component>
Quaternion<component> Quaternion<component>::unit() {
	return this / this->norm();
}

//operator overloads
template <typename component>
Quaternion<component> operator + (Quaternion<component> a, Quaternion<component> b) {
	return Quaternion<component>(a.Re() + b.Re(), a.Im() + b.Im());
}

template <typename component>
Quaternion<component> operator - (Quaternion<component> a, Quaternion<component> b) {
	return Quaternion<component>(a.Re() - b.Re(), a.Im() - b.Im());
}

template <typename component>
Quaternion<component> operator * (Quaternion<component> a, Quaternion<component> b) {
	return Quaternion<component>(a.Re() * b.Re() - dot(a.Im(), b.Im()), a.Re() * b.Im() + b.Re() * a.Im() + cross(a.Im(), b.Im()));
}

template <typename component>
Quaternion<component> operator * (Quaternion<component> a, double b) {
	return Quaternion<component>(a.Re() * b, a.Im() * b);
}

template <typename component>
Quaternion<component> operator * (double a, Quaternion<component> b) {
	return Quaternion<component>(b.Re() * a, b.Im() * a);
}

template <typename component>
Quaternion<component> operator / (Quaternion<component> a, double b) {
	return Quaternion<component>(a.Re() / b, a.Im() / b);
}