#pragma once
#include <map>
#include "FactorioClasses.h"

class TrainStopControl
{
	std::map<TrainStop*, int> _CRCs;
	TrainManager *_trainManager;

public:
	explicit TrainStopControl(TrainManager *manager) : _trainManager(manager) {}

	void disableTrainStop(TrainStop *trainStop) //Disables train stop by changing it's CRC to 0
	{
		if (!trainStop->stationId.crc) return;

		_CRCs[trainStop] = trainStop->stationId.crc;
		trainStop->stationId.crc = 0;
	}

	void enableTrainStop(TrainStop *trainStop)
	{
		auto i = _CRCs.find(trainStop);

		if (i == _CRCs.end()) return;

		trainStop->stationId.crc = i->second;

		_CRCs.erase(i);
	}

	bool trainStopEnabled(TrainStop *trainStop)
	{
		return _CRCs.find(trainStop) == _CRCs.end();
	}

	void beginSave() //Restores CRC before saving the game...
	{
		for (auto &i : _CRCs)
		{
			i.first->stationId.crc = i.second;
		}
	}

	void endSave()	//...and changes it back when save process finished
	{
		for (auto &i : _CRCs)
		{
			i.first->stationId.crc = 0;
		}
	}

	int trainsGoingToStation(TrainStop *trainStop) const //Returns the number of trains enroute to trainStop
	{
		auto tc = _trainManager->getTrainsCount();
		auto ta = _trainManager->getTrainsArray();

		int result = 0;

		for (int i = 0; i < tc; ++i)
		{
			if (ta[i]->trainDestination() == trainStop || TrainFuncs::getTrainStop(ta[i]) == trainStop) result++;
		}

		return result;
	}

	static int64_t trainStopCapacity(TrainStop *trainStop)  //Returns maximum number of trains the trainStop can accept:
	{														//reads Licomotive signal from train stop
		auto circuitConnector = TrainStopFuncs::getCircuitConnector(trainStop, 1);

		CircuitNetwork *redNetwork = nullptr,
			*greenNetwork = nullptr;

		if (circuitConnector)
		{
			redNetwork = CircuitConnectorFuncs::getNetwork(circuitConnector, CircuitConnectorFuncs::Red); //1 for both networks?? 
			greenNetwork = CircuitConnectorFuncs::getNetwork(circuitConnector, CircuitConnectorFuncs::Green);
		}

		if(redNetwork || greenNetwork)
		{
			SignalID id;
			id.itemIndex = 0x23; //Locomotive index

			int64_t signalVal = 0;
			
			if (redNetwork) signalVal += CircuitNetworkFuncs::getValue(redNetwork, &id);
			if (greenNetwork) signalVal += CircuitNetworkFuncs::getValue(greenNetwork, &id);

			return signalVal;
		}

		return 1000000; //Default value for not connected to circuit network train stops
	}

	void update()
	{
		auto tsc = _trainManager->getTrainStopsCount();
		auto tss = _trainManager->getTrainStopsArray();

		for (int i = 0; i < tsc; ++i)
		{
			if (trainsGoingToStation(tss[i]) >= trainStopCapacity(tss[i])) disableTrainStop(tss[i]);
			else enableTrainStop(tss[i]);
		}
	}
};
