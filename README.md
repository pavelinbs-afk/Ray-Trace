# Ray-Trace

**Shared ray tracing interface for Metamod:Source & CounterStrikeSharp
plugins**

------------------------------------------------------------------------

## Overview

`Ray-Trace` is a lightweight **Metamod interface module** for\
**Counter-Strike 2** servers (C++ / MM 2.0 + KHook).

It exposes a shared interface: `CRayTraceInterface002` for native
Metamod plugins. The CounterStrikeSharp (C#) API and plugin live in a
separate tree: `D:/projects/csharp/RayTrace` (RayTraceApi + RayTraceImpl).

The goal is to provide a **single tracing backend** without duplicating
engine detours.

------------------------------------------------------------------------

## Features

-   Metamod meta interface (`CRayTraceInterface002`)
-   Works in **C++ and C#**
-   Physics / hitbox / world trace presets
-   Custom collision masks
-   Optional debug beam rendering
-   Zero configuration
-   Ultra low overhead
-   ABI-safe (no STL types in interface)
-   Virtual destructor for safe lifetime management

------------------------------------------------------------------------

## Exposed Interface (C++)

``` cpp
class CRayTraceInterface
{
public:
    virtual ~CRayTraceInterface() = default;

    virtual bool TraceShape(
        const Vector* pOrigin,
        const QAngle* pViewAngles,
        CBaseEntity* pIgnoreEntity,
        const TraceOptions* pOpts,
        TraceResult* pOutResult
    ) = 0;

    virtual bool TraceEndShape(
        const Vector* pOrigin,
        const Vector* pEndOrigin,
        CBaseEntity* pIgnoreEntity,
        const TraceOptions* pOpts,
        TraceResult* pOutResult
    ) = 0;

    virtual bool TraceHullShape(
        const Vector* pVecStart,
        const Vector* pVecEnd,
        const Vector* pHullMins,
        const Vector* pHullMaxs,
        CBaseEntity* pIgnoreEntity,
        const TraceOptions* pOpts,
        TraceResult* pOutResult
    ) = 0;

    virtual bool TraceShapeEx(
        const Vector* pVecStart,
        const Vector* pVecEnd,
        CTraceFilter* pFilterInc,
        Ray_t* rayInc,
        TraceResult* pOutResult
    ) = 0;
};
```

**Return value:**

-   `true` → trace hit something, `TraceResult` is valid\
-   `false` → no hit

------------------------------------------------------------------------

## Getting the interface

### C++ (Metamod plugin)

Include header from `public/craytraceinterface.h`

``` cpp
CRayTraceInterface* g_pRayTrace = nullptr;
bool g_bRayTraceLoaded = false;

bool LoadRayTrace()
{
    int iRet = 0;

    g_pRayTrace = static_cast<CRayTraceInterface*>(
        g_SMAPI->MetaFactory(RAYTRACE_INTERFACE_VERSION, &iRet, nullptr)
    );

    if (iRet == META_IFACE_FAILED || !g_pRayTrace)
    {
        FP_ERROR("Failed to lookup Ray-Trace interface!");
        return false;
    }

    g_bRayTraceLoaded = true;
    return true;
}
```

### C# (CounterStrikeSharp)

See **`D:/projects/csharp/RayTrace`** — build/install RayTraceImpl + RayTraceApi from there.

------------------------------------------------------------------------

## Calling methods from C++ (Metamod)

### TraceShape example

``` cpp
Vector vecOrigin{};
QAngle angView{};
TraceOptions traceOpts{};
traceOpts.InteractsWith = static_cast<uint64_t>(MASK_SHOT_PHYSICS);
traceOpts.DrawBeam = 1;

TraceResult traceResult{};

if (g_pRayTrace && g_bRayTraceLoaded)
{
    bool bHit = g_pRayTrace->TraceShape(
        &vecOrigin,
        &angView,
        nullptr,
        &traceOpts,
        &traceResult
    );

    if (bHit)
    {
        Msg("Hit fraction: %f\n", traceResult.Fraction);
        Msg("End pos: %f %f %f\n",
            traceResult.EndPos.x,
            traceResult.EndPos.y,
            traceResult.EndPos.z);
    }
}
```

------------------------------------------------------------------------

## Notes about ABI & Destructor

-   `CRayTraceInterface` has a virtual destructor.
-   The object is owned by the Ray-Trace Metamod module.
-   Plugins must never call `delete` on the interface pointer.
-   C# consumers: see `D:/projects/csharp/RayTrace` (handle only; never free native object).
-   All parameters are passed as native pointers.

------------------------------------------------------------------------

## License

GPLv3\
https://www.gnu.org/licenses/gpl-3.0.en.html

------------------------------------------------------------------------

## Author

**Michal "Slynx" Přikryl**\
https://slynxdev.cz
