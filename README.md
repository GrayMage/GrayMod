# GrayMod
GrayMod is a DLL which changes behavior of Factorio train stops

GrayMod is unofficial extension library for **Non-steam version of Factorio 0.14.21** (https://www.factorio.com). **No other versions are supported**. Be aware that no version checks of any kind performed, so make sure you have exactly v0.14.21.

Don't forget to make a backup copy of your save before using it.

GrayMod allows users to connect train stops to circuit network and restrict the number of trains a train stop can accept by sending a **Locomotive signal**. The connected train stop becomes disabled for all trains except those already enroute to it as I suggested here: https://forums.factorio.com/viewtopic.php?f=6&t=40919&sid=dc2776e50337b7196a73ddc1e149df32. The mentioned suggestion is the main reason I created this "mod". 

**I hope this train stop behavior will be added into the game in future releases :-)**

This package contains 3 projects: GrayMod, DllLoader and DllInjector. DllLoader uses DllInjector to inject GrayMod.dll into Factorio's address space. It is capable of injecting any DLL into any process, so use it with caution.

All projects are written in Microsoft Visual Studio 2015.

#Installation and usage
1. Clone this Repo.
2. Open solution file in MS Visual Studio 2015.
3. Press "Run".
4. Run factorio.exe.

Then you should be able to use it ingame.
You can eject the DLL at any time by typing 'x' character in console window, but it's recommended that you pause the game before doing so (just press ESC ingame). **Do not close the console while game is still running!** Close the game first or use 'x' as described above.

Since GrayMod changes game state, it can't be used in multiplayer games (although I might be fixing it later).

#Testing suggestions
There are two test maps included (see Saves folder):
- GMSmallMap - the map shown in this video: https://www.youtube.com/watch?v=eM_sAFMYXmc It should be easy to experiment with GrayMod there;
- GMBiggerMap - the map I've been playing on for a while, which inspired me to create this project. All oil, iron and copper outposts sending 1 Locomotive signal to their train stops, so they will accept only one train. Coal outpost uses combinator logic to calculate the number of trains accepted. You'll need Fusion Power mod by bkrauscs. It can be found in provided Mods folder (**I've changed factorio version to 0.14** because it seems Fusion Power mod is not supported anymore, latest version available here https://mods.factorio.com/mods/bkrauscs/FusionPower and it's for 0.13).

To quickly see the difference GrayMod brings in game i suggest to do the following:

1. Do not use GrayMod, load GMBiggerMap, take a look at rail configuration on map (notice that there are a lot of train stops with same name).
2. Find Oil depot station (you should be spawned right next to it).
3. Turn on the constant combinator located near it.
4. Open Train view and you'll see that **both oil trains are heading to the same Oil station**, but when the first train arrives, the second one has to choose another Oil station which might be **far away in the opposite direction**.
5. Press ESC, run GrayMod, and reload GMBiggerMap.
6. Do steps 2 and 3, and notice that both oil trains started moving, but they are heading to **different train stops**.

Thanks.
