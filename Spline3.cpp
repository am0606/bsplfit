#include "Spline3.h"

Spline3D::Spline3D()
{
	m_nsegm = 0;
	m_knots = 0;
	m_controls = 0;
	m_ders = 0;
}

//копирующий конструктор
Spline3D::Spline3D(const Spline3D& crv)
{
	this->m_controls = 0;
	this->m_knots = 0;
	this->m_ders = 0;
	this->Init(crv.m_nsegm,crv.m_knots,crv.m_controls,crv.m_ders);
}

Spline3D::~Spline3D()
{
	delete [] m_knots;
	delete [] m_controls;
	delete [] m_ders;
}

void Spline3D::Init(int nsegm, double* knots, vector3D* controls, vector3D* ders)
{
	//независимы от кратностей узлов: 
	//число контрольных точек;
	//размерность массива узлов;
	//суммарная кратность.

	m_nsegm = nsegm;

	//заполнение массива узлов; подсчёт числа сегментов

	assert( knots );
	m_knots = new double[m_nsegm+1];
	
	for (int i = 0; i <= m_nsegm; i++)
		m_knots[i] = knots[i];

	// Вершины определяющего многоугольника (опорные точки)
	assert( m_nsegm != 0 );
	assert( controls );
	
	//m_n-верхний индекс числа контрольных точек
	if (m_controls) delete[] m_controls;
	m_controls = new vector3D[m_nsegm+1];
	for( int i=0; i<=m_nsegm; i++ )
		m_controls[i] = controls[i];
	
	assert( ders );	
	if (m_ders) delete[] m_ders;
	m_ders = new vector3D[m_nsegm+1];
	for( int i=0; i<=m_nsegm; i++ )
		m_ders[i] = ders[i];
}

//0 - успешный импорт
//> 0 - ошибка импорта
int Spline3D::Import(const char* filename)
{
	std::ifstream input(filename);
	int n;
	double* knots = 0;
	vector3D* controls = 0;
	vector3D* ders = 0;
	
	std::string line;

	do { std::getline(input,line); } while (empty(line));
	int m_n_imp_result = getProperty(line, "n", n);
	if (m_n_imp_result == -1) return 1;
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
	else return 2;
	line.clear();
	
	do { std::getline(input,line); } while (empty(line));
	if (line.find("controls") != std::string::npos)
	{
		controls = new vector3D[n+1];
		for(int i=0; i<=n; i++)
			input >> controls[i];
	}
	else
	{
		if (knots) delete[] knots;
		return 3;
	}
	line.clear();

	do { std::getline(input,line); } while (empty(line));
	if (line.find("derivatives") != std::string::npos)
	{
		ders = new vector3D[n+1];
		for(int i=0; i<=n; i++)
			input >> ders[i];
	}
	else
	{
		if (knots) delete[] knots;
		if (controls) delete[] controls;
		return 4;
	}
	line.clear();


	Init(n, knots, controls, ders);
	
	if (knots) delete[] knots;
	if (controls) delete[] controls;
	if (ders) delete[] ders;

	input.close();
	return 0;
}

void Spline3D::Export(const char* filename)
{
	std::ofstream output(filename);
	output << "n\t" << m_nsegm << '\n';
	output << "knots\n";
	output << m_nsegm+1 << '\t';
	for(int i=0;i<m_nsegm;i++) output << m_knots[i] << '\t';
	output << m_knots[m_nsegm] << '\n';

	output << "\n\n";
	output << "controls\n";
	for(int i=0;i<=m_nsegm;i++)
		output << m_controls[i] << '\n';
	output << '\n';

	output << "derivatives\n";
	for(int i=0;i<=m_nsegm;i++)
		output << m_ders[i] << '\n';
	output << '\n';

	output.close();
}

vector3D Spline3D::Point1(int segm, double t)
{
	double tk1 = m_knots[segm+1] - m_knots[segm];
	assert( t >= 0 && t <= tk1 );
	
	vector3D Pk = m_controls[segm];
	vector3D Pk1 = m_controls[segm+1];
	vector3D Pkd = m_ders[segm];
	vector3D Pkd1 = m_ders[segm+1];

	double tau = t/tk1;
	double tau2 = tau*tau;
	double tau3 = tau2*tau;
	double F1 = 2*tau3 - 3*tau2 + 1;
	double F2 = -2*tau3+3*tau2;
	double F3 = tau*(tau2-2*tau+1)*tk1;
	double F4 = tau*(tau2-tau)*tk1;
	vector3D point = F1*Pk+F2*Pk1+F3*Pkd+F4*Pkd1;

	return point;
}

void Spline3D::outputpoints(std::ostream& out, int segm, double step)
{
	int npoints = (int)floor((m_knots[segm+1]-m_knots[segm])/step)+1;

	for (int i = 0; i < npoints; i++)
	{
		vector3D mypoint=Point1(segm,i*step);
		out << mypoint[0] << '\t' << mypoint[1] << '\t' << mypoint[2] << '\t' << std::endl;
	}
	out.flush();
}

void Spline3D::outputpoints(const char *output, double step)
{
	std::ofstream myfile(output);
	for (int is=0;is<m_nsegm;is++)
		outputpoints(myfile,is,step);
	myfile.close();
}
