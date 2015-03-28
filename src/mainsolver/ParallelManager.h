#ifndef NewKidzTurbo_MainSolver_ParallelManager
#define NewKidzTurbo_MainSolver_ParallelManager

#include "mpi.h"
#include <chrono>

//Public structure for parallel manager properties
struct ParallelManagerProperties {

};

//MPI parallel functions implementation
class ParallelManager {
	//General MPI information
	bool _MPIinitialized;
	MPI_Status _status;

	//Communicator information
	int _rank;
	int _np;
	MPI_Comm _comm;

	//MPI communication time accounting data
	std::chrono::high_resolution_clock::duration _syncDuration; //total synchronisation time
	std::chrono::high_resolution_clock::duration _commDuration; //total communication time
public:
	//Public methods

	//Data accessors
	inline const int& np() { return _np; };
	inline const int& rank() { return _rank; };
	inline const MPI_Comm& comm() { return _comm; };

	//Public constructor initializes MPI
	ParallelManager(int argc, char *argv[]) {
		MPI_Init(&argc, &argv);
		_comm = MPI_COMM_WORLD;
		MPI_Comm_rank(_comm, &_rank);
		MPI_Comm_size(_comm, &_np);
		_syncDuration = std::chrono::high_resolution_clock::duration(0);
		_commDuration = std::chrono::high_resolution_clock::duration(0);

		_MPIinitialized = false;
		MPI_Initialized((int*)&_MPIinitialized);
	};

	//Public destructor finilizes MPI 
	~ParallelManager() {
		if (_MPIinitialized) MPI_Finalize(); 
	};

	//=============================== Synchronisation primitives =====================================

	//Barrier synchronisation
	void Barrier() {
		auto before = std::chrono::high_resolution_clock::now();
		MPI_Barrier(_comm);
		auto after = std::chrono::high_resolution_clock::now();
		_syncDuration += after - before; // update synchronisation time duration
	};

	//Send synchronisation signal to target process
	void SendSignal(int rankDestination) {
		auto before = std::chrono::high_resolution_clock::now();
		MPI_Barrier(_comm);
		auto after = std::chrono::high_resolution_clock::now();
		_syncDuration += after - before; // update synchronisation time duration
	};

	//Blocking wait for synchronisation signal from source process
	void WaitSignal(int rankSource) {
		auto before = std::chrono::high_resolution_clock::now();
		MPI_Barrier(_comm);
		auto after = std::chrono::high_resolution_clock::now();
		_syncDuration += after - before; // update synchronisation time duration
	};

	//Data communication primitives
	template<class DataTypeClass, MPI_Datatype MPIDataTypeClass>
	void Allgatherv(std::vector<DataTypeClass>& local, std::vector<DataTypeClass>& counts, std::vector<DataTypeClass>& result) {
		//Make displacement array
		std::vector<int> displs(counts.size());
		int totalSize = 0;
		for (int i = 0; i<counts.size(); i++) {
			displs[i] = totalSize;
			totalSize += counts[i];
		};

		//Prepare recieving buffer
		if (result.size() != totalSize) result.resize(totalSize);

		//Call MPI and measure time
		auto before = std::chrono::high_resolution_clock::now();
		MPI_Allgatherv(&local.front(), local.size(), MPIDataTypeClass, &result.front(), &counts.front(), &displs.front(), MPIDataTypeClass, _comm);	
		auto after = std::chrono::high_resolution_clock::now();
		_commDuration += after - before; // update data communication time duration
	};

	
};

#endif