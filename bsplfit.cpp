#include <fstream>
#include <sstream>
#include "BSplineCurve.h"
#include "Spline3.h"
#include "strtools.h"

void importSpline(const char* input, Spline3D& spcurve)
{
	int errcode = spcurve.Import(input);
	if (errcode)
	{
		std::cout << "Input " << input << ": error of the spline curve import!" << std::endl;
		exit(1);
	}
}

void importBSpline(const char* input, BSplineCurve& bspcurve)
{
	int errcode = bspcurve.Import(input);
	if (errcode)
	{
		std::cout << "Input " << input << ": error of the B-spline curve import!" << std::endl;
		exit(1);
	}
}

void testSpline(const char* input)
{
	Spline3D spcurve;
	importSpline(input, spcurve);
	int segm = 0;
	double step = 0.001;
	std::ofstream segm0_stream("output/spoints0.txt");
	spcurve.outputpoints(segm0_stream,segm,step);
	spcurve.outputpoints("output/spoints.txt",step);
}

void testBSpline(const char* input)
{
	BSplineCurve bspcurve;
	importBSpline(input, bspcurve);

	std::cout << bspcurve.m_nsegm << " bspline curve segments\n";
	const int npar = 5;
	double t[npar] = {0.1, 0.2, 0.3, 0.73, 1};
	std::ofstream points_file("output/selected_points.txt");
	std::ofstream ders_file("output/selected_ders.txt");
	for(int i=0;i<npar;i++)
	{
		vector3D xpoint = bspcurve.Point1(t[i]);
		vector3D xder = bspcurve.der1(t[i]);
		points_file << xpoint[0] << '\t' << xpoint[1] << '\t' << xpoint[2] << std::endl;
		ders_file << xder[0] << '\t' << xder[1] << '\t' << xder[2] << std::endl;
		std::cout << t[i] << '\t' << xpoint << '\t' << xder << std::endl;
	}
	points_file.close();
	ders_file.close();

	bspcurve.outputvertexes("output/controlPoints.txt", 0);
	double step = 0.001;
	bspcurve.outputpoints("output/bspoints.txt",step);
}

void testBasis(const char* input)
{
	BSplineCurve bspcurve;
	importBSpline(input,bspcurve);
	std::cout << bspcurve.m_nsegm << " bspline curve segments\n";
	char* prefix = "output/basis";
	double step = 0.001;
	std::cout << bspcurve.m_n + 1 << " basis functions" << std::endl;
	bspcurve.basiscurves(prefix, step);
}

void fitSegment(int xseg, const Spline3D& spline, BSplineCurve& bspcurve)
{
	std::vector<double> sp_knots;
	spline.get_knots(sp_knots);
	int deg = 3;
	//по одной кривой на каждый сегмент
	vector3D* sp_controls;
	spline.get_controls(sp_controls);
	vector3D* sp_ders;
	spline.get_ders(sp_ders);
	int nsegm = spline.get_nseg();
	assert(xseg >= 0 && xseg <= nsegm);
	vector3D P0 = sp_controls[xseg];
	vector3D Pn = sp_controls[xseg+1];
	vector3D C0 = sp_ders[xseg];
	vector3D C1 = sp_ders[xseg+1];
	double u1 = sp_knots[xseg];
	double u2 = sp_knots[xseg+1];
	vector3D P1 = P0 + (u2-u1)*C0/deg;
	vector3D Pn1 = Pn - (u2-u1)*C1/deg;
	vector3D* controls = new vector3D[deg+1];
	controls[0] = P0;
	controls[1] = P1;
	controls[2] = Pn1;
	controls[3] = Pn;
	double* knots = new double[2*(deg+1)];
	for(int i=0;i<=deg;i++)
		knots[i] = u1;
	for(int i=deg+1;i<2*(deg+1);i++)
		knots[i] = u2;
	
	bool isClosed = false;
	bool isPolinom = true;
	bool isPeriodic = false;
	bspcurve.Init(isClosed,isPolinom,isPeriodic,deg,deg,knots,controls,0);
	vector3D xder0 = bspcurve.der1(knots[0]);
	vector3D xder1 = bspcurve.der1(knots[deg+1]);
	std::cout << knots[0] << '\t' << xder0 << std::endl;
	std::cout << knots[deg+1] << '\t' << xder1 << "\n\n";

	delete[] controls;
	delete[] knots;
	delete[] sp_controls;
	delete[] sp_ders;
}

