#ifndef __SPLINE3D__
#define __SPLINE3D__

#include <fstream>
#include <string>
#include <vector>
#include "vector.h"
#include "strtools.h"

class Spline3D
{
	int m_nsegm;			//  число сегментов
	double* m_knots;		//  значения параметра t
	vector3D* m_controls;	//  контрольные (опорные) точки
	vector3D* m_ders;		//  касательные векторы

public:

	Spline3D();
	Spline3D(const Spline3D& crv);
	virtual ~Spline3D();
	void Init(int nsegm, double* knots, vector3D* controls, vector3D* ders);
	int Import(const char* filename);
	void Export(const char* filename);
	vector3D Point1(int segm, double t);
	void outputpoints(std::ostream& out, int segm, double step);
	void outputpoints(const char *output, double step);
	int get_nseg() const
	{
		return m_nsegm;
	}
	
	void get_knots(std::vector<double>& sp_knots) const
	{
		for (int i=0;i<=m_nsegm;i++)
			sp_knots.push_back(m_knots[i]);
	}
	
	void get_controls(vector3D*& controls) const
	{
		controls = new vector3D[m_nsegm+1];
		for (int i=0;i<=m_nsegm;i++)
			controls[i] = m_controls[i];
	}

	void get_ders(vector3D*& ders) const
	{
		ders = new vector3D[m_nsegm+1];
		for (int i=0;i<=m_nsegm;i++)
			ders[i] = m_ders[i];
	}

};

#endif