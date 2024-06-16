
#include "Exporter/common.h"
#include "Exporter/types.h"
#include "utility.h"


aPtr<aApp> app;
aPtr<aUI> ui;

STLHelperConfig conf;
STLHelperError err;
STLHelperInputs ginputs;
std::vector<STLHelperFileTaskInfo> tasks;

inline void printError(const STLHelperError & err) {
    ui->messageBox(err.message.c_str(), err.title, acore::OKButtonType, err.isFatal ? acore::CriticalIconType : acore::WarningIconType);
}

class OnExecuteEventHandler : public acore::CommandEventHandler {
public:
            void notify(const acore::Ptr<acore::CommandEventArgs>& eventArgs) override {
                auto cmd = eventArgs->command();
                if (!cmd)
                    return;

                aPtr<aDesign> design = app->activeProduct();
                if (!design) {
                    ui->messageBox("No active design", "No Design");
                    return;
                }

                auto inputs = cmd->commandInputs();

                if (!handleInputs(inputs, ginputs, conf, &err)) {
                    printError(err);
                    return;
                }

                if (!generateFileList(conf, tasks, &err)) {
                    printError(err);
                    return;
                }

                if (!exportSTLFiles(tasks, design, &err)) {
                    printError(err);
                    return;
                }

            }
};

// input validation
class OnValidateEventHandler : public acore::ValidateInputsEventHandler {
public:
    void notify(const aPtr<acore::ValidateInputsEventArgs>& eventArgs) override {
        auto inputs = eventArgs->inputs();
        if (!inputs)
            return;
        STLHelperConfig c;
        STLHelperInputs ins;
        if (!handleInputs(inputs, ins, c, nullptr)) {
            eventArgs->areInputsValid(false);
            return;
        }
        eventArgs->areInputsValid(true);
}
};

// Input Change
class OnInputChangedEventHandler : public acore::InputChangedEventHandler {
public:
    void notify(const aPtr<acore::InputChangedEventArgs>& eventArgs) override {
        auto inputs = eventArgs->inputs();
        if (!inputs)
            return;
        auto input = eventArgs->input();
        if (input->id() == kOutputFolderInput) {
            aPtr<acore::BoolValueCommandInput> button = input;
            aPtr<acore::TextBoxCommandInput> folder = inputs->itemById(kOutputFolderTextBoxInput);
            aPtr<acore::FolderDialog> folderDialog = ui->createFolderDialog();
            if (!folderDialog)
                return;
            folderDialog->initialDirectory(getDownloadsFolder().string().c_str());
            folderDialog->title("Select Output Folder");
            acore::DialogResults dialogResult = folderDialog->showDialog();
            if (dialogResult == acore::DialogResults::DialogOK) {
                folder->text(folderDialog->folder());
            }
        }
    }
};

// Activate Handler
class OnActivateEventHandler : public acore::CommandEventHandler {
public:
    void notify(const aPtr<acore::CommandEventArgs>& eventArgs) override {
        auto doc = app->activeDocument();
        if (!doc)
            return;

        aPtr<aDesign> design = app->activeProduct();
        if (!design) {
            ui->messageBox("No active design", "No Design");
            return;
        }

        aPtr<acore::Command> cmd = eventArgs->command();

        if (!cmd)
            return;

        auto inputs = cmd->commandInputs();
        ginputs = {
            inputs->itemById(kBodiesInput),
                    inputs->itemById(kOutputFileSuffixInput),
                    inputs->itemById(kOutputFileOverwriteInput),
                    inputs->itemById(kOutputFolderInput)
        };
        std::vector<aPtr<acore::Attribute>> selectedBodies = design->findAttributes(kAttributeGroup, kAttributeSelectedBodies);
        for (auto&& a : selectedBodies) {
            if (a->parent() != nullptr)
                a->deleteMe();
        }

        aPtr<acore::Attribute> outputFileSuffixAttribute = design->attributes()->itemByName(kAttributeGroup, kAttributeOutputFileSuffix);
        if (outputFileSuffixAttribute != nullptr) {
            ginputs.outputFileSuffixInput->value(outputFileSuffixAttribute->value());
        }

        aPtr<acore::Attribute> outputFolderAttribute = design->attributes()->itemByName(kAttributeGroup, kAttributeOutputFolder);
        if (outputFolderAttribute != nullptr) {
            ginputs.outputFolderInput->text(outputFolderAttribute->value());
        }

        aPtr<acore::Attribute> overwriteAttribute = design->attributes()->itemByName(kAttributeGroup, kAttributeOverwrite);
        if (overwriteAttribute != nullptr) {
            ginputs.outputOverwriteInput->value(true);
        }


    }
};

// Destroy
class OnDestroyEventHandler : public acore::CommandEventHandler {
public:
    void notify(const aPtr<acore::CommandEventArgs>& eventArgs) override {
    }
};

