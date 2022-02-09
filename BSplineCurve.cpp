// BSplineCurve.cpp: implementation of the BSplineCurve class.
//
//////////////////////////////////////////////////////////////////////

#include "BSplineCurve.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BSplineCurve::BSplineCurve()
{
	m_knots = NULL;
	m_controls	= NULL;
	m_weights	= NULL;
	m_knotMult = NULL;
	m_ps = NULL;
	m_n = 0;
}

//копирующий конструктор
BSplineCurve::BSplineCurve(const BSplineCurve& crv)
{
	this->m_controls = NULL;
	this->m_knots = NULL;
	this->m_weights = NULL;
	this->Init(crv.m_isClosed,crv.m_isPolynom,crv.m_isPeriodic,crv.m_n,crv.m_deg,crv.m_knots,crv.m_controls,crv.m_weights);
}

//0 - успешный импорт
//> 0 - ошибка импорта
int BSplineCurve::Import(const char* filename)
{
	std::ifstream input(filename);
	int isClosed, isPeriodic, isRational;
	int n, k;
	double* knots = 0;
	vector3D* controls = 0;
	double* weights = 0;
	
	std::string line;

	do { std::getline(input,line); } while (empty(line));
	int m_closed_imp_result = getProperty(line, "is_closed", isClosed);
	if (m_closed_imp_result == -1) return 1;
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	int m_periodic_imp_result = getProperty(line, "is_periodic", isPeriodic);
	if (m_periodic_imp_result == -1) return 2;
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	int m_n_imp_result = getProperty(line, "n", n);
	if (m_n_imp_result == -1) return 3;
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	int k_imp_result = getProperty(line, "k", k);
	if (k_imp_result == -1) return 4;
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	if (line.find("knots") != std::string::npos)
	{
		int knots_number;
		input >> knots_number;
		knots = new double[knots_number];
		for(int i=0;i<knots_number;i++)
			input >> knots[i];
	}
	else return 5;
	line.clear();
	
	do { std::getline(input,line); } while (empty(line));
	if (line.find("controls") != std::string::npos)
	{
		controls = new vector3D[n+1];
		for(int i=0; i<=n; i++)
			input >> controls[i];
	}
	else {
		if (knots) delete[] knots;
		return 6;
	}
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	int weights_header_imp_result = getProperty(line, "weights", isRational);
	if (weights_header_imp_result != -1) {
		if (isRational) {
			weights = new double[n+1];
			for(int i=0; i<=n; i++)
				input >> weights[i];
		}
	}
	else {
		if (knots) delete[] knots;
		if (controls) delete[] controls;
		return 7;
	}
	
	int isPolynom;
	if (isRational) isPolynom = 0;
	else isPolynom = 1;

	Init(isClosed, isPolynom, isPeriodic, n, k-1, knots, controls, weights);
	
	if (knots) delete[] knots;
	if (controls) delete[] controls;
	
	if (weights) delete[] weights;

	input.close();
	return 0;
}

void BSplineCurve::Export(const char* filename)
{
	std::ofstream output(filename);
	output.precision(21);
	output << "is_closed\t" << m_isClosed << '\n';
	output << "is_periodic\t" << m_isPeriodic << '\n';
	output << "n\t" << m_n << '\n';
	output << "k\t" << m_deg + 1 << '\n';
	output << '\n';
	output << "knots\n";
	int number_of_knots = m_sum;
	output << number_of_knots << '\t';
	for(int i=0;i<number_of_knots-1;i++) output << m_knots[i] << '\t';
	output <<m_knots[number_of_knots-1] << '\n';

	output << "\n\n";
	output << "controls\n";
	for(int i=0;i<=m_n;i++)
		output << m_controls[i] << '\n';
	output << '\n';

	if (!m_isPolynom) {
		output << "weights 1\n";
		for(int i=0;i<=m_n;i++)
			output << m_weights[i] << '\n';
	}
	else output << "weights 0";

	output.close();
}

BSplineCurve::~BSplineCurve()
{
	if ( m_knots ) delete [] m_knots;
	if ( m_controls )	delete [] m_controls;
	if ( m_weights )	delete [] m_weights;
	if ( m_knotMult )   delete [] m_knotMult;
	if ( m_ps )   delete [] m_ps;

}

void BSplineCurve::InitType(int isClosed, int isPolynom, int isPeriodic)
{
 	m_isPolynom = isPolynom;
 	m_isPeriodic = isPeriodic;
	m_isClosed = isClosed;
}

