#pragma once

namespace sdk {
    enum class SIGNONSTATE {
        NONE = 0,
        CHALLENGE,
        CONNECTED,
        NEW,
        PRESPAWN,
        SPAWN,
        FULL,
        CHANGELEVEL
    };

    // just some useful bone ids
    enum class Bones {
        Head = 8
    };

    struct GlobalVars {
        float flRealTime;
        int iFrameCount;
        float flAbsFrameTime;
        float flAbsFrameStartTime;
        float flCurrentTime;
        float flFrameTime;
        int nMaxClients;
        int iTickCount;
        float flIntervalPerTick;
        float flInterpolationAmount;
        int nFrameSimulationTicks;
        int iNetworkProtocol;
        void *pSaveData;
        bool bClient;
        bool bRemoteClient;
        int iTimestampNetworkingBase;
        int iTimestampRandomizeWindow;
    };
}// namespace sdk