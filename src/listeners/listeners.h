//
// Created by Michal Přikryl on 09.07.2025.
// Copyright (c) 2025 slynxcz. All rights reserved.
//
#pragma once
#include <eiface.h>
#include <iserver.h>
#include <igameevents.h>
#include <ISmmPlugin.h>

// Engine only passes this by reference; KHook needs a complete type for sizeof.
class GameSessionConfiguration_t
{
};

namespace RayTracePlugin::Listeners
{
    bool InitListeners();
    void DestructListeners();

    class Hooks
    {
    public:
        Hooks();

        KHook::Return<void> Hook_StartupServer(INetworkServerService* pThis, const GameSessionConfiguration_t& config,
                                               ISource2WorldSession* pWorldSession, const char* pszMapName);
        KHook::Return<int> Hook_LoadEventsFromFile(IGameEventManager2* pThis, const char* filename, bool bSearchAll);

        bool Init();
        void Destruct();

    private:
        KHook::Virtual<INetworkServerService, void, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*>
            m_StartupServer;
        KHook::Virtual<IGameEventManager2, int, const char*, bool> m_LoadEventsFromFile;
        void* m_pEventMgrVtbl = nullptr;
    };

    extern Hooks g_Hooks;
}
