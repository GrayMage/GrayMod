#include <windows.h>

#include "../Libs/minhookex/MinHookEx/MinHookEx.h"
#include "TrainStopControl.h"

auto &hooks = CMinHookEx::getInstance();

size_t baseAddr;

template<typename T>
void initFunc(T &func)
{
	auto addr = (size_t)func;
	addr += baseAddr;
	func = (T)addr;
}

std::unique_ptr<TrainStopControl> trainStopControl;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			baseAddr = (size_t)GetModuleHandle("factorio.exe");

			initFunc(CircuitNetworkFuncs::onChestContentsChange);
			initFunc(CircuitNetworkFuncs::getValue);
			initFunc(TrainStopControlBehaviorFuncs::connectedToCircuitNetwork);
			initFunc(TrainStopFuncs::getCircuitConnector);
			initFunc(TrainStopFuncs::getOrCreateControlBehavior);
			initFunc(RailPathFuncs::getEndRail);
			initFunc(CircuitConnectorFuncs::getNetwork);
			initFunc(TrainManagerFuncs::TrainManagerCtor);
			initFunc(TrainManagerFuncs::getFirstStopWithStation);
			initFunc(TrainManagerFuncs::update);
			initFunc(TrainManagerFuncs::save);
			initFunc(TrainFuncs::update);
			initFunc(TrainFuncs::onGotOutOfStation);
			initFunc(TrainFuncs::getTrainStop);
			initFunc(TrainFuncs::deletePath);
			initFunc(TrainFuncs::recalculatePath);
			initFunc(TrainManagerFuncs::setup);

			hooks.addHook(TrainFuncs::update, [](Train *pTrain)
				{
					auto pDest = pTrain->trainDestination();

					if (!pDest)
					{
						hooks.at(TrainFuncs::update).originalFunc(pTrain);
						return;
					}

					trainStopControl->update();

					if (!trainStopControl->trainStopEnabled(pDest))
					{
						if (trainStopControl->trainStopCapacity(pDest) - trainStopControl->trainsGoingToStation(pDest) >= 0) trainStopControl->enableTrainStop(pDest);
						else
						{
							TrainFuncs::recalculatePath(pTrain, true);
						}
					}

					hooks.at(TrainFuncs::update).originalFunc(pTrain);
				}).enable();

			auto init = [](TrainManager *pTrainManager)
				{
					trainStopControl = std::make_unique<TrainStopControl>(pTrainManager);

					auto trains = pTrainManager->getTrainsArray();
					auto trainCount = pTrainManager->getTrainsCount();

					for (auto i = 0; i < trainCount; i++)
					{
						auto t = trains[i];

						if (t->getState() == Train::MANUAL_CONTROL || TrainFuncs::getTrainStop(t)) continue;

						TrainFuncs::deletePath(t);				//Causes desync in MP??
						TrainFuncs::recalculatePath(t, true);
					}

					Beep(300, 100);
				};

			hooks.addHook(TrainManagerFuncs::setup, [=](TrainManager *pTrainManager, void *dummy)
				{
					hooks.at(TrainManagerFuncs::setup).originalFunc(pTrainManager, dummy);

					init(pTrainManager);
				}).enable();

			hooks.addHook(TrainManagerFuncs::update, [=](TrainManager *pTrainManager, bool b)
				{
					init(pTrainManager);

					hooks.at(TrainManagerFuncs::update).disable();
					return hooks.at(TrainManagerFuncs::update).originalFunc(pTrainManager, b);
				}).enable();

			hooks.addHook(TrainManagerFuncs::save, [](TrainManager *pManager, void *a)
				{
					trainStopControl->beginSave();

					hooks.at(TrainManagerFuncs::save).originalFunc(pManager, a);

					trainStopControl->endSave();
				}).enable();

			Beep(300, 500);
		}
		break;

	case DLL_PROCESS_DETACH:

		if(trainStopControl) trainStopControl->beginSave(); //Restore CRC's
		hooks.removeAll();
		break;
	}

	return TRUE;
}
