#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "ExporterUI.h"

adsk::core::Ptr<adsk::core::Application> app;
adsk::core::Ptr<adsk::core::UserInterface> ui;

extern "C" XI_EXPORT bool run(const char* context) {
  app = adsk::core::Application::get();
  if (!app)
	  return false;

  ui = app->userInterface();
  if (!ui)
	  return false;

  CreatePanel(ui);
  return true;
}

extern "C" XI_EXPORT bool stop(const char* context)
{
  if (!ui)
	return true;

  DestroyPanel(ui);
  ui = nullptr;
  app = nullptr;
  return true;
}

#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif // XI_WIN
