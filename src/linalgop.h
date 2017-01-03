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

#ifndef LINALGOP_H_INCLUDED
#define LINALGOP_H_INCLUDED

#ifdef __cplusplus
namespace linalg {
#endif

#ifdef __cplusplus
	static inline v2 operator+ (v2 lhs, v2 rhs) { return v2add(lhs, rhs); }
	static inline v2 operator- (v2 lhs, v2 rhs) { return v2sub(lhs, rhs); }
	static inline v2 operator* (v2 lhs, float rhs) { return v2mul(lhs, rhs); }
	static inline v2 operator/ (v2 lhs, float rhs) { return v2div(lhs, rhs); }

	static inline v2& operator+= (v2& lhs, v2 rhs) { lhs = v2add(lhs, rhs); return lhs; }
	static inline v2& operator-= (v2& lhs, v2 rhs) { lhs = v2sub(lhs, rhs); return lhs; }
	static inline v2& operator*= (v2& lhs, float rhs) { lhs = v2mul(lhs, rhs); return lhs; }
	static inline v2& operator/= (v2& lhs, float rhs) { lhs = v2div(lhs, rhs); return lhs; }

	static inline v3 operator+ (v3 lhs, v3 rhs) { return v3add(lhs, rhs); }
	static inline v3 operator- (v3 lhs, v3 rhs) { return v3sub(lhs, rhs); }
	static inline v3 operator* (v3 lhs, float rhs) { return v3mul(lhs, rhs); }
	static inline v3 operator/ (v3 lhs, float rhs) { return v3div(lhs, rhs); }

	static inline v3& operator+= (v3& lhs, v3 rhs) { lhs = v3add(lhs, rhs); return lhs; }
	static inline v3& operator-= (v3& lhs, v3 rhs) { lhs = v3sub(lhs, rhs); return lhs; }
	static inline v3& operator*= (v3& lhs, float rhs) { lhs = v3mul(lhs, rhs); return lhs; }
	static inline v3& operator/= (v3& lhs, float rhs) { lhs = v3div(lhs, rhs); return lhs; }

	static inline v4 operator+ (v4 lhs, v4 rhs) { return v4add(lhs, rhs); }
	static inline v4 operator- (v4 lhs, v4 rhs) { return v4sub(lhs, rhs); }
	static inline v4 operator* (v4 lhs, float rhs) { return v4mul(lhs, rhs); }
	static inline v4 operator/ (v4 lhs, float rhs) { return v4div(lhs, rhs); }
#endif

#ifdef __cplusplus
} /* namespace linalg */
#endif

#endif /* LINALGOP_H_INCLUDED */
