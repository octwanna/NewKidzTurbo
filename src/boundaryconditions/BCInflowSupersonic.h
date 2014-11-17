#ifndef TURBO_BoundaryConditions_BCInflowSupersonic
#define TURBO_BoundaryConditions_BCInflowSupersonic

#include "BoundaryCondition.h"

namespace BoundaryConditions {

class BCInflowSupersonic : public BoundaryConditions::BoundaryCondition {
public:
	Vector Velocity;
	double Density;
	double InternalEnergy;

	std::vector<double> getDummyValues(std::vector<double> values, const Cell& dummyCell) {
		//Obtain face
		if (dummyCell.Faces.size() != 1) throw new Exception("Dummy cell has more than one face");
		Face& face = _grid->localFaces[dummyCell.Faces[0]];
		//Obtain neighbour cell
		int nCellIndex = _grid->cellsGlobalToLocal[face.FaceCell_1];
		int nVariables = _gasModel->nConservativeVariables;

		//Compute dummy values
		std::vector<double> res(nVariables);	
		res[0] = Density;
		res[1] = Velocity.x * Density;
		res[2] = Velocity.y * Density;
		res[3] = Velocity.z * Density;
		res[4] = InternalEnergy * Density;
		return res;
	};

	void loadConfiguration(BoundaryConditionConfiguration& bcConfig) {			
		//Read configuration
		if (_gasModel->GasModelName == "LomonosovFortovGasModel") {
			std::pair<double, bool> propertyValue;
			propertyValue = bcConfig.GetPropertyValue("Density");
			Density = propertyValue.first;
			propertyValue = bcConfig.GetPropertyValue("VelocityX");
			Velocity.x = propertyValue.first;
			propertyValue = bcConfig.GetPropertyValue("VelocityY");
			Velocity.y = propertyValue.first;
			propertyValue = bcConfig.GetPropertyValue("VelocityZ");
			Velocity.z = propertyValue.first;
			propertyValue = bcConfig.GetPropertyValue("InternalEnergy");
			InternalEnergy = propertyValue.first;
		};
	}; 

};

};

#endif