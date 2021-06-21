#pragma once

#define MATH_PI 3.14159265358979323846
#define DEG2RAD(a) (a) * (MATH_PI / 180.0f)
#define RAD2DEG(a) (a) * (180.0f / MATH_PI)

#include <pmmintrin.h>

/*__forceinline float float_atan(float x, float y)
{
	double x2 = (double)(x);
	double y2 = (double)(y);
	double result = 0.0;

	__asm
	{
		fld QWORD PTR[x2]
		fld QWORD PTR[y2]
		fpatan

		fstp QWORD PTR[result]
		fwait
	}

	return (float)(result);
}*/

__forceinline int clamp_int(int x, int y, int z)
{
	if (x < y)
		x = y;

	if (x > z)
		x = z;

	return x;
}

__forceinline float clamp_float(float x, float y, float z)
{
	if (x < y)
		x = y;

	if (x > z)
		x = z;

	return x;
}

__forceinline float ceil(float x)
{
	return (float)((int)(x + 0.9999999f));
}

static float sse_sqrtfast(float x)
{
	float result = 0.0f;
	__m128 xmm = _mm_load_ss(&x);
	
	_mm_store_ss(&result, _mm_mul_ss(xmm, _mm_rsqrt_ss(xmm)));

	return result;
}

class Vector
{
public:
	Vector()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	Vector(float a,float b,float c)
	{
		x = a;
		y = b;
		z = c;
	}

	inline void operator=(Vector a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}
	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}
	inline bool operator!()
	{
		return !x && !y && !z;
	}
	inline bool operator==(Vector a)
	{
		return x == a.x && y == a.y && z == a.z;
	}
	inline bool operator!=(Vector a)
	{
		return x != a.x || y != a.y || z != a.z;
	}
	inline bool operator!=(float a)
	{
		return x != a && y != a && z != a;
	}
	inline Vector operator+(Vector a)
	{
		return Vector(x + a.x, y + a.y, z + a.z);
	}
	inline Vector operator-(Vector a)
	{
		return Vector(x - a.x, y - a.y, z - a.z);
	}
	inline Vector operator*(Vector a)
	{
		return Vector(x * a.x, y * a.y, z * a.z);
	}
	inline Vector operator/(Vector a)
	{
		return Vector(x / a.x, y / a.y, z / a.z);
	}
	inline void operator+=(Vector a)
	{
		*this = Vector(x + a.x, y + a.y, z + a.z);
	}
	inline void operator-=(Vector a)
	{
		*this = Vector(x - a.x, y - a.y, z - a.z);
	}
	inline void operator*=(Vector a)
	{
		*this = Vector(x * a.x, y * a.y, z * a.z);
	}
	inline void operator/=(Vector a)
	{
		*this = Vector(x / a.x, y / a.y, z / a.z);
	}
	inline Vector operator*(float a)
	{
		return Vector(x * a, y * a, z * a);
	}
	inline Vector operator/(float a)
	{
		return Vector(x / a, y / a, z / a);
	}
	inline void operator*=(float a)
	{
		*this = Vector(x * a, y * a, z * a);
	}
	inline void operator/=(float a)
	{
		*this = Vector(x / a, y / a, z / a);
	}
	inline float LengthSqr()
	{
		return x * x + y * y + z * z;
	}
	inline float Length()
	{
		return float_sqrt(LengthSqr());
	}
	inline Vector Normal()
	{
		Vector out = *this;
		
		float len = Length();

		if (len == 0.0f)
			return Vector();

		return out / len;
	}
	inline float Dot(Vector a)
	{
		return x * a.x + y * a.y + z * a.z;
	}
	inline Vector Cross(Vector a)
	{
		return Vector(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
	}
	inline float Pitch()
	{
		float rot = float_atan2(-z, float_sqrt(x * x + y * y));
		return RAD2DEG(rot);
	}
	inline float Yaw()
	{
		float rot = float_atan2(y, x);
		return RAD2DEG(rot);
	}
	inline Vector Angles()
	{
		return Vector(Pitch(), Yaw(), 0.0f);
	}
	Vector Forward()
	{
		float pitch = DEG2RAD(x);
		float yaw = DEG2RAD(y);

		float sp = 0.0f;
		float cp = 0.0f;
		float_sincos(pitch, &sp, &cp);

		float sy = 0.0f;
		float cy = 0.0f;
		float_sincos(yaw, &sy, &cy);

		return Vector(cp * cy, cp * sy, -sp);
	}
	Vector Right()
	{
		float pitch = DEG2RAD(x);
		float yaw = DEG2RAD(y);
		float roll = DEG2RAD(z);

		float sp = 0.0f;
		float cp = 0.0f;
		float_sincos(pitch, &sp, &cp);

		float sy = 0.0f;
		float cy = 0.0f;
		float_sincos(yaw, &sy, &cy);

		float sr = 0.0f;
		float cr = 0.0f;
		float_sincos(roll, &sr, &cr);

		return Vector(-1.0f * sr * sp * cy + -1.0f * cr * -sy, -1.0f * sr * sp * sy + -1.0f * cr * cy, -1.0f * sr * cp);
	}
	Vector Up()
	{
		float pitch = DEG2RAD(x);
		float yaw = DEG2RAD(y);
		float roll = DEG2RAD(z);

		float sp = 0.0f;
		float cp = 0.0f;
		float_sincos(pitch, &sp, &cp);

		float sy = 0.0f;
		float cy = 0.0f;
		float_sincos(yaw, &sy, &cy);

		float sr = 0.0f;
		float cr = 0.0f;
		float_sincos(roll, &sr, &cr);

		return Vector(cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp);
	}


	float x,y,z;
};

