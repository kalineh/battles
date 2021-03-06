/*
 * Copyright (c) 2016 Ilya Kaliman
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Project: https://github.com/ilyak/linalg
 */

#ifndef LINALG_H_INCLUDED
#define LINALG_H_INCLUDED

#include <math.h>

#ifdef __cplusplus
namespace linalg {
#endif

#define LINALG_SAFE_EPSILON 1.0e-06F

#ifdef LINALG_DOUBLE_PRECISION
typedef double real;
#define LINALG_SQRT sqrt
#define LINALG_ABS abs
#define LINALG_MIN fmin
#define LINALG_MAX fmax
#define LINALG_ATAN2 atan2
#define LINALG_SIN sin
#define LINALG_COS cos
#else /* LINALG_DOUBLE_PRECISION */
typedef float real;
#define LINALG_SQRT sqrtf
#define LINALG_ABS fabsf
#define LINALG_MIN fminf
#define LINALG_MAX fmaxf
#define LINALG_ATAN2 atan2f
#define LINALG_SIN sinf
#define LINALG_COS cosf
#endif /* LINALG_DOUBLE_PRECISION */


typedef struct {
	int x, y;
} v2i;

typedef struct {
	int x, y, z;
} v3i;

typedef struct {
	real x, y;
} v2;

typedef struct {
	real x, y, z;
} v3;

typedef struct {
	real x, y, z, w;
} v4;

typedef struct {
	real xx, xy;
	real yx, yy;
} m22;

typedef struct {
	real xx, xy, xz;
	real yx, yy, yz;
	real zx, zy, zz;
} m33;

typedef struct {
	real w, x, y, z;
} q4;

static inline int
realeq(real a, real b, real eps)
{
	return (fabs((double)(a - b)) < (double)eps);
}

static inline v2i
v2inew(int x, int y)
{
	v2i v = { x, y };
	return (v);
}

static inline v2i
v2izero(void)
{
	return v2inew(0, 0);
}

static inline int
v2iidx(v2i v, unsigned i)
{
	return ((int *)&v)[i];
}

static inline v2i
v2iadd(v2i a, v2i b)
{
	return v2inew(a.x + b.x, a.y + b.y);
}

static inline v2i
v2isub(v2i a, v2i b)
{
	return v2inew(a.x - b.x, a.y - b.y);
}

static inline v2i
v2ineg(v2i v)
{
	return v2inew(-v.x, -v.y);
}

static inline v2i
v2imul(v2i v, real s)
{
	return v2inew((int)(v.x * s), (int)(v.y * s));
}

static inline v2
v2new(real x, real y)
{
	v2 v = { x, y };
	return (v);
}

static inline v2
v2zero(void)
{
	return v2new(0, 0);
}

static inline real
v2idx(v2 v, unsigned i)
{
	return ((real *)&v)[i];
}

static inline v2
v2add(v2 a, v2 b)
{
	return v2new(a.x + b.x, a.y + b.y);
}

static inline v2
v2sub(v2 a, v2 b)
{
	return v2new(a.x - b.x, a.y - b.y);
}

static inline v2
v2neg(v2 v)
{
	return v2new(-v.x, -v.y);
}

static inline v2
v2mul(v2 v, real s)
{
	return v2new(v.x * s, v.y * s);
}

static inline v2
v2mulv(v2 v, v2 s)
{
	return v2new(v.x * s.x, v.y * s.y);
}

static inline v2
v2div(v2 v, real s)
{
	return v2new(v.x / s, v.y / s);
}

static inline real
v2dot(v2 a, v2 b)
{
	return (a.x * b.x + a.y * b.y);
}

static inline v2
v2proj(v2 a, v2 b)
{
	return v2mul(b, v2dot(a, b) / v2dot(b, b));
}

static inline v2
v2projsafe(v2 a, v2 b)
{
	const real dotab = v2dot(a, b);
	const real dotbb = v2dot(b, b);
	if (realeq(dotbb, 0, LINALG_SAFE_EPSILON))
		return v2zero();
	return v2mul(b, dotab / dotbb);
}

static inline real
v2lensq(v2 v)
{
	return v2dot(v, v);
}

static inline real
v2len(v2 v)
{
	return LINALG_SQRT(v2lensq(v));
}

static inline real
v2lensafe(v2 v)
{
	real lensq = v2lensq(v);
	if (realeq(lensq, 0.0f, LINALG_SAFE_EPSILON))
		return 0;
	return LINALG_SQRT(lensq);
}

static inline v2
v2unit(v2 v)
{
	return v2div(v, v2len(v));
}

static inline v2
v2unitsafe(v2 v)
{
	real lensq = v2lensq(v);
	if (realeq(lensq, 0.0f, LINALG_SAFE_EPSILON))
		return v2zero();
	real len = LINALG_SQRT(lensq);
	v2 result = v2div(v, len);
	return result;
}

static inline void
v2unitlensafe(v2 v, v2* ounit, real* olen)
{
	real lensq = v2lensq(v);
	if (realeq(lensq, 0.0f, LINALG_SAFE_EPSILON))
	{
		*ounit = v2zero();
		*olen = 0;
		return;
	}		

	real len = LINALG_SQRT(lensq);
	*ounit = v2div(v, len);
	*olen = len;
}

static inline real
v2distsq(v2 a, v2 b)
{
	return v2lensq(v2sub(a, b));
}

static inline real
v2dist(v2 a, v2 b)
{
	return v2len(v2sub(a, b));
}

static inline int
v2eq(v2 a, v2 b, real eps)
{
	if (!realeq(a.x, b.x, eps)) return (0);
	if (!realeq(a.y, b.y, eps)) return (0);
	return (1);
}

static inline v3
v3new(real x, real y, real z)
{
	v3 v = { x, y, z };
	return (v);
}

static inline v3
v3zero(void)
{
	return v3new(0, 0, 0);
}

static inline real
v3idx(v3 v, unsigned i)
{
	return ((real *)&v)[i];
}

static inline v3
v3add(v3 a, v3 b)
{
	return v3new(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline v3
v3sub(v3 a, v3 b)
{
	return v3new(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline v3
v3neg(v3 v)
{
	return v3new(-v.x, -v.y, -v.z);
}

static inline v3
v3mul(v3 v, real s)
{
	return v3new(v.x * s, v.y * s, v.z * s);
}

static inline v3
v3div(v3 v, real s)
{
	return v3new(v.x / s, v.y / s, v.z / s);
}

static inline v3
v3cross(v3 a, v3 b)
{
	return v3new(a.y * b.z - a.z * b.y,
		     a.z * b.x - a.x * b.z,
		     a.x * b.y - a.y * b.x);
}

static inline real
v3dot(v3 a, v3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

static inline real
v3lensq(v3 v)
{
	return v3dot(v, v);
}

static inline real
v3len(v3 v)
{
	return LINALG_SQRT(v3lensq(v));
}

static inline real
v3lensafe(v3 v)
{
	real lensq = v3lensq(v);
	if (realeq(lensq, 0.0f, LINALG_SAFE_EPSILON))
		return 0;
	return LINALG_SQRT(lensq);
}

static inline v3
v3unit(v3 v)
{
	return v3div(v, v3len(v));
}

static inline v3
v3unitsafe(v3 v)
{
	real lensq = v3lensq(v);
	if (realeq(lensq, 0.0f, LINALG_SAFE_EPSILON))
		return v3zero();
	real len = LINALG_SQRT(lensq);
	v3 result = v3div(v, len);
	return result;
}

static inline real
v3distsq(v3 a, v3 b)
{
	return v3lensq(v3sub(a, b));
}

static inline real
v3dist(v3 a, v3 b)
{
	return v3len(v3sub(a, b));
}

static inline int
v3eq(v3 a, v3 b, real eps)
{
	if (!realeq(a.x, b.x, eps)) return (0);
	if (!realeq(a.y, b.y, eps)) return (0);
	if (!realeq(a.z, b.z, eps)) return (0);
	return (1);
}

static inline v4
v4new(real x, real y, real z, real w)
{
	v4 v = { x, y, z, w };
	return (v);
}

static inline v4
v4zero(void)
{
	return v4new(0, 0, 0, 0);
}

static inline real
v4idx(v4 v, unsigned i)
{
	return ((real *)&v)[i];
}

static inline v4
v4add(v4 a, v4 b)
{
	return v4new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

static inline v4
v4sub(v4 a, v4 b)
{
	return v4new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

static inline v4
v4neg(v4 v)
{
	return v4new(-v.x, -v.y, -v.z, -v.w);
}

static inline v4
v4mul(v4 v, real s)
{
	return v4new(v.x * s, v.y * s, v.z * s, v.w * s);
}

static inline v4
v4div(v4 v, real s)
{
	return v4new(v.x / s, v.y / s, v.z / s, v.w / s);
}

static inline v2 v3xx(v3 v) { return v2new(v.x, v.x); } 
static inline v2 v3xy(v3 v) { return v2new(v.x, v.y); } 
static inline v2 v3yy(v3 v) { return v2new(v.y, v.y); } 

static inline v3 v2xy0(v2 v) { return v3new(v.x, v.y, 0); } 
static inline v4 v2xy00(v2 v) { return v4new(v.x, v.y, 0, 0); } 

static inline v2 v4xy(v4 v) { return v2new(v.x, v.y); } 
static inline v3 v4xyz(v4 v) { return v3new(v.x, v.y, v.z); } 


static inline m22
m22new(real xx, real xy, real yx, real yy)
{
	m22 m = { xx, xy, yx, yy };
	return (m);
}

static inline m22
m22zero(void)
{
	return m22new(0, 0, 0, 0);
}

static inline real
m22idx(m22 m, unsigned i, unsigned j)
{
	return ((real *)&m)[2*i+j];
}

static inline v2
m22rowx(m22 m)
{
	return v2new(m.xx, m.xy);
}

static inline v2
m22rowy(m22 m)
{
	return v2new(m.yx, m.yy);
}

static inline v2
m22colx(m22 m)
{
	return v2new(m.xx, m.yx);
}

static inline v2
m22coly(m22 m)
{
	return v2new(m.xy, m.yy);
}

static inline m22
m22ident(void)
{
	return m22new(1, 0, 0, 1);
}

static inline m22
m22rot(real angle)
{
	real c = (real)cos((double)angle);
	real s = (real)sin((double)angle);
	return m22new(c, -s, s, c);
}

static inline m22
m22add(m22 a, m22 b)
{
	return m22new(a.xx + b.xx, a.xy + b.xy, a.yx + b.yx, a.yy + b.yy);
}

static inline m22
m22sub(m22 a, m22 b)
{
	return m22new(a.xx - b.xx, a.xy - b.xy, a.yx - b.yx, a.yy - b.yy);
}

static inline m22
m22neg(m22 m)
{
	return m22new(-m.xx, -m.xy, -m.yx, -m.yy);
}

static inline m22
m22mul(m22 m, real s)
{
	return m22new(m.xx * s, m.xy * s, m.yx * s, m.yy * s);
}

static inline m22
m22div(m22 m, real s)
{
	return m22mul(m, (real)1.0 / s);
}

static inline m22
m22trans(m22 m)
{
	return m22new(m.xx, m.yx, m.xy, m.yy);
}

static inline v2
m22v2(m22 m, v2 v)
{
	return v2new(m.xx * v.x + m.xy * v.y,
		     m.yx * v.x + m.yy * v.y);
}

static inline m22
m22m22(m22 a, m22 b)
{
	return m22new(a.xx * b.xx + a.xy * b.yx,
		      a.xx * b.xy + a.xy * b.yy,
		      a.yx * b.xx + a.yy * b.yx,
		      a.yx * b.xy + a.yy * b.yy);
}

static inline real
m22trace(m22 m)
{
	return (m.xx + m.yy);
}

static inline real
m22det(m22 m)
{
	return (m.xx * m.yy - m.xy * m.yx);
}

static inline m22
m22inv(m22 m)
{
	m22 i = m22new(m.yy, -m.xy, -m.yx, m.xx);
	return m22div(i, m22det(m));
}

static inline v2
m22solve(m22 a, v2 b)
{
	return v2new((a.xy*b.y - a.yy*b.x) / (a.xy*a.yx - a.xx*a.yy),
		     (a.yx*b.x - a.xx*b.y) / (a.xy*a.yx - a.xx*a.yy));
}

static inline int
m22eq(m22 a, m22 b, real eps)
{
	if (!realeq(a.xx, b.xx, eps)) return (0);
	if (!realeq(a.xy, b.xy, eps)) return (0);
	if (!realeq(a.yx, b.yx, eps)) return (0);
	if (!realeq(a.yy, b.yy, eps)) return (0);
	return (1);
}

static inline m33
m33new(real xx, real xy, real xz,
       real yx, real yy, real yz,
       real zx, real zy, real zz)
{
	m33 m = { xx, xy, xz, yx, yy, yz, zx, zy, zz };
	return (m);
}

static inline m33
m33zero(void)
{
	return m33new(0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static inline real
m33idx(m33 m, unsigned i, unsigned j)
{
	return ((real *)&m)[3*i+j];
}

static inline v3
m33rowx(m33 m)
{
	return v3new(m.xx, m.xy, m.xz);
}

static inline v3
m33rowy(m33 m)
{
	return v3new(m.yx, m.yy, m.yz);
}

static inline v3
m33rowz(m33 m)
{
	return v3new(m.zx, m.zy, m.zz);
}

static inline v3
m33colx(m33 m)
{
	return v3new(m.xx, m.yx, m.zx);
}

static inline v3
m33coly(m33 m)
{
	return v3new(m.xy, m.yy, m.zy);
}

static inline v3
m33colz(m33 m)
{
	return v3new(m.xz, m.yz, m.zz);
}

static inline m33
m33ident(void)
{
	return m33new(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

static inline m33
m33rotx(real angle)
{
	real c = (real)cos((double)angle);
	real s = (real)sin((double)angle);
	return m33new(1, 0, 0, 0, c, -s, 0, s, c);
}

static inline m33
m33roty(real angle)
{
	real c = (real)cos((double)angle);
	real s = (real)sin((double)angle);
	return m33new(c, 0, s, 0, 1, 0, -s, 0, c);
}

static inline m33
m33rotz(real angle)
{
	real c = (real)cos((double)angle);
	real s = (real)sin((double)angle);
	return m33new(c, -s, 0, s, c, 0, 0, 0, 1);
}

static inline m33
m33add(m33 a, m33 b)
{
	return m33new(a.xx + b.xx, a.xy + b.xy, a.xz + b.xz,
		      a.yx + b.yx, a.yy + b.yy, a.yz + b.yz,
		      a.zx + b.zx, a.zy + b.zy, a.zz + b.zz);
}

static inline m33
m33sub(m33 a, m33 b)
{
	return m33new(a.xx - b.xx, a.xy - b.xy, a.xz - b.xz,
		      a.yx - b.yx, a.yy - b.yy, a.yz - b.yz,
		      a.zx - b.zx, a.zy - b.zy, a.zz - b.zz);
}

static inline m33
m33neg(m33 m)
{
	return m33new(-m.xx, -m.xy, -m.xz,
		      -m.yx, -m.yy, -m.yz,
		      -m.zx, -m.zy, -m.zz);
}

static inline m33
m33mul(m33 m, real s)
{
	return m33new(m.xx * s, m.xy * s, m.xz * s,
		      m.yx * s, m.yy * s, m.yz * s,
		      m.zx * s, m.zy * s, m.zz * s);
}

static inline m33
m33div(m33 m, real s)
{
	return m33mul(m, (real)1.0 / s);
}

static inline m33
m33trans(m33 m)
{
	return m33new(m.xx, m.yx, m.zx, m.xy, m.yy, m.zy, m.xz, m.yz, m.zz);
}

static inline v3
m33v3(m33 m, v3 v)
{
	return v3new(m.xx * v.x + m.xy * v.y + m.xz * v.z,
		     m.yx * v.x + m.yy * v.y + m.yz * v.z,
		     m.zx * v.x + m.zy * v.y + m.zz * v.z);
}

static inline m33
m33m33(m33 a, m33 b)
{
	return m33new(a.xx * b.xx + a.xy * b.yx + a.xz * b.zx,
		      a.xx * b.xy + a.xy * b.yy + a.xz * b.zy,
		      a.xx * b.xz + a.xy * b.yz + a.xz * b.zz,
		      a.yx * b.xx + a.yy * b.yx + a.yz * b.zx,
		      a.yx * b.xy + a.yy * b.yy + a.yz * b.zy,
		      a.yx * b.xz + a.yy * b.yz + a.yz * b.zz,
		      a.zx * b.xx + a.zy * b.yx + a.zz * b.zx,
		      a.zx * b.xy + a.zy * b.yy + a.zz * b.zy,
		      a.zx * b.xz + a.zy * b.yz + a.zz * b.zz);
}

static inline real
m33trace(m33 m)
{
	return (m.xx + m.yy + m.zz);
}

static inline real
m33det(m33 m)
{
	return (m.xx * m.yy * m.zz + m.xy * m.yz * m.zx +
		m.yx * m.zy * m.xz - m.xz * m.yy * m.zx -
		m.xx * m.yz * m.zy - m.xy * m.yx * m.zz);
}

static inline m33
m33inv(m33 m)
{
	m33 i = m33new(m.yy * m.zz - m.yz * m.zy,
		       m.zy * m.xz - m.zz * m.xy,
		       m.xy * m.yz - m.xz * m.yy,
		       m.yz * m.zx - m.yx * m.zz,
		       m.zz * m.xx - m.zx * m.xz,
		       m.xz * m.yx - m.xx * m.yz,
		       m.yx * m.zy - m.yy * m.zx,
		       m.zx * m.xy - m.zy * m.xx,
		       m.xx * m.yy - m.xy * m.yx);
	return m33div(i, m33det(m));
}

static inline v3
m33solve(m33 a, v3 b)
{
	real d = m33det(a);
	real dx = m33det(m33new(b.x,a.xy,a.xz,b.y,a.yy,a.yz,b.z,a.zy,a.zz));
	real dy = m33det(m33new(a.xx,b.x,a.xz,a.yx,b.y,a.yz,a.zx,b.z,a.zz));
	real dz = m33det(m33new(a.xx,a.xy,b.x,a.yx,a.yy,b.y,a.zx,a.zy,b.z));
	return v3new(dx/d, dy/d, dz/d);
}

static inline int
m33eq(m33 a, m33 b, real eps)
{
	if (!realeq(a.xx, b.xx, eps)) return (0);
	if (!realeq(a.xy, b.xy, eps)) return (0);
	if (!realeq(a.xz, b.xz, eps)) return (0);
	if (!realeq(a.yx, b.yx, eps)) return (0);
	if (!realeq(a.yy, b.yy, eps)) return (0);
	if (!realeq(a.yz, b.yz, eps)) return (0);
	if (!realeq(a.zx, b.zx, eps)) return (0);
	if (!realeq(a.zy, b.zy, eps)) return (0);
	if (!realeq(a.zz, b.zz, eps)) return (0);
	return (1);
}

static inline q4
q4new(real w, real x, real y, real z)
{
	q4 q = { w, x, y, z };
	return (q);
}

static inline q4
q4zero(void)
{
	return q4new(0, 0, 0, 0);
}

static inline real
q4idx(q4 q, unsigned i)
{
	return ((real *)&q)[i];
}

static inline q4
q4add(q4 a, q4 b)
{
	return q4new(a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline q4
q4sub(q4 a, q4 b)
{
	return q4new(a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline q4
q4neg(q4 q)
{
	return q4new(-q.w, -q.x, -q.y, -q.z);
}

static inline q4
q4mul(q4 q, real s)
{
	return q4new(q.w * s, q.x * s, q.y * s, q.z * s);
}

static inline q4
q4div(q4 q, real s)
{
	return q4new(q.w / s, q.x / s, q.y / s, q.z / s);
}

static inline q4
q4conj(q4 q)
{
	return q4new(q.w, -q.x, -q.y, -q.z);
}

static inline q4
q4q4(q4 a, q4 b)
{
	return q4new(a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
		     a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
		     a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
		     a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w);
}

static inline real
q4normsq(q4 q)
{
	return (q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
}

static inline real
q4norm(q4 q)
{
	return ((real)sqrt((double)q4normsq(q)));
}

static inline int
q4eq(q4 a, q4 b, real eps)
{
	if (!realeq(a.w, b.w, eps)) return (0);
	if (!realeq(a.x, b.x, eps)) return (0);
	if (!realeq(a.y, b.y, eps)) return (0);
	if (!realeq(a.z, b.z, eps)) return (0);
	return (1);
}

#ifdef __cplusplus
} /* namespace linalg */
#endif

#endif /* LINALG_H_INCLUDED */
