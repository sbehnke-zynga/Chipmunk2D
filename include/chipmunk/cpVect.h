/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CHIPMUNK_VECT_H
#define CHIPMUNK_VECT_H

#include "chipmunk_types.h"

/// @defgroup cpVect cpVect
/// Chipmunk's 2D vector type along with a handy 2D vector math lib.
/// @{

/// Constant for the zero vector.
static const cpVect cpvzero = {0.0f,0.0f};

/// Convenience constructor for cpVect structs.
static inline cpVect cpv(const cpFloat x, const cpFloat y)
{
	cpVect v = {x, y};
	return v;
}

/// Check if two vectors are equal. (Be careful when comparing floating point numbers!)
static inline bool cpveql(const cpVect v1, const cpVect v2)
{
	return (v1.x == v2.x && v1.y == v2.y);
}

/// Add two vectors
static inline cpVect cpvadd(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x + v2.x, v1.y + v2.y);
}

/// Subtract two vectors.
static inline cpVect cpvsub(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x - v2.x, v1.y - v2.y);
}

/// Negate a vector.
static inline cpVect cpvneg(const cpVect v)
{
	return cpv(-v.x, -v.y);
}

/// Scalar multiplication.
static inline cpVect cpvmult(const cpVect v, const cpFloat s)
{
	return cpv(v.x*s, v.y*s);
}

/// Vector dot product.
static inline cpFloat cpvdot(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

/// 2D vector cross product analog.
/// The cross product of 2D vectors results in a 3D vector with only a z component.
/// This function returns the magnitude of the z value.
static inline cpFloat cpvcross(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

/// Returns a perpendicular vector. (90 degree rotation)
static inline cpVect cpvperp(const cpVect v)
{
	return cpv(-v.y, v.x);
}

/// Returns a perpendicular vector. (-90 degree rotation)
static inline cpVect cpvrperp(const cpVect v)
{
	return cpv(v.y, -v.x);
}

/// Returns the vector projection of v1 onto v2.
static inline cpVect cpvproject(const cpVect v1, const cpVect v2)
{
	return cpvmult(v2, cpvdot(v1, v2)/cpvdot(v2, v2));
}

/// Returns the unit length vector for the given angle (in radians).
static inline cpVect cpvforangle(const cpFloat a)
{
	return cpv(cpfcos(a), cpfsin(a));
}

/// Returns the angular direction v is pointing in (in radians).
static inline cpFloat cpvtoangle(const cpVect v)
{
	return cpfatan2(v.y, v.x);
}

/// Uses complex number multiplication to rotate v1 by v2. Scaling will occur if v1 is not a unit vector.
static inline cpVect cpvrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
}

/// Inverse of cpvrotate().
static inline cpVect cpvunrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x + v1.y*v2.y, v1.y*v2.x - v1.x*v2.y);
}

/// Returns the squared length of v. Faster than cpvlength() when you only need to compare lengths.
static inline cpFloat cpvlengthsq(const cpVect v)
{
	return cpvdot(v, v);
}

/// Returns the length of v.
static inline cpFloat cpvlength(const cpVect v)
{
	return cpfsqrt(cpvdot(v, v));
}

/// Linearly interpolate between v1 and v2.
static inline cpVect cpvlerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	return cpvadd(cpvmult(v1, 1.0f - t), cpvmult(v2, t));
}

/// Returns a normalized copy of v.
static inline cpVect cpvnormalize(const cpVect v)
{
	// Neat trick I saw somewhere to avoid div/0.
	return cpvmult(v, 1.0f/(cpvlength(v) + CPFLOAT_MIN));
}

/// Spherical linearly interpolate between v1 and v2.
static inline cpVect
cpvslerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));
	
	if(omega < 1e-3){
		// If the angle between two vectors is very small, lerp instead to avoid precision issues.
		return cpvlerp(v1, v2, t);
	} else {
		cpFloat denom = 1.0f/cpfsin(omega);
		return cpvadd(cpvmult(v1, cpfsin((1.0f - t)*omega)*denom), cpvmult(v2, cpfsin(t*omega)*denom));
	}
}

/// Spherical linearly interpolate between v1 towards v2 by no more than angle a radians
static inline cpVect
cpvslerpconst(const cpVect v1, const cpVect v2, const cpFloat a)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));
	
	return cpvslerp(v1, v2, cpfmin(a, omega)/omega);
}

/// Clamp v to length len.
static inline cpVect cpvclamp(const cpVect v, const cpFloat len)
{
	return (cpvdot(v,v) > len*len) ? cpvmult(cpvnormalize(v), len) : v;
}

/// Linearly interpolate between v1 towards v2 by distance d.
static inline cpVect cpvlerpconst(cpVect v1, cpVect v2, cpFloat d)
{
	return cpvadd(v1, cpvclamp(cpvsub(v2, v1), d));
}

/// Returns the distance between v1 and v2.
static inline cpFloat cpvdist(const cpVect v1, const cpVect v2)
{
	return cpvlength(cpvsub(v1, v2));
}

/// Returns the squared distance between v1 and v2. Faster than cpvdist() when you only need to compare distances.
static inline cpFloat cpvdistsq(const cpVect v1, const cpVect v2)
{
	return cpvlengthsq(cpvsub(v1, v2));
}

/// Returns true if the distance between v1 and v2 is less than dist.
static inline bool cpvnear(const cpVect v1, const cpVect v2, const cpFloat dist)
{
	return cpvdistsq(v1, v2) < dist*dist;
}

/// @}

/// @defgroup cpMat2x2 cpMat2x2
/// 2x2 matrix type used for tensors and such.
/// @{

// NUKE
static inline cpMat2x2
cpMat2x2New(cpFloat a, cpFloat b, cpFloat c, cpFloat d)
{
	cpMat2x2 m = {a, b, c, d};
	return m;
}

static inline cpVect
cpMat2x2Transform(cpMat2x2 m, cpVect v)
{
	return cpv(v.x*m.a + v.y*m.b, v.x*m.c + v.y*m.d);
}

///@}

#endif
