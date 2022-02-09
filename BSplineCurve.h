#ifndef __BSPLINECURVE__
#define __BSPLINECURVE__


#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "vector.h"
#include "strtools.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class BSplineCurve  
{
// ----- Данные
public:
	int m_n;	        //  Верхний индекс суммы (число опорных точек). Это P0,...,Pn
	int m_deg;		    //  Степень базовой функции. 3-для функции 3-ей степени
	int m_isClosed;		//  0 - замкнута, 1 - незамкнута
	int m_isPolynom;	//  0 - rational, 1 - polynomial
	int m_isPeriodic;	//  0 - непериодическая, 1 - периодическая
	int m_nsegmExt;		//  число сегментов, часть которых может быть вырожденными
	int m_nsegm;		//  число невырожденных сегментов 
						//  кратные узлы, дающие вырожденные сегменты, исключаются
						//  в случае отсутствия кратности узлов m_nsegmExt = m_nsegm

	int m_sum;			//  суммарная кратность (полностью определяется по m_n и m_deg)
	double m_pmin;		//  Минимальное значение параметра t (в зависимости от массива параметров)
	double m_pmax;		//  Максимальное значение параметра t (в зависимости от массива параметров)
						//  в простейшем случае
						//  m_nsegmExt = m_controlCount - (m_degree+1) + 2;
						//  например 3-(3+1)+2 для кубического случая с одним сегментом,
						//  который задаётся 4 точками

						//  в более сложном случае значения параметра на уникальных
						//  интервалах задаётся пользователем
	double* m_ps;		//  Вектор параметров в STEPe (кратные Knot указываются только один раз) 
	double* m_knots;	//  Узлы (значения параметра). 
						//  их m_nsegmExt-1+ 2*(m_degree+1)=
						//  = m_controlCount+(m_degree+1)+1
						//  3+(3+1)+1 в кубическом случае для одного сегмента
						//  последовательность узлов 0,0,0,0,1,1,1,1
	int* m_knotMult;	//  кратности, используемые в частности в STEP
	double* m_weights;	//	Весовые коэффициенты для рационального

	vector3D* m_controls;	//  Контрольные (опорные) точки многоугольника

public:
	void outputpoints(const char *output, double step);
	void outputvertexes(const char *output, int addflag);
	vector3D der1(double t);
	void der12(double t, vector3D& d1, vector3D& d2);
	vector3D Point1(double t);
	/*--------------------------------
	barray - выходной параметр
	функция возвращает iStart: (t>=m_knots[iStart]) && (t<m_knots[iStart+1])
	barray - массив размера m_n+1
	---------------------------------*/
	int basisf(double t, int k, double*& barray);
	int basisf(double t, int k, double*& barray, double*& b1array);
	int basisf(double t, int k, double*& barray, double*& b1array, double*& b2array);
	void basiscurves(char *prefix, double step);
	BSplineCurve();
	BSplineCurve(const BSplineCurve& crv);
	int Import(const char* filename);
	void Export(const char* filename);
	virtual ~BSplineCurve();
	void Init(int isClosed, int isPolynom, int isPeriodic, int n, int P, double* knots, vector3D* controls, double* weights);
	void InitType(int isClosed, int isPolynom, int isPeriodic);
	void InitG(int n, int P, double* knots);
	void InitControls(vector3D* controls);
	void InitWeights(double* weights);

	int get_nseg() const
	{
		return m_nsegm;
	}

	int get_n() const
	{
		return m_n;
	}

	void get_knots(std::vector<double>& knots) const
	{
		for (int i=0;i<=m_nsegmExt+2*m_deg;i++)
			knots.push_back(m_knots[i]);
	}
	
	void get_controls(vector3D*& controls) const
	{
		controls = new vector3D[m_n+1];
		for (int i=0;i<=m_n;i++)
			controls[i] = m_controls[i];
	}

};

#endif