// Created
class CommandCreatedEventHander : public acore::CommandCreatedEventHandler {
public:
    void notify(const aPtr<acore::CommandCreatedEventArgs>& eventArgs) override {
        if (!eventArgs)
            return;

        auto command = eventArgs->command();
        if (!command)
            return;

        auto onDestroy = command->destroy();
        if (!onDestroy)
            return;
        bool ok = onDestroy->add(&onDestroyHandler);
        if (!ok)
            return;

        auto onActivate = command->activate();
        if (!onActivate)
            return;
        ok = onActivate->add(&onActivateHandler);
        if (!ok)
            return;

        auto onExecute = command->execute();
        if (!onExecute)
            return;
        ok = onExecute->add(&onExecuteHandler);
        if (!ok)
            return;

        auto onValidate = command->validateInputs();
        if (!onValidate)
            return;
        ok = onValidate->add(&onValidateHandler);
        if (!ok)
            return;

        auto onInputChanged = command->inputChanged();
        if (!onInputChanged)
            return;
        ok = onInputChanged->add(&onInputChangedHandler);
        if (!ok)
            return;

        auto inputs = command->commandInputs();
        if (!inputs)
            return;

        // body selection
        aPtr<acore::SelectionCommandInput> bodySelectionInput = inputs->addSelectionInput(kBodiesInput, "Select Bodies", "Select bodies to export to STL");
        if (!bodySelectionInput)
            return;
        bodySelectionInput->addSelectionFilter("SolidBodies"); // only allow selection of solid bodies
        bodySelectionInput->setSelectionLimits(1, 0);
        bodySelectionInput->tooltip("Select bodies to export to STL");
        bodySelectionInput->tooltipDescription("Selected bodies must be solid.");

        // Output directory
        aPtr<acore::TextBoxCommandInput> outputFolderInput = inputs->addTextBoxCommandInput(kOutputFolderInput, "Output Folder", getDownloadsFolder().string().c_str(), 1, true);
        if (!outputFolderInput)
            return;
        outputFolderInput->tooltip("Output Folder");
        outputFolderInput->tooltipDescription("Select the output folder for the STL files");
        aPtr<acore::BoolValueCommandInput> dirbtn = inputs->addBoolValueInput(kOutputFolderInput, "Output Folder", false, "", true);
        inputs->addTextBoxCommandInput(kOutputFolderTextBoxInput, "", "", 1, true);

        // File suffix
        aPtr<acore::StringValueCommandInput> outputFileSuffixInput = inputs->addStringValueInput(kOutputFileSuffixInput, "File Suffix", "");
        if (!outputFileSuffixInput)
            return;
        outputFileSuffixInput->tooltip("File Suffix");
        outputFileSuffixInput->tooltipDescription("Suffix to append to the output file name. Note that a leading underscore will be added.");

        // overwrite
        aPtr<acore::BoolValueCommandInput> outputOverwriteInput = inputs->addBoolValueInput(kOutputFileOverwriteInput, "Overwrite Existing Files", false, "", true);
        if (!outputOverwriteInput)
            return;
        outputOverwriteInput->text("Overwrite Existing Files");
        outputOverwriteInput->tooltip("Overwrite Existing Files");
        outputOverwriteInput->tooltipDescription("If checked, existing files will be overwritten.");
    }
private:
    OnExecuteEventHandler onExecuteHandler;
    OnValidateEventHandler onValidateHandler;
    OnInputChangedEventHandler onInputChangedHandler;
    OnActivateEventHandler onActivateHandler;
    OnDestroyEventHandler onDestroyHandler;
} _cmdCreatedHandler;
extern "C" XI_EXPORT bool run(const char* context)
{
    app = adsk::core::Application::get();
    if (!app)
        return false;

    ui = app->userInterface();
    if (!ui)
        return false;

    auto cmdDefs = ui->commandDefinitions();
    auto cmdDef = cmdDefs->addButtonDefinition(kCommandId, "Export STL", "Export selected bodies to STL files", "");

    auto addinsPanel = ui->allToolbarPanels()->itemById(kPanelName);
    auto ctrl = addinsPanel->controls()->itemById(kCommandId);
    if (!ctrl) {
        addinsPanel->controls()->addCommand(cmdDef);
    }
    aPtr<acore::CommandCreatedEvent> commandCreatedEvent = cmdDef->commandCreated();
    commandCreatedEvent->add(&_cmdCreatedHandler);
    return true;
}

extern "C" XI_EXPORT bool stop(const char* context)
{
    if (ui)
    {
        auto commandDefs = ui->commandDefinitions();
        if (!commandDefs) {
            ui = nullptr;
            return false;
        }

        auto cmdDef = commandDefs->itemById(kCommandId);
        if (cmdDef) {
            cmdDef->deleteMe();
        }
        else {
            ui->messageBox("Cannot find command definition!", "Error");
        }

        auto addinsPanel = ui->allToolbarPanels()->itemById(kPanelName);
        auto ctrl = addinsPanel->controls()->itemById(kCommandId);
        if (ctrl) {
            ctrl->deleteMe();
        }
        ui = nullptr;
    }

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