void fitCurve(const Spline3D& spline, BSplineCurve& bspcurvec)
{
	int nsegm = spline.get_nseg();
	int deg=3;
	//контрольные точки составной кривой
	vector3D* controls = new vector3D[3*nsegm+1];
	std::vector<double> sp_knots;
	spline.get_knots(sp_knots);	
	unsigned n_sp_knots = sp_knots.size();
	std::cout << "knots size of a spline curve = " << n_sp_knots << std::endl;
	unsigned n_knots = (nsegm+1)*deg+2;
	std::cout << "knots size of a composite curve = " << n_knots << std::endl;
	//unsigned n_knots1 = (n_sp_knots-2)*deg+2*(deg+1);	
	std::vector<double> knots; ///размер (nsegm-1)*deg + 2*(deg+1) или (n_sp_knots-2)+2*(deg+1)
	std::vector<BSplineCurve> curves;
	for(int i=0;i<nsegm;i++)
	{
		BSplineCurve bspcurve;
		fitSegment(i, spline, bspcurve);
		std::ostringstream fitseg_stream;
		fitseg_stream << "output/fitSegment_" << i << ".txt";

		bspcurve.Export(fitseg_stream.str().c_str());
		
		std::ostringstream vertseg_stream;
		vertseg_stream << "output/vertexes_seg_" << i << ".txt";

		bspcurve.outputvertexes(vertseg_stream.str().c_str(),false);
		
		double step = 0.01;
		std::ostringstream bspoints_seg_stream;
		bspoints_seg_stream << "output/bspoints_seg_" << i << ".txt";
		bspcurve.outputpoints(bspoints_seg_stream.str().c_str(),step);
		curves.push_back(bspcurve);
	}
	
	knots.push_back(sp_knots[0]);
	for(unsigned j=0;j<n_sp_knots;j++)
		for (int k=0;k<deg;k++)
			knots.push_back(sp_knots[j]);
	knots.push_back(sp_knots[n_sp_knots-1]);

	vector3D* seg_controls0;
	curves[0].get_controls(seg_controls0);
	unsigned n = 0;
	for (int i=0;i<=deg;i++)
		controls[n++] = seg_controls0[i];
	delete[] seg_controls0;
	for (unsigned i=1;i<curves.size();i++)
	{
		BSplineCurve bspcurve = curves[i];
		vector3D* seg_controls;
		bspcurve.get_controls(seg_controls);
		for (int i=1;i<=deg;i++)
			controls[n++] = seg_controls[i];
		delete[] seg_controls;
	}
	n = n-1; //верхний индекс числа контрольных точек составной кривой
	
	bool isClosed = false;
	bool isPolinom = true;
	bool isPeriodic = false;
	bspcurvec.Init(isClosed,isPolinom,isPeriodic,n,deg,&knots[0],controls,0);

	delete[] controls;
}

void fitCurveDrv(const char* input)
{
	Spline3D spline;
	importSpline(input, spline);
	BSplineCurve bspcurvec;
	fitCurve(spline,bspcurvec);
	bspcurvec.Export("output/bspcurvec.dat");
	double step = 0.001;
	bspcurvec.outputpoints("output/bscurvec_points.txt",step);
}

