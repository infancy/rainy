
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


// core/transform.cpp*
#include "transform.h"
//#include "error.h"		
//#include "interaction.h"

//namespace pbrt

namespace valley
{

// Matrix4x4 Method Definitions
bool SolveLinearSystem2x2(const Float A[2][2], const Float B[2], Float *x0,
                          Float *x1) {
    Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
    if (std::abs(det) < 1e-10f) return false;
    *x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
    *x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
    if (std::isnan(*x0) || std::isnan(*x1)) return false;
    return true;
}

Matrix4x4::Matrix4x4(Float mat[4][4]) { memcpy(m, mat, 16 * sizeof(Float)); }

Matrix4x4::Matrix4x4(Float t00, Float t01, Float t02, Float t03, Float t10,
                     Float t11, Float t12, Float t13, Float t20, Float t21,
                     Float t22, Float t23, Float t30, Float t31, Float t32,
                     Float t33) {
    m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
    m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
    m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
    m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
}

Matrix4x4 Transpose(const Matrix4x4 &m) {
    return Matrix4x4(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0], m.m[0][1],
                     m.m[1][1], m.m[2][1], m.m[3][1], m.m[0][2], m.m[1][2],
                     m.m[2][2], m.m[3][2], m.m[0][3], m.m[1][3], m.m[2][3],
                     m.m[3][3]);
}

