//
// Created by Michal Přikryl on 10.07.2025.
// Copyright (c) 2025 slynxcz. All rights reserved.
//
#include "listeners.h"
#include <raytrace.h>
#include <shared.h>
#include <dynlibutils/module.h>
#include <entitysystem.h>
#include "log.h"

namespace RayTracePlugin::Listeners
{
    Hooks g_Hooks;

    template <typename CLASS, typename RETURN, typename... ARGS>
    static void AddGlobalByVtbl(KHook::Virtual<CLASS, RETURN, ARGS...>& hook, void* vtbl)
    {
        struct
        {
            void* v;
        } fake{vtbl};
        hook.AddGlobal(reinterpret_cast<CLASS*>(&fake));
    }

    template <typename CLASS, typename RETURN, typename... ARGS>
    static void RemoveGlobalByVtbl(KHook::Virtual<CLASS, RETURN, ARGS...>& hook, void* vtbl)
    {
        if (!vtbl)
            return;
        struct
        {
            void* v;
        } fake{vtbl};
        hook.RemoveGlobal(reinterpret_cast<CLASS*>(&fake));
    }

    Hooks::Hooks()
        : m_StartupServer(&INetworkServerService::StartupServer, this, nullptr, &Hooks::Hook_StartupServer),
          m_LoadEventsFromFile(&IGameEventManager2::LoadEventsFromFile, this, &Hooks::Hook_LoadEventsFromFile, nullptr)
    {
    }

    bool InitListeners()
    {
        return g_Hooks.Init();
    }

    void DestructListeners()
    {
        g_Hooks.Destruct();
    }

    bool Hooks::Init()
    {
        m_StartupServer.Add(shared::g_pNetworkServerService);

        m_pEventMgrVtbl =
            DynLibUtils::CModule(shared::g_pServer).GetVirtualTableByName("CGameEventManager").RCast<void*>();
        if (!m_pEventMgrVtbl)
        {
            FP_ERROR("Failed to locate CGameEventManager vtable");
            return false;
        }

        AddGlobalByVtbl(m_LoadEventsFromFile, m_pEventMgrVtbl);
        return true;
    }

    void Hooks::Destruct()
    {
        m_StartupServer.Remove(shared::g_pNetworkServerService);
        RemoveGlobalByVtbl(m_LoadEventsFromFile, m_pEventMgrVtbl);
        m_pEventMgrVtbl = nullptr;
    }

    KHook::Return<void> Hooks::Hook_StartupServer(INetworkServerService*, const GameSessionConfiguration_t&,
                                                  ISource2WorldSession*, const char*)
    {
        if (!shared::g_bDetoursLoaded)
        {
            shared::g_pEntitySystem = GameEntitySystem();
            RayTrace::g_CRayTrace.Initialize();
            shared::g_bDetoursLoaded = true;
        }

        return {KHook::Action::Ignore};
    }

    KHook::Return<int> Hooks::Hook_LoadEventsFromFile(IGameEventManager2* pThis, const char*, bool)
    {
        if (!shared::g_pGameEventManager && pThis)
            shared::g_pGameEventManager = pThis;

        return {KHook::Action::Ignore};
    }
}