class Matrix
{
public:
	Matrix()
	{
		
	}
	Matrix(Matrix& m)
	{
		for (int i = 0; i < 4; ++i)
		{
			self[i][0] = m[i][0];
			self[i][1] = m[i][1];
			self[i][2] = m[i][2];
		}
	}
	inline float* operator[](int i)
	{
		return self[i];
	}
	inline Vector GetColumn(int i)
	{
		return Vector(self[0][i], self[1][i], self[2][i]);
	}
	inline void SetColumn(Vector in,int i)
	{
		self[0][i] = in[0];
		self[1][i] = in[1];
		self[2][i] = in[2];
	}
	inline Vector GetTransform()
	{
		return GetColumn(3);
	}
	void Init(Vector pos, Vector angles)
	{
		SetColumn(angles.Forward(), 0);
		SetColumn(angles.Right(), 1);
		SetColumn(angles.Up(), 2);
		SetColumn(pos, 3);
	}
	inline Vector Rotate(Vector& in)
	{
		Vector v1(self[0][0], self[0][1], self[0][2]);
		Vector v2(self[1][0], self[1][1], self[1][2]);
		Vector v3(self[2][0], self[2][1], self[2][2]);

		return Vector(in.Dot(v1), in.Dot(v2), in.Dot(v3));
	}
	inline Vector Transform(Vector in)
	{
		Vector rot = Rotate(in);

		return GetTransform() + rot;
	}

	float self[3][4];
};

static bool within_box(Vector point, Vector mins, Vector maxs)
{
	if (point.x < mins.x || point.x > maxs.x)
		return false;

	if (point.y < mins.y || point.y > maxs.y)
		return false;

	if (point.z < mins.z || point.z > maxs.z)
		return false;
	
	return true;
}

static bool within_cylinder(Vector start, Vector end, float length, float radius, Vector test)
{
	float length_sq = length * length;
	float radius_sq = radius * radius;

	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float dz = end.z - start.z;

	float pdx = test.x - start.x;
	float pdy = test.y - start.y;
	float pdz = test.z - start.z;

	float dot = (pdx * dx) + (pdy * dy) + (pdz * dz);

	if (dot < 0.0f || dot > length_sq)
		return false;

	float dsq = ((pdx * pdx) + (pdy * pdy) + (pdz * pdz)) - ((dot * dot) / length_sq);

	if (dsq > radius_sq)
		return false;

	return true;
}

static bool intersect_sphere(Vector org, Vector center, Vector dir, float radius_sqr, float* tmin)
{
	Vector delta = org - center;

	float b = 2.0f * delta.Dot(dir);
	float c = delta.Dot(delta) - radius_sqr;

	float d = (b * b) - (4.0f * c);

	if (d < 0.0f)
		return false;

	float d2 = float_sqrt(d);

	if (tmin)
		*tmin = (-b - d2) * 0.5;

	return true;
}

static bool intersect_capsule(Vector org, Vector dir, Vector min, Vector max, float radius, Vector* point)
{
	const float radius_sqr = radius * radius;

	Vector va = max - min;
	Vector vb = org - min;
	Vector vc = (min + (va)) - org;

	float va_dot_d = va.Dot(dir);
	float va_dot_vb = va.Dot(vb);
	float va_dot_va = va.Dot(va);
	float vc_dot_d = vc.Dot(dir);

	if (vc_dot_d < 0.0f)
		return false;

	float m = va_dot_d / va_dot_va;
	float n = va_dot_vb / va_dot_va;

	Vector q = dir - (va * m);
	Vector r = vb - (va * n);

	float a = q.Dot(q);

	if (a == 0.0f)
		return false;

	float b = 2.0f * q.Dot(r);
	float c = r.Dot(r) - radius_sqr;

	float d = (b * b) - (4.0f * a * c);

	if (d < 0.0f)
		return false;

	float div = 2.0f * a;

	float d2 = float_sqrt(d);

	float tmin = (-b - d2) / div;
	float tmax = (-b + d2) / div;
	if (tmin > tmax)
	{
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}

	float t_k1 = tmin * m + n;

	if (t_k1 < 0.0f)
	{
		float len = 0.0f;

		if (!intersect_sphere(org, min, dir, radius_sqr, &len))
			return false;

		if (point)
			*point = org + (dir * len);
	}
	else if (t_k1 > 1.0f)
	{
		float len = 0.0f;

		if (!intersect_sphere(org, max, dir, radius_sqr, &len))
			return false;

		if (point)
			*point = org + (dir * len);
	}
	else
	{
		if (point)
			*point = org + (dir * tmin);
	}

	return true;
}

static bool intersect_box(Vector origin, Vector dir, Vector min, Vector max, float* length)
{
	Vector dirfrac;
	dirfrac.x = 1 / dir.x;
	dirfrac.y = 1 / dir.y;
	dirfrac.z = 1 / dir.z;
	
	float t1 = (min.x - origin.x) * dirfrac.x;
	float t2 = (max.x - origin.x) * dirfrac.x;
	float t3 = (min.y - origin.y) * dirfrac.y;
	float t4 = (max.y - origin.y) * dirfrac.y;
	float t5 = (min.z - origin.z) * dirfrac.z;
	float t6 = (max.z - origin.z) * dirfrac.z;
	
	float tmin = max(max(min(t1,t2), min(t3,t4)), min(t5,t6));
	float tmax = min(min(max(t1,t2), max(t3,t4)), max(t5,t6));
	
	float t = 0.0f;
	
	if (tmin < 0.0f || tmax < 0.0f || tmin > tmax)
		return false;
	
	t = tmin;

	if (length)
		*length = t;
	
	return true;
}
