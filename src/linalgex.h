//
// math extension helpers for linalg.h
//

#ifndef LINALGEX_H_INCLUDED
#define LINALGEX_H_INCLUDED

#include "linalg.h"

#ifdef __cplusplus
namespace linalg {
#endif

static inline v4
v4rgba(real r, real g, real b, real a)
{
	return v4new(r, g, b, a);
}

static inline v4
v4rgb1(real r, real g, real b)
{
	return v4new(r, g, b, 1.0f);
}

static inline bool
rectcontains(v2 p, v2 bl, v2 tr)
{
	if (p.x > tr.x) return false;
	if (p.y > tr.y) return false;
	if (p.x < bl.x) return false;
	if (p.y < bl.y) return false;
	return true;
}

static inline bool
rectoverlap(v2 abl, v2 atr, v2 bbl, v2 btr)
{
	if (abl.x > btr.x) return false;
	if (abl.y > btr.y) return false;
	if (atr.x < bbl.x) return false;
	if (atr.y < bbl.y) return false;
	return true;
}

#ifdef __cplusplus
} /* namespace linalg */
#endif

#endif /* LINALGEX_H_INCLUDED */
