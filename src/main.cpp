#include "stdafx.h"

#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "kernel.h"
#include "gridgeneration.h"
#include <cmath>
#include <random>

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}


//Shear layer computation
class ImpactShockInitialConditions : public InitialConditions::InitialConditions
{
private:
	//State of target	
	double _u0;
	double _roL;
	double _roR;
	int _nmatLeft;
	int _nmatRight;
	//Speed of projectile	
	double _V;		
public:	

	ImpactShockInitialConditions(double V, int nmatLeft, double roL, int nmatRight, double roR) {
		_V = V;
		_roL = roL;
		_nmatLeft = nmatLeft;
		_roR = roR;
		_nmatRight = nmatRight;
	};

	virtual int getInitialGasModelIndex(const Cell& cell) {
		//Cell center
		double x = cell.CellCenter.x;
		double y = cell.CellCenter.y;
		double z = cell.CellCenter.z;
		
		if (x <= 0) {
			return _nmatLeft; //Pb
		} else {
			return _nmatRight; //Steel			
		};
	};

	virtual std::vector<double> getInitialValues(const Cell& cell) {
		int nmat = getInitialGasModelIndex(cell); //get material index

		std::vector<double> initValues;				
		//Other velocities
		double v = 0;
		double w = 0;

		//Internal energy
		double e = 0;

		//Left state		
		double roL = _roL;		
		double uL = 0;
			
		//Right state		
		double roR = _roR;		
		double uR = -1.0*_V;

		//Cell center
		double x = cell.CellCenter.x;
		double y = cell.CellCenter.y;
		double z = cell.CellCenter.z;
				
		//Values
		double u = 0;
		double ro = 0;
		double roE = 0;				
		if (x <= 0) {
			ro = roL;
			u = uL;			
		} else {
			ro = roR;
			u = uR;						
		};
			
		//Convert to conservative variables
		double P = 1e5;
		//e = P / (0.4 * ro);
		roE = ro*(e + (u*u + v*v + w*w) / 2.0);
		initValues.resize(_gasModels[nmat]->nConservativeVariables);
		initValues[0] = ro;
		initValues[1] = ro * u;
		initValues[2] = ro * v;
		initValues[3] = ro * w;
		initValues[4] = roE;// + (u*u + v*v + w*w) / 2.0); //TO DO check
		return initValues;
	};
};

void runImpactShockTest(int argc, char *argv[]) {
	Kernel _kernel;
	const double PI = 3.14159265359;
	
	_kernel.Initilize(&argc, &argv);
	double L = 15e-3; // 15 mm; 
	Grid _grid = GenGrid1D(_kernel.getParallelHelper(), 1000, -L, L, false); //Change grid size here
	_kernel.BindGrid(_grid);
	//_kernel.LoadGrid("C:\\Users\\Ilya\\Downloads\\cilindr5.11(1).cgns");

	//Fill in configuration
	_kernel.ReadConfiguration(""); //Change run parameters here
	_kernel.InitCalculation();

	//Initial conditions
	//double ro0 = 1;
	double roPb = 1000 * 1.0 / 0.88200003E-01; // SI	for Pb
	double roSteel = 1000 * 1.0 / 0.127; // SI	for stainless steel	
	double V = 500; //m/s
	ImpactShockInitialConditions ic(V, 1, roSteel, 2, roPb);
	//ImpactShockInitialConditions ic(V, 1, roSteel, 1, roSteel);
	//ImpactShockInitialConditions ic(V, 2, roPb, 2, roPb);
	_kernel.GenerateInitialConditions(&ic);	

	//Run test
	_kernel.RunCalculation();
	_kernel.FinalizeCalculation();

	//Output result
	_kernel.SaveGrid("result.cgns");
	_kernel.SaveSolution("result.cgns", "Solution");
	_kernel.Finalize();	
};

void runImpactShockTest2D(int argc, char *argv[]) {
	Kernel _kernel;
	const double PI = 3.14159265359;
	
	_kernel.Initilize(&argc, &argv);
	double L = 15e-3; // 15 mm; 
	Grid _grid = GenGrid2D(_kernel.getParallelHelper(), 200, 100, -3*L, L, -L, L, 1.0, 1.0, false, false); //Change grid size here
	//Modify grid for initial disturbances
	double A = 0; //4e-5;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-A,A);	
	for (Node& n : _grid.localNodes) {
		if ((n.P.x == 0) && (std::abs(n.P.y) != L)) {
			double dr = distribution(generator);
			n.P.x += dr;
		};
	};
	_kernel.BindGrid(_grid);
	//_kernel.LoadGrid("C:\\Users\\Ilya\\Downloads\\cilindr5.11(1).cgns");

	//Fill in configuration
	_kernel.ReadConfiguration(""); //Change run parameters here
	_kernel.InitCalculation();

	//Initial conditions
	//double ro0 = 1;
	double roPb = 1000 * 1.0 / 0.88200003E-01; // SI	for Pb
	double roSteel = 1000 * 1.0 / 0.127; // SI	for stainless steel	
	double V = 500; //m/s
	//ImpactShockInitialConditions ic(V, 0, roSteel, 0, roPb);
	ImpactShockInitialConditions ic(V, 1, roSteel, 2, roPb);
	//ImpactShockInitialConditions ic(V, 1, roSteel, 1, roSteel);
	//ImpactShockInitialConditions ic(V, 2, roPb, 2, roPb);
	_kernel.GenerateInitialConditions(&ic);	

	//Run test
	_kernel.RunCalculation();
	_kernel.FinalizeCalculation();

	//Output result
	_kernel.SaveGrid("result.cgns");
	_kernel.SaveSolution("result.cgns", "Solution");
	_kernel.Finalize();	
};

//Main program ))
 int main(int argc, char *argv[]) {
	//runPeriodicTest2D(argc, argv);
	//runSodTest(argc, argv);
	//runShearLayer(argc, argv);

 	runImpactShockTest2D(argc, argv);

	//LomonosovFortovGasModel gasModel(1);
	//double P;
	//double c;
	//bool nonPhysical;
	//double ro = 1./0.88200003E-01;
	//double e = 0;	
	//double Gr = 0;
	//gasModel.EOSE5(ro, e, P, c, Gr, nonPhysical);
	//double P_SI = 1e5;
	//double ro_SI = ro * 1000;
	////e = gasModel.FindInternalEnergy(ro_SI, P_SI);
	//std::cout<<"Pressure = "<<P<<"\n";
	////std::cout<<"Energy = "<<e<<"\n";
	return 0;	
};