void BSplineCurve::Init(int isClosed, int isPolynom, int isPeriodic, int n, int P, double* knots, vector3D* controls, double* weights)
{
 	InitType(isClosed, isPolynom, isPeriodic);
	InitG(n,P,knots);
	InitControls(controls);
	InitWeights(weights);
}

void BSplineCurve::InitG(int n, int P, double* knots)
{
	//независимы от кратностей узлов: 
	//число контрольных точек;
	//размерность массива узлов;
	//суммарная кратность.

	m_n = n;
	m_deg = P;
	//часть сегментов может быть вырожденными
	m_nsegmExt = m_n - m_deg + 1;

	//заполнение массива узлов; подсчёт числа сегментов

	assert( knots );
	if (m_knots) delete[] m_knots;
	m_knots = new double[m_n+m_deg+2];
	int i;
	m_nsegm = 0; //число сегментов - сколько раз изменится параметр
	double temp = knots[0];
	for (i = 0; i < m_n+m_deg+2; i++) {
		m_knots[i] = knots[i];
		if (temp != knots[i]) {
			temp = knots[i];
			m_nsegm++;
		}
	}
	
	if (m_isPeriodic == false) { 
		m_pmin = m_knots[0];
		//размерность массива параметров = числу уникальных интервалов
		m_pmax = m_knots[m_n+m_deg+1];
	}
	else
	{
		m_pmin = m_knots[m_deg];
		m_pmax = m_knots[m_n+1];
	}

	//сумма кратностей и размерность массива узлов должны совпадать
	m_knotMult = new int[m_nsegm+1];
	m_ps = new double[m_nsegm+1];
	m_sum=0;
	temp = m_knots[0];
	int count = 0;
	int j=0;
	for (i=0; i<m_n+m_deg+2; i++) {
		if (temp == knots[i]) count++;
		else
		{
			temp = knots[i];
			m_knotMult[j] = count;
			m_ps[j] = knots[i-1];
			j++;
			m_sum = m_sum + count;
			count = 1;
		}
	}
	m_knotMult[j] = count;
	m_ps[j] = knots[i-1];
	m_sum = m_sum + count;
}

void BSplineCurve::InitControls(vector3D* controls)
{
	 
	// Вершины определяющего многоугольника (опорные точки)
	assert( controls );
	assert( m_n!=0 );
	//m_n-верхний индекс числа контрольных точек
	if (m_controls) delete[] m_controls;
	m_controls = new vector3D[m_n+1];
	for( int i=0; i<=m_n; i++ ) {
		m_controls[i] = controls[i];
	}

}

void BSplineCurve::InitWeights(double* weights)
{
	// Весовые коэффициенты относятся к случаю рационального B-сплайна
	if( m_isPolynom ) 
	{
		assert( weights==NULL );
		m_weights = weights;
	} else {
		m_weights = new double[m_n+1];
		for( int i=0; i<m_n+1; i++ ) {
			m_weights[i] = weights[i];
		}
	}

}

vector3D BSplineCurve::Point1(double t)
{
	vector3D point(0,0,0);
	
	double* basis = 0;

	int iStart=basisf(t,m_deg+1,basis);

	double denom;
	denom = 0.;
	if( m_isPolynom )  {	// Polinomial case
		for (int i=iStart-m_deg; i<=iStart; i++)
			point = point + m_controls[i]*basis[i];
	}
	else
	{
		for (int i=iStart-m_deg; i<=iStart; i++)
		{
			denom = denom + m_weights[i]*basis[i];
			point = point + m_controls[i]*(m_weights[i]*basis[i]);
		}
	}
	if( ! m_isPolynom )	point /= denom;
	
	delete[] basis;
	
	return point; 
}

