#pragma once

class TrainStopControlBehavior
{
	virtual ~TrainStopControlBehavior() = 0;
};

namespace TrainStopControlBehaviorFuncs
{
	bool (__cdecl *connectedToCircuitNetwork)(TrainStopControlBehavior *pThis) = (decltype(connectedToCircuitNetwork))0x769ee0;
}

struct SignalID
{
	enum ContainedType
	{ Item, Fluid, VirtualSignal };

	ContainedType type = Item;
	uint16_t itemIndex;
	uint16_t fluidIndex;
	uint16_t virtSignalIndex;

	uint16_t unknown;
};

class CircuitNetwork
{
public:
	/*
		const std::map<SignalID, int>* valuesThisTick()
		{
			return (std::map<SignalID, int>*)((char*)this + 0x18);
		}
	*/
};

namespace CircuitNetworkFuncs
{
	void (__fastcall *onChestContentsChange)(CircuitNetwork *pNetwork, SignalID *pSignalID, int change) = (decltype(onChestContentsChange))0x1ed40;
	int64_t (__fastcall *getValue)(CircuitNetwork *pNetwork, SignalID *id) = (decltype(getValue))0x7B94F0;
}

struct __declspec(align(8)) StationID //size 0x28
{
	char nameCont[0x1F]; //std::string name;// () { return std::string((char*)this); }; - doesn't work... Different std lib versions??
	unsigned int crc;	 
};

class CircuitConnector {};

namespace CircuitConnectorFuncs
{
	enum WireType
	{ Red = 2, Green = 3 };

	CircuitNetwork* (__fastcall *getNetwork)(CircuitConnector *pThis, WireType wireType) = (decltype(getNetwork))0x162270;
}

class Rail
{
	char unknown[0x140];
};

class TrainStop;

namespace TrainStopFuncs
{
	TrainStopControlBehavior* (__cdecl *getOrCreateControlBehavior)(TrainStop *pThis) = (decltype(getOrCreateControlBehavior))0x164810;
	CircuitConnector* (__fastcall *getCircuitConnector)(TrainStop *pThis, char circuitConnectorID) = (decltype(getCircuitConnector))0x1649a0; //circuitConnectorID always == 1 ! 0x1649a0
}

class TrainStop
{
public:
	char unknown[167 - sizeof(void*)]; //vtable size
	StationID stationId;
	Rail *rail;
	char unknown1[8];
	TrainStopControlBehavior *controlBehavior;

	virtual ~TrainStop() = 0;
};

class RailPath { };

namespace RailPathFuncs
{
	Rail* (__fastcall *getEndRail)(RailPath *pRailPath) = (decltype(getEndRail))0x3A1540;
}

class Train;

class TrainManager
{
public:
	TrainStop** getTrainStopsArray()
	{
		return *(TrainStop***)((size_t)this + 0x28 + 40);
	}

	size_t getTrainStopsCount()
	{
		auto count = ((int64_t)*(char**)((int64_t)this + 0x28 + 48) - (int64_t)getTrainStopsArray() + 7) / 8;
		return count < 0 ? 0 : count;
	}

	Train** getTrainsArray()
	{
		return *(Train***)((size_t)this + 0x28 + 88);
	}

	size_t getTrainsCount()
	{
		auto count = ((int64_t)*(char**)((int64_t)this + 0x28 + 96) - (int64_t)getTrainsArray() + 7) / 8;
		return count < 0 ? 0 : count;
	}
};

namespace TrainManagerFuncs
{
	void (__fastcall *TrainManagerCtor)(TrainManager *pTrainManager, void *map, void *input) = (decltype(TrainManagerCtor))0x31EAE0; //can't hook :-(
	TrainStop* (__fastcall *getFirstStopWithStation)(TrainManager *, StationID *) = (decltype(getFirstStopWithStation))0x322920;
	bool (__fastcall *update)(TrainManager *pTrainManager, bool moveTrains) = (decltype(update))0x31F4D0;
	void (__fastcall *save)(TrainManager *pTrainManager, void *pMapSerializer) = (decltype(save))0x31F1C0;
	void(__fastcall *setup)(TrainManager *pTrainManager, void *dummy) = (decltype(setup))0x31F880;
}

class Train
{
public:
	enum State
	{MANUAL_CONTROL = 9};

	char unknown[0x139];
	RailPath *path;
	TrainManager *pTrainManager;

	State getState()
	{
		return *(State*)((size_t)this + 0x70);
	}

	TrainStop* trainDestination() const
	{
		if (!path) return nullptr;

		Rail *trainDest = RailPathFuncs::getEndRail(path);

		if (!trainDest) return nullptr;

		auto count = pTrainManager->getTrainStopsCount();
		auto ts = pTrainManager->getTrainStopsArray();

		for (int i = 0; i < count; ++i)					//There's gotta be much better way to retrieve train destinantion..
		{
			if (ts[i]->rail == trainDest) return ts[i];
		}

		return nullptr;
	}
};

namespace TrainFuncs
{
	void (__fastcall *update)(Train *pTrain) = (decltype(update))0x2c1b00;
	void (__fastcall *onGotOutOfStation)(Train *pTrain) = (decltype(update))0x2C9460;
	TrainStop* (__fastcall *getTrainStop)(Train *pTrain) = (decltype(getTrainStop))0x2C3730;
	void (__fastcall *deletePath)(Train *pTrain) = (decltype(deletePath))0x2C09F0;
	void (__fastcall *recalculatePath)(Train *pTrain, bool bForce) = (decltype(recalculatePath))0x2C4A20;
}