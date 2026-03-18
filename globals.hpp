#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/helpers/memory/Memory.hpp>
#include <hyprland/src/protocols/XDGShell.hpp>

inline HANDLE PHANDLE = nullptr;
inline CFunctionHook* g_pCXDGToplevelResConstructor = nullptr;

typedef void (*origCXDGToplevelResConstructor)(CXDGToplevelResource*, SP<CXdgToplevel>, SP<CXDGSurfaceResource>);
CHyprSignalListener g_pMouseBtnCallback = nullptr;