/*--------------------------------
barray - выходной параметр
функция возвращает iStart: (t>=m_knots[iStart]) && (t<m_knots[iStart+1])
barray - массив размера m_n+2
---------------------------------*/
int BSplineCurve::basisf(double t, int k, double*& barray)
{
	double eps = 0.000001;
	if (m_pmin-t > eps) t=m_pmin;
	if ((t-m_pmax > eps) || (fabs(t-m_pmax) < eps)) t=m_pmax;
//  assert( (t>=m_pmin) && (t<=m_pmax));

	//далее по тексту max(iStart) = m_n+1, 
	//следовательно надо выделить на один 
	//элемент больше числа базисных функций
	//basis[m_n+1] всегда содержит 0 и 
	//введена для удобства вычисления
	assert(barray == 0);
	barray = new double[m_n+1+1];



	//определим интервал к которому относится значание параметра
	//данный цикл не учитывает t==m_knots[m_n+1]
	//данное значение обрабатывается отдельно

	int iStart;
	for (iStart=m_deg; iStart<=(m_sum-1)-m_deg-1; iStart++) 
		if ( (t>=m_knots[iStart]) && (t<m_knots[iStart+1]) ) break;

	double d,e;

	//обнуление массива
	for (int i=0; i <= m_n+1; i++) barray[i]=0;

	// здесь учитывается конечная точка
	//(m_sum-1)-m_deg-1 = k+m_nsegmExt-2
	if (iStart>(m_sum-1)-m_deg-1) iStart = (m_sum-1)-m_deg-1;
	// общий случай
	barray[iStart] = 1;


/* calculate the higher order barray functions */

	for (int ki = 2; ki <= k; ki++){
	    for (int i = iStart-ki+1; i <=iStart; i++){
        	if (barray[i] != 0)    /* if the lower order basis function is zero skip the calculation */
           		d = ((t-m_knots[i])*barray[i])/(m_knots[i+ki-1]-m_knots[i]);
	        else
				d = 0;

    	    if (barray[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
        		e = ((m_knots[i+ki]-t)*barray[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else
    			e = 0;

    	    barray[i] = d + e;
		}
	}
	return iStart;
}

int BSplineCurve::basisf(double t, int k, double*& barray, double*& b1array)
{
	double eps = 1e-9;
	if (m_pmin-t > eps) t=m_pmin;
	if ((t-m_pmax > eps) || (fabs(t-m_pmax) < eps)) t=m_pmax;
	
	assert((barray == 0) && (b1array == 0));

//  assert( (t>=m_pmin) && (t<=m_pmax));

	//определим интервал к которому относится значание параметра
	//данный цикл не учитывает t==m_knots[m_n+1]
	//данное значение обрабатывается отдельно

	int iStart;

	for (iStart=m_deg; iStart<=(m_sum-1)-m_deg-1; iStart++) 
		if ( (t>=m_knots[iStart]) && (t<m_knots[iStart+1]) ) break;

	double b1,b2;
	double f1,f2,f3,f4;


	//max(iStart) = m_n+1, 
	//следовательно надо выделить на один 
	//элемент больше числа базисных функций
	//basis[m_n+1] всегда содержит 0 и 
	//введена для удобства вычисления
	barray = new double[m_n+2];
	b1array = new double[m_n+2];

	//обнуление/инициализация массивов
	for (int i=0; i<=m_n+1; i++) barray[i]=0;
	for (int i=0; i<=m_n+1; i++) b1array[i]=0;
	
	if (k < 2) return iStart;

	// здесь учитывается конечная точка
	if (iStart>(m_sum-1)-m_deg-1) iStart = (m_sum-1)-m_deg-1;
	// общий случай
	barray[iStart] = 1;
	
/// calculate the higher order barray functions 

	for (int ki = 2; ki <= k; ki++){
	    for (int i = iStart-ki+1; i <=iStart; i++){
			if (barray[i] != 0)
           		b1 = ((t-m_knots[i])*barray[i])/(m_knots[i+ki-1]-m_knots[i]);
			else b1 = 0;

			if (barray[i+1] != 0)
        		b2 = ((m_knots[i+ki]-t)*barray[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else b2 = 0;
	        
			//       calculate first derivative
			if (barray[i] != 0)       // if the lower order basis function is zero skip the calculation
	            f1 = barray[i]/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            f1 = 0;

	       	if (barray[i+1] != 0)     // if the lower order basis function is zero skip the calculation
	            f2 = -barray[i+1]/(m_knots[i+ki]-m_knots[i+1]);
	        else
	            f2 = 0;

	        if (b1array[i] != 0)      // if the lower order basis function is zero skip the calculation
	            f3 = ((t-m_knots[i])*b1array[i])/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            f3 = 0;

	        if (b1array[i+1] != 0)    // if the lower order basis function is zero skip the calculation
	            f4 = ((m_knots[i+ki]-t)*b1array[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else
	            f4 = 0;
			
    	    barray[i] = b1 + b2;
			b1array[i] = f1 + f2 + f3 + f4;
		}
	}

	return iStart;
}

int BSplineCurve::basisf(double t, int k, double*& barray, double*& b1array, double*& b2array)
{

	assert( (t>=m_pmin) && (t<=m_pmax) );
	assert((barray == 0) && (b1array == 0) && (b2array == 0));

	//определим интервал к которому относится значание параметра
	//данный цикл не учитывает t==m_knots[m_n+1]
	//данное значение обрабатывается отдельно

	int iStart;
	for (iStart=m_deg; iStart<=(m_sum-1)-m_deg-1; iStart++) 
		if ( (t>=m_knots[iStart]) && (t<m_knots[iStart+1]) ) break;

	double b1,b2;
	double f1,f2,f3,f4;
	double s1,s2,s3,s4;


	//max(iStart) = m_n+1, 
	//следовательно надо выделить на один 
	//элемент больше числа базисных функций
	//basis[m_n+1] всегда содержит 0 и 
	//введена для удобства вычисления
	barray = new double[m_n+2];
	b1array = new double[m_n+2];
	b2array = new double[m_n+2];

	//обнуление/инициализация массивов
	for (int i=0; i<=m_n+1; i++) barray[i]=0;
	for (int i=0; i<=m_n+1; i++) b1array[i]=0;
	for (int i=0; i<=m_n+1; i++) b2array[i]=0;
	
	if (k < 2) return iStart;

	// здесь учитывается конечная точка
	if (iStart>(m_sum-1)-m_deg-1) iStart = (m_sum-1)-m_deg-1;
	// общий случай
	barray[iStart] = 1;
	
/// calculate the higher order barray functions 

	for (int ki = 2; ki <= k; ki++){
	    for (int i = iStart-ki+1; i <=iStart; i++){
			if (barray[i] != 0)
           		b1 = ((t-m_knots[i])*barray[i])/(m_knots[i+ki-1]-m_knots[i]);
			else b1 = 0;

			if (barray[i+1] != 0)
        		b2 = ((m_knots[i+ki]-t)*barray[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else b2 = 0;
	        
			//       calculate first derivative
			if (barray[i] != 0)       // if the lower order basis function is zero skip the calculation
	            f1 = barray[i]/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            f1 = 0;

	       	if (barray[i+1] != 0)     // if the lower order basis function is zero skip the calculation
	            f2 = -barray[i+1]/(m_knots[i+ki]-m_knots[i+1]);
	        else
	            f2 = 0;

	        if (b1array[i] != 0)      // if the lower order basis function is zero skip the calculation
	            f3 = ((t-m_knots[i])*b1array[i])/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            f3 = 0;

	        if (b1array[i+1] != 0)    // if the lower order basis function is zero skip the calculation
	            f4 = ((m_knots[i+ki]-t)*b1array[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else
	            f4 = 0;
			
			//       calculate second derivative

	        if (b1array[i] != 0)      // if the lower order basis function is zero skip the calculation
	            s1 = (2*b1array[i])/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            s1 = 0;

	        if (b1array[i+1] != 0)      // if the lower order basis function is zero skip the calculation
	            s2 = (-2*b1array[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else 
	            s2 = 0;

	        if (b2array[i] != 0)       // if the lower order basis function is zero skip the calculation
	            s3 = ((t-m_knots[i])*b2array[i])/(m_knots[i+ki-1]-m_knots[i]);
	        else
	            s3 = 0;

	        	if (b2array[i+1] != 0)    // if the lower order basis function is zero skip the calculation
	            s4 = ((m_knots[i+ki]-t)*b2array[i+1])/(m_knots[i+ki]-m_knots[i+1]);
	        else
	            s4 = 0;
 

    	    barray[i] = b1 + b2;
			b1array[i] = f1 + f2 + f3 + f4;
			b2array[i] = s1 + s2 + s3 + s4;
		}
	}

	return iStart;	
}

void BSplineCurve::basiscurves(char *prefix, double step)
{

	int i,j;
	int npoints = (int)floor((m_pmax-m_pmin)/step)+1;
	double** basis = new double* [npoints];
	for(i=0; i<npoints; i++) basis[i] = 0;
	
	int iStart;
	
	for(i=0; i<npoints; i++)
		iStart=basisf(m_pmin + i*step,m_deg+1,basis[i]);
	
	for(j=0; j<=m_n; j++) {
		std::ostringstream myfile_stream;
		myfile_stream << prefix << '_' << j << ".dat";
		std::ofstream myfile(myfile_stream.str().c_str());	
		for(i=0; i<npoints; i++) 
		{
			myfile << m_pmin + i*step << '\t' << basis[i][j] << '\n';		
		}
		myfile.close();
	}
	
	for(i=0; i<npoints; i++)
		delete[] basis[i];
	delete[] basis;

}

void BSplineCurve::outputpoints(const char *output, double step)
{
	std::ofstream myfile(output);

	
	//будем брать первую и вторую координаты кривой
	vector3D mypoint;


	//	mypoint=mycurve->Point1(4);


	// (npoints-1)*(step) = ((m_pmax-m_pmin)/step)*step = m_pmax - m_pmin
	int npoints = (int)floor((m_pmax-m_pmin)/step)+1;

	int i;
	for (i = 0; i < npoints; i++) {
		mypoint=Point1(m_pmin + i*step);
		myfile << mypoint[0] << '\t' << mypoint[1] << '\t' << mypoint[2] << '\t' << m_pmin + i*step << '\n';
	}
	
	myfile.close();
}

void BSplineCurve::outputvertexes(const char *output, int addflag)
{
	std::ofstream* myfile;
	if (addflag == 1)  myfile = new std::ofstream(output,std::ios::app);
	else myfile = new std::ofstream(output);

	//вывод многоугольника
	//tecplot header
	//(*myfile) << "ZONE I=" << m_n+1 << ", F=POINT, C=GREEN\n";
	for (int i=0;i<m_n+1;i++)
		(*myfile) << m_controls[i][0] << '\t' << m_controls[i][1] << '\t' << m_controls[i][2] << '\n';	

	myfile->close();

}

vector3D BSplineCurve::der1(double t)
{
	vector3D point(0,0,0);
	int iStart;

	double* basis = 0;
	double* b1der = 0;


	iStart=basisf(t,m_deg+1,basis,b1der);
	int i;
	double denom1,num1;
	num1 = 0.;
	denom1 = 0.;
	if( m_isPolynom )  {	// Polinomial case
		for (i=iStart-m_deg; i<=iStart; i++)
			point = point + m_controls[i]*b1der[i];
	}
	else
	{
		for (i=iStart-m_deg; i<=iStart; i++) {
			num1 = num1 + m_weights[i]*b1der[i];
			denom1 = denom1 + m_weights[i]*basis[i];
		}
		
		for (i=iStart-m_deg; i<=iStart; i++)
			point = point + m_controls[i]*(m_weights[i]*b1der[i]*denom1 - m_weights[i]*basis[i]*num1);
		
	}
	if( ! m_isPolynom )	point /= (denom1*denom1);

	delete[] basis;
	delete[] b1der;

	return point;
}

void BSplineCurve::der12(double t, vector3D& d1, vector3D& d2)
{
	vector3D point1(0,0,0);
	vector3D point2(0,0,0);
	int iStart;

	double* basis = 0;
	double* b1der = 0;
	double* b2der = 0;

	iStart=basisf(t,m_deg+1,basis,b1der,b2der);
	int i;
	double denom1, denom12, num1, num12, num2;
	num1 = 0.;
	num12 = 0.;
	num2 = 0.;
	denom1 = 0.;
	denom12 = 0.;

	if( m_isPolynom )  {	// Polinomial case
		for (i=iStart-m_deg; i<=iStart; i++) {
			point1 = point1 + m_controls[i]*b1der[i];
			point2 = point2 + m_controls[i]*b2der[i];
		}
	}
	else
	{
		for (i=iStart-m_deg; i<=iStart; i++) {
			num1 = num1 + m_weights[i]*b1der[i];
			num2 = num2 + m_weights[i]*b2der[i];
			denom1 = denom1 + m_weights[i]*basis[i];
		}
		
		denom12 = denom1*denom1;
		num12 = num1*num1;

		for (i=iStart-m_deg; i<=iStart; i++)
			point1 = point1 + m_controls[i]*(m_weights[i]*b1der[i]*denom1 - m_weights[i]*basis[i]*num1);

		for (i=iStart-m_deg; i<=iStart; i++)
			point2 = point2 + m_controls[i]*(m_weights[i]*b2der[i]*denom12 - \
			m_weights[i]*b1der[i]*num1*denom1 - \
			(m_weights[i]*b1der[i]*num1 + m_weights[i]*basis[i]*num2)*denom1 + \
			2*m_weights[i]*basis[i]*num12);	
	}
	if( ! m_isPolynom )	{
		point1 /= denom12;
		point2 /= denom12*denom1;
	}

	d1 = point1;
	d2 = point2;

	delete[] b2der;
	delete[] b1der;
	delete[] basis;
}