Matrix4x4 Inverse(const Matrix4x4 &m) {
    int indxc[4], indxr[4];
    int ipiv[4] = {0, 0, 0, 0};
    Float minv[4][4];
    memcpy(minv, m.m, 4 * 4 * sizeof(Float));
    for (int i = 0; i < 4; i++) {
        int irow = 0, icol = 0;
        Float big = 0.f;
        // Choose pivot
        for (int j = 0; j < 4; j++) {
            if (ipiv[j] != 1) {
                for (int k = 0; k < 4; k++) {
                    if (ipiv[k] == 0) {
                        if (std::abs(minv[j][k]) >= big) {
                            big = Float(std::abs(minv[j][k]));
                            irow = j;
                            icol = k;
                        }
                    } else if (ipiv[k] > 1)
                        //Error("Singular matrix in MatrixInvert");
					std::cerr << "Singular matrix in MatrixInvert";
                }
            }
        }
        ++ipiv[icol];
        // Swap rows _irow_ and _icol_ for pivot
        if (irow != icol) {
            for (int k = 0; k < 4; ++k) std::swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (minv[icol][icol] == 0.f) //Error("Singular matrix in MatrixInvert");
			std::cerr << "Singular matrix in MatrixInvert";

        // Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
        Float pivinv = 1. / minv[icol][icol];
        minv[icol][icol] = 1.;
        for (int j = 0; j < 4; j++) minv[icol][j] *= pivinv;

        // Subtract this row from others to zero out their columns
        for (int j = 0; j < 4; j++) {
            if (j != icol) {
                Float save = minv[j][icol];
                minv[j][icol] = 0;
                for (int k = 0; k < 4; k++) minv[j][k] -= minv[icol][k] * save;
            }
        }
    }
    // Swap columns to reflect permutation
    for (int j = 3; j >= 0; j--) {
        if (indxr[j] != indxc[j]) {
            for (int k = 0; k < 4; k++)
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }
    return Matrix4x4(minv);
}

// Transform Method Definitions
void Transform::Print(FILE *f) const { m.Print(f); }

Transform Translate(const Vector3f &delta) {
    Matrix4x4 m(1, 0, 0, delta.x,
				0, 1, 0, delta.y,
				0, 0, 1, delta.z,
				0, 0, 0, 1);
    Matrix4x4 minv(1, 0, 0, -delta.x,
				   0, 1, 0, -delta.y,
				   0, 0, 1, -delta.z,
				   0, 0, 0, 1);
    return Transform(m, minv);
}

Transform Scale(Float x, Float y, Float z) {
    Matrix4x4 m(x, 0, 0, 0,
				0, y, 0, 0,
				0, 0, z, 0,
				0, 0, 0, 1);
    Matrix4x4 minv(1 / x, 0, 0, 0,
				   0, 1 / y, 0, 0,
				   0, 0, 1 / z, 0,
				   0, 0, 0, 1);
    return Transform(m, minv);
}

Transform RotateX(Float theta) {
    Float sinTheta = std::sin(Radians(theta));
    Float cosTheta = std::cos(Radians(theta));
    Matrix4x4 m(1, 0, 0, 0, 0, cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0,
                0, 0, 0, 1);
    return Transform(m, Transpose(m));
}

Transform RotateY(Float theta) {
    Float sinTheta = std::sin(Radians(theta));
    Float cosTheta = std::cos(Radians(theta));
    Matrix4x4 m(cosTheta, 0, sinTheta, 0, 0, 1, 0, 0, -sinTheta, 0, cosTheta, 0,
                0, 0, 0, 1);
    return Transform(m, Transpose(m));
}

Transform RotateZ(Float theta) {
    Float sinTheta = std::sin(Radians(theta));
    Float cosTheta = std::cos(Radians(theta));
    Matrix4x4 m(cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0, 0, 0, 1, 0,
                0, 0, 0, 1);
    return Transform(m, Transpose(m));
}

Transform Rotate(Float theta, const Vector3f &axis) {
    Vector3f a = Normalize(axis);
    Float sinTheta = std::sin(Radians(theta));
    Float cosTheta = std::cos(Radians(theta));
    Matrix4x4 m;
    // Compute rotation of first basis vector
    m.m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
    m.m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
    m.m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
    m.m[0][3] = 0;

    // Compute rotations of second and third basis vectors
    m.m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
    m.m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
    m.m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
    m.m[1][3] = 0;

    m.m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
    m.m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
    m.m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
    m.m[2][3] = 0;
    return Transform(m, Transpose(m));
}

Transform LookAt(const Point3f &pos, const Point3f &look, const Vector3f &up) {
    Matrix4x4 cameraToWorld;
    // Initialize fourth column of viewing matrix
    cameraToWorld.m[0][3] = pos.x;
    cameraToWorld.m[1][3] = pos.y;
    cameraToWorld.m[2][3] = pos.z;
    cameraToWorld.m[3][3] = 1;

    // Initialize first three columns of viewing matrix
    Vector3f dir = Normalize(look - pos);
    if (Cross(Normalize(up), dir).Length() == 0) {
		/*
        Error(
            "\"up\" vector (%f, %f, %f) and viewing direction (%f, %f, %f) "
            "passed to LookAt are pointing in the same direction.  Using "
            "the identity transformation.",
            up.x, up.y, up.z, dir.x, dir.y, dir.z);
		*/
		std::cerr << "up vector and viewing direction passed to LookAt are pointing in the same direction.";
        return Transform();
    }
    Vector3f left = Normalize(Cross(Normalize(up), dir));
    Vector3f newUp = Cross(dir, left);
    cameraToWorld.m[0][0] = left.x;
    cameraToWorld.m[1][0] = left.y;
    cameraToWorld.m[2][0] = left.z;
    cameraToWorld.m[3][0] = 0.;
    cameraToWorld.m[0][1] = newUp.x;
    cameraToWorld.m[1][1] = newUp.y;
    cameraToWorld.m[2][1] = newUp.z;
    cameraToWorld.m[3][1] = 0.;
    cameraToWorld.m[0][2] = dir.x;
    cameraToWorld.m[1][2] = dir.y;
    cameraToWorld.m[2][2] = dir.z;
    cameraToWorld.m[3][2] = 0.;
    return Transform(Inverse(cameraToWorld), cameraToWorld);
}

Bounds3f Transform::operator()(const Bounds3f &b) const {
    const Transform &M = *this;
    Bounds3f ret(M(Point3f(b.pMin.x, b.pMin.y, b.pMin.z)));
    ret = Union(ret, M(Point3f(b.pMax.x, b.pMin.y, b.pMin.z)));
    ret = Union(ret, M(Point3f(b.pMin.x, b.pMax.y, b.pMin.z)));
    ret = Union(ret, M(Point3f(b.pMin.x, b.pMin.y, b.pMax.z)));
    ret = Union(ret, M(Point3f(b.pMin.x, b.pMax.y, b.pMax.z)));
    ret = Union(ret, M(Point3f(b.pMax.x, b.pMax.y, b.pMin.z)));
    ret = Union(ret, M(Point3f(b.pMax.x, b.pMin.y, b.pMax.z)));
    ret = Union(ret, M(Point3f(b.pMax.x, b.pMax.y, b.pMax.z)));
    return ret;
}

Transform Transform::operator*(const Transform &t2) const {
	//m3 = m1*m2, m3Inv = m2Inv*m1Inv
	//m3 * m3Inve = m1*m2*m2Inv*m1Inv
    return Transform(Matrix4x4::Mul(m, t2.m), 
						Matrix4x4::Mul(t2.mInv, mInv));
}

bool Transform::SwapsHandedness() const {
    Float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
                m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
                m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return det < 0;
}
/*
SurfaceInteraction Transform::operator()(const SurfaceInteraction &si) const {
    SurfaceInteraction ret;
    // Transform _p_ and _pError_ in _SurfaceInteraction_
    ret.p = (*this)(si.p, si.pError, &ret.pError);

    // Transform remaining members of _SurfaceInteraction_
    const Transform &t = *this;
    ret.n = Normalize(t(si.n));
    ret.wo = Normalize(t(si.wo));
    ret.time = si.time;
    ret.mediumInterface = si.mediumInterface;
    ret.uv = si.uv;
    ret.shape = si.shape;
    ret.dpdu = t(si.dpdu);
    ret.dpdv = t(si.dpdv);
    ret.dndu = t(si.dndu);
    ret.dndv = t(si.dndv);
    ret.shading.n = Normalize(t(si.shading.n));
    ret.shading.dpdu = t(si.shading.dpdu);
    ret.shading.dpdv = t(si.shading.dpdv);
    ret.shading.dndu = t(si.shading.dndu);
    ret.shading.dndv = t(si.shading.dndv);
    ret.dudx = si.dudx;
    ret.dvdx = si.dvdx;
    ret.dudy = si.dudy;
    ret.dvdy = si.dvdy;
    ret.dpdx = t(si.dpdx);
    ret.dpdy = t(si.dpdy);
    ret.bsdf = si.bsdf;
    ret.bssrdf = si.bssrdf;
    ret.primitive = si.primitive;
    //    ret.n = Faceforward(ret.n, ret.shading.n);
    ret.shading.n = Faceforward(ret.shading.n, ret.n);
    return ret;
}
*/
Transform Orthographic(Float zNear, Float zFar) {
    return Scale(1, 1, 1 / (zFar - zNear)) * Translate(Vector3f(0, 0, -zNear));
}

Transform Perspective(Float fov, Float n, Float f) {
    // Perform projective divide for perspective projection
	//(x,y,z,1) -> (x,y,f(z-n)/(f-n),z) -> (x/z,y/z,f(z-n)/z(f-n),1)
    Matrix4x4 persp(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, f / (f - n), -f * n / (f - n),
                    0, 0, 1, 0);	

    // Scale canonical perspective view to specified field of view
    Float invTanAng = 1 / std::tan(Radians(fov) / 2);
    return Scale(invTanAng, invTanAng, 1) * Transform(persp);
}

// Interval Definitions
class Interval {
  public:
    // Interval Public Methods
    Interval(Float v) : low(v), high(v) {}
    Interval(Float v0, Float v1)
        : low(std::min(v0, v1)), high(std::max(v0, v1)) {}
    Interval operator+(const Interval &i) const {
        return Interval(low + i.low, high + i.high);
    }
    Interval operator-(const Interval &i) const {
        return Interval(low - i.high, high - i.low);
    }
    Interval operator*(const Interval &i) const {
        return Interval(std::min(std::min(low * i.low, high * i.low),
                                 std::min(low * i.high, high * i.high)),
                        std::max(std::max(low * i.low, high * i.low),
                                 std::max(low * i.high, high * i.high)));
    }
    Float low, high;
};

inline Interval Sin(const Interval &i) {
    CHECK_GE(i.low, 0);
    CHECK_LE(i.high, 2.0001 * Pi);
    Float sinLow = std::sin(i.low), sinHigh = std::sin(i.high);
    if (sinLow > sinHigh) std::swap(sinLow, sinHigh);
    if (i.low < Pi / 2 && i.high > Pi / 2) sinHigh = 1.;
    if (i.low < (3.f / 2.f) * Pi && i.high > (3.f / 2.f) * Pi) sinLow = -1.;
    return Interval(sinLow, sinHigh);
}

inline Interval Cos(const Interval &i) {
    CHECK_GE(i.low, 0);
    CHECK_LE(i.high, 2.0001 * Pi);
    Float cosLow = std::cos(i.low), cosHigh = std::cos(i.high);
    if (cosLow > cosHigh) std::swap(cosLow, cosHigh);
    if (i.low < Pi && i.high > Pi) cosLow = -1.;
    return Interval(cosLow, cosHigh);
}

void IntervalFindZeros(Float c1, Float c2, Float c3, Float c4, Float c5,
                       Float theta, Interval tInterval, Float *zeros,
                       int *zeroCount, int depth = 8) {
    // Evaluate motion derivative in interval form, return if no zeros
    Interval range = Interval(c1) +
                     (Interval(c2) + Interval(c3) * tInterval) *
                         Cos(Interval(2 * theta) * tInterval) +
                     (Interval(c4) + Interval(c5) * tInterval) *
                         Sin(Interval(2 * theta) * tInterval);
    if (range.low > 0. || range.high < 0. || range.low == range.high) return;
    if (depth > 0) {
        // Split _tInterval_ and check both resulting intervals
        Float mid = (tInterval.low + tInterval.high) * 0.5f;
        IntervalFindZeros(c1, c2, c3, c4, c5, theta,
                          Interval(tInterval.low, mid), zeros, zeroCount,
                          depth - 1);
        IntervalFindZeros(c1, c2, c3, c4, c5, theta,
                          Interval(mid, tInterval.high), zeros, zeroCount,
                          depth - 1);
    } else {
        // Use Newton's method to refine zero
        Float tNewton = (tInterval.low + tInterval.high) * 0.5f;
        for (int i = 0; i < 4; ++i) {
            Float fNewton =
                c1 + (c2 + c3 * tNewton) * std::cos(2.f * theta * tNewton) +
                (c4 + c5 * tNewton) * std::sin(2.f * theta * tNewton);
            Float fPrimeNewton = (c3 + 2 * (c4 + c5 * tNewton) * theta) *
                                     std::cos(2.f * tNewton * theta) +
                                 (c5 - 2 * (c2 + c3 * tNewton) * theta) *
                                     std::sin(2.f * tNewton * theta);
            if (fNewton == 0 || fPrimeNewton == 0) break;
            tNewton = tNewton - fNewton / fPrimeNewton;
        }
        if (tNewton >= tInterval.low - 1e-3f &&
            tNewton < tInterval.high + 1e-3f) {
            zeros[*zeroCount] = tNewton;
            (*zeroCount)++;
        }
    }
}

// AnimatedTransform Method Definitions
}  // namespace pbrt
