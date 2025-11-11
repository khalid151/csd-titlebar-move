#define WLR_USE_UNSTABLE

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>

#include "globals.hpp"

// Methods
void onClick(SCallbackInfo& info, IPointer::SButtonEvent e) {
    if (e.state == WL_POINTER_BUTTON_STATE_RELEASED)
        if (!info.cancelled)
            g_pKeybindManager->m_dispatchers["mouse"]("0movewindow");
}

void hkCXDGToplevelResConstructor(CXDGToplevelResource* thisptr, SP<CXdgToplevel> resource, SP<CXDGSurfaceResource> owner) {
    (*(origCXDGToplevelResConstructor)g_pCXDGToplevelResConstructor->m_original)(thisptr, resource, owner);

    resource->setMove([thisptr](CXdgToplevel* t, wl_resource* seat, uint32_t serial) {
        // treat it as down event, moustButton event is used for up event only
        if (auto window = thisptr->m_window.lock()) {
            g_pCompositor->focusWindow(window);
            g_pKeybindManager->m_dispatchers["mouse"]("1movewindow");
        }
    });
}

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE,
                                     "[csd-titlebar-move] Failure in initialization: Version mismatch "
                                     "(headers ver is not equal to running hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[csd-titlebar-move] Version mismatch");
    }

    auto FNS = HyprlandAPI::findFunctionsByName(PHANDLE, "CXDGToplevelResource");
    for (auto& fn : FNS) {
        if (!fn.demangled.contains("CXDGToplevelResource::CXDGToplevelResource"))
            continue;
        g_pCXDGToplevelResConstructor = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void*)::hkCXDGToplevelResConstructor);
        break;
    }

    g_pMouseBtnCallback = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "mouseButton", [&](void* self, SCallbackInfo& info, std::any param) { onClick(info, std::any_cast<IPointer::SButtonEvent>(param)); });

    bool success = g_pCXDGToplevelResConstructor && g_pMouseBtnCallback;

    if (!success) {
        HyprlandAPI::addNotification(PHANDLE,
                                     "[csd-titlebar-move] Failure in initialization: Failed to find "
                                     "required hook fns",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[csd-titlebar-move] Hooks fn init failed");
    }

    g_pCXDGToplevelResConstructor->hook();

    if (success)
        HyprlandAPI::addNotification(PHANDLE, "[csd-titlebar-move] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);
    else {
        HyprlandAPI::addNotification(PHANDLE, "[csd-titlebar-move] Failure in initialization (hook failed)!", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[csd-titlebar-move] Hooks failed");
    }

    return {"csd-titlebar-move", "A plugin to move CSD GTK applications by their titlebar", "khalid151", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    ;
}