//удаляем кратные узлы с конца
void removeKnot(int knum, BSplineCurve& bspcurve)
{
	std::vector<double> knots;
	bspcurve.get_knots(knots);
	double ud = knots[knum]; //knot value to delete
	knots.erase(knots.begin() + knum);
	knum = knum-1; //узел не вставляется, а удаляется, поэтому оперировать нужно преобразованным узловым вестором
	int sz_knots = (int)knots.size();
	for(int i=0;i<sz_knots;i++)
		std::cout << '(' << i << ')' << knots[i] << ' ';
	std::cout << std::endl;
	vector3D* controls;
	bspcurve.get_controls(controls);
	int deg=3;
	assert(knum < sz_knots-deg-1 && knum > deg); //открытый B-spline
	int n = bspcurve.get_n()-1;
	vector3D* new_controls = new vector3D[n+1]; //на одну точку меньше
	for(int i=0;i<knum-deg+1;i++) 
		new_controls[i] = controls[i];
	//for(int i=knum+1;i<=n;i++)
	//for(int i=knum-deg+1;i<=n;i++)
		//new_controls[i] = controls[i+1];
	for(int i=knum-deg+1;i<=n+1;i++)
		new_controls[i-1] = controls[i];
	bool forward = false;
	if (forward)
		for(int i=knum-deg+1;i<=knum;i++) //knot multiplicity = 3
		{
			std::cout << "knum = " << knum << std::endl;
			std::cout << "i = " << i << std::endl;
			std::cout << "knots[i] = " << knots[i] << std::endl;
			std::cout << "i+deg = " << i+deg+1 << std::endl;
			std::cout << "knots[i+deg] = " << knots[i+deg] << std::endl;
			double alphai = (ud-knots[i])/(knots[i+deg]-knots[i]);
			std::cout << "alphai = " << alphai << std::endl;
			if (alphai > 0) new_controls[i] = (controls[i]-(1-alphai)*new_controls[i-1])/alphai;
			//else new_controls[i-1] = controls[i];
			std::cout << "c[i] = " << controls[i][0] << ", c[i+1] = " << controls[i+1][0] << ", n_c[i-1] = " << new_controls[i-1][0] << ", n_c[i] = " << new_controls[i][0] << ", n_c[i+1] = " << new_controls[i+1][0] << "\n\n";
		}
	else
		for(int i=knum;i>=knum-deg+1;i--) //knot multiplicity = 3, reverse
		{
			std::cout << "knum = " << knum << std::endl;
			std::cout << "i = " << i << std::endl;
			std::cout << "knots[i] = " << knots[i] << std::endl;
			std::cout << "i+deg = " << i+deg << std::endl;
			std::cout << "knots[i+deg] = " << knots[i+deg] << std::endl;
			double alphai = (ud-knots[i])/(knots[i+deg]-knots[i]);
			std::cout << "alphai = " << alphai << std::endl;
			if (1-alphai > 0) new_controls[i-1] = (controls[i]-alphai*new_controls[i])/(1-alphai);
			//else new_controls[i] = controls[i];
			std::cout << "c[i] = " << controls[i][0] << ", c[i+1] = " << controls[i+1][0] << ", n_c[i-1] = " << new_controls[i-1][0] << ", n_c[i] = " << new_controls[i][0] << ", n_c[i+1] = " << new_controls[i+1][0] << "\n\n";
		}

	bspcurve.InitG(n,deg,&knots[0]);
	bspcurve.InitControls(new_controls);
	bspcurve.Export("output/bspcurvec_new.dat");
	double step = 0.001;
	bspcurve.outputpoints("output/bscurvec_points_new.txt",step);
	bspcurve.outputvertexes("output/bscurvec_vertexes_new.txt",0);

	delete[] controls;
	delete[] new_controls;
}

void optimizeCurve(BSplineCurve& bspcurve)
{
	std::vector<double> knots;
	bspcurve.get_knots(knots);	

	int deg=3;
	int nsegm = bspcurve.get_nseg();
	int nd = 0;
	for(int i=0;i<nsegm-1;i++)
	{
		//if (i < 17 || i > 24) 
		//	continue;
		for(int j=deg;j>deg-2;j--)
		{
			int del_knot_idx = (i+2)*deg-nd;
			removeKnot(del_knot_idx, bspcurve);
			nd++;
		}
	}
}

void optimizeCurveDrv(const char* input)
{
	BSplineCurve bspcurve;
	importBSpline(input, bspcurve);
	optimizeCurve(bspcurve);
}

void fitOptimizeCurve(const char* input)
{
	Spline3D spline;
	importSpline(input, spline);
	BSplineCurve bspcurvec;
	fitCurve(spline,bspcurvec);
	bspcurvec.Export("output/bspcurvec.dat");
	optimizeCurve(bspcurvec);
}

void printUsageInfo()
{
	std::cout << "Usage: bsplfit \"functionName\" \"input filename\"" << std::endl;
	std::cout << "Usage example 1: ./bsplfit testSpline input/s_curve.dat" << std::endl;
	std::cout << "Usage example 2: ./bsplfit testBSpline input/bs_curve.dat" << std::endl;
	std::cout << "Usage example 2: ./bsplfit testBasis input/bs_curve.dat" << std::endl;
	std::cout << "Usage example 4: ./bsplfit fitCurve input/s_curve.dat" << std::endl;
	std::cout << "Usage example 5: ./bsplfit optimizeCurve input/bs_curve_mult_knots.dat" << std::endl;
	std::cout << "Usage example 6: ./bsplfit fitOptimizeCurve input/s_curve.dat" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc == 3)
	{
		const char* inputname = argv[2];
		if (equals(argv[1], "testSpline")) testSpline(inputname);
		else if (equals(argv[1], "testBSpline")) testBSpline(inputname);
		else if (equals(argv[1], "testBasis")) testBasis(inputname);
		else if (equals(argv[1], "fitCurve")) fitCurveDrv(inputname);
		else if (equals(argv[1], "optimizeCurve")) optimizeCurveDrv(inputname);
		else if (equals(argv[1], "fitOptimizeCurve")) fitOptimizeCurve(inputname);
		else printUsageInfo();
	}
	else printUsageInfo();
	return 0;
}

