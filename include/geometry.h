#pragma once

#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>


template <size_t DIM, typename T>
struct vec{
    vec() {
        for (size_t i = 0; i < DIM; i++) {
            data[i] = T();
        }
    }
    T& operator[](const size_t i) {
        assert(i < DIM);
        return data[i];
    }
    const T& operator[](const size_t i) {
        assert(i < DIM);
        return data[i];
    }
private:
	T data[DIM];
};

typedef vec<2, float> Vec2f;
typedef vec<3, float> Vec3f;
typedef vec<3, int  > Vec3i;
typedef vec<3, unsigned char> Vec3uc;
typedef vec<4, float> Vec4f;


template <typename T> 
struct vec<2, T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2, T>(const vec<2, U>& v);
    T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }
    T x, y;
};

template <typename T> 
struct vec<3, T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    float norm() { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }
    T x, y, z;
};

template <typename T> 
struct vec<4, T> {
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    T& operator[](const size_t i) { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    const T& operator[](const size_t i) const { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    T x, y, z, w;
};

//Dot product
template<size_t DIM, typename T>
T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = 0; i < DIM; i++) {
        ret += lhs[i] * rhs[i];
    }
    return ret;
}

//Addition
template<size_t DIM, typename T>
vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; i++) {
        lhs[i] = lhs[i] + rhs[i];
    }
    return lhs;
}

template<size_t DIM, typename T>
void operator+=(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; i++) {
        lhs[i] += rhs[i];
    }
    return;
}


//Subtraction
template<size_t DIM, typename T>
vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = 0; i < DIM; i++) {
        lhs[i] = lhs[i] - rhs[i];
    }
    return lhs;
}

//Scaling
template<size_t DIM, typename T, typename U>
vec<DIM, T> operator*(const vec<DIM, T>& lhs, const U& rhs) {
    vec<DIM, T> ret;
    for (size_t i = 0; i < DIM; i++) {
        ret[i] = lhs[i] * rhs;
    }
    return ret;
}

template<size_t DIM, typename T, typename U>
vec<DIM, T> operator*(const U& rhs, const vec<DIM, T>& lhs) {
    return lhs * rhs;
}

//Negation
template<size_t DIM, typename T>
vec<DIM, T> operator-(const vec<DIM, T>& lhs) {
    return lhs * (T(-1));
}

//Cross product
template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}



