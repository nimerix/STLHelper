#pragma once
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <Cam/CamAll.h>
#include <filesystem>

namespace fs = std::filesystem;
namespace acore = adsk::core;
namespace afusion = adsk::fusion;
namespace acam = adsk::cam;

namespace xpt {
    template<typename T>
    using aPtr = acore::Ptr<T>;

    using aSel = acore::Selection;
    using aBRepBody = afusion::BRepBody;
    using aApp = acore::Application;
    using aUI = acore::UserInterface;
    using aDesign = afusion::Design;
    using aComp = afusion::Component;
}

namespace xpt::constants {
    const char *const kButtonName = "STLHelperButton";
	const char *const kCommandName = "STL Export Helper";
	const char *const kCommandDescription = "Export selected bodies to STL files.";
    const char *const kPanelName = "MakePanel";
    const char *const kCommandId = "STLHelperCommandId";
    const char *const kBodiesInput = "BodiesInput";
    const char *const kOutputFileSuffixInput = "OutputFileSuffixInput";
    const char *const kOutputFileOverwriteInput = "OutputFileOverwriteInput";
    const char *const kOutputFolderInput = "OutputFolderInput";
    const char *const kOutputFolderTextBoxInput = "OutputFolderTextBoxInput";
    const char *const kAttributeGroup = "MP-STLHelper";
    const char *const kAttributeSelectedBodies = "SelectedBodies";
    const char *const kAttributeOutputFileSuffix = "OutputFileSuffix";
    const char *const kAttributeOutputFolder = "OutputFolder";
    const char *const kAttributeOverwrite = "Overwrite";
}