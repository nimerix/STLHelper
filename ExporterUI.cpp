//
// Created by Michael Pearce on 6/19/24.
//

#include "ExporterUI.h"
#include "ExporterPlatform.h"

#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
namespace ac = adsk::core;
namespace af = adsk::fusion;

static const char *const kCommandId{"STLExporterCommandId"};
static const char *const kCommandName{"STL Exporter"};
static const char *const kCommandDescription{"Export selected bodies to STL files."};

static const char *const kPanelName{"UtilityPanel"};
static const char *const kFileDialogTitle{"Select Output Folder"};
static const char *const kDefaultSeparator{"_"};
// Input names
static const char *const kBodiesInput{"SEIBodies"};
static const char *const kOutputFileSuffixInput{"SEIOutputFileSuffix"};
static const char *const kOutputFileOverwriteInput{"SEIOutputFileOverwrite"};
static const char *const kOutputFileSeparatorInput{"SEIOutputFileSeparator"};
static const char *const kIncludeComponentNameInput{"SEIIncludeComponentName"};
static const char *const kOutputFolderInput{"SEIOutputFolder"};
static const char *const kOutputFolderTriggerInput{"SEIOutputFolderTrigger"};
static const char *const kOutputFilePrefixInput{"SEIOutputFilePrefix"};

// Attribute names
static const char *const kAttributeGroup{"STLExporterAttributes"};
static const char *const kAttributeOutputFileSuffix{"SEAOutputFileSuffix"};
static const char *const kAttributeOutputFilePrefix{"SEAOutputFilePrefix"};
static const char *const kAttributeOutputFileSeparator{"SEAOutputFileSeparator"};
static const char *const kAttributeOutputFolder{"SEAOutputFolder"};
static const char *const kAttributeOverwrite{"SEAOverwrite"};
static const char *const kAttributeIncludeComponentName{"SEAIncludeComponentName"};

template<typename T>
ac::Ptr<T> filterOnlyBRepBodies(ac::Ptr<T> selection) {
  return selection && selection->objectType() == af::BRepBody::classType() ? selection : nullptr;
}

class ExporterParameters {

 public:
  fs::path outputFolder{getDownloadsFolder()};
  std::string outputFileSuffix;
  std::string outputFilePrefix;
  std::string outputFileSeparator{kDefaultSeparator};
  std::vector<ac::Ptr<af::BRepBody>> bodies;
  bool overwriteExistingFiles{true};
  bool includeComponentName{true};

  bool Validate() const {
	if (outputFolder.empty() || bodies.empty()) {
	  return false;
	}
	// check folder
	if (!fs::exists(outputFolder)) { // && !fs::is_directory(outputFolder.parent_path())) {
	  fs::path parent = outputFolder;
	  // allow us to create a folder if it doesn't exist
	  while (!fs::exists(parent) && !parent.empty()) { parent = parent.parent_path(); }
	  if (parent.empty()) {
		return false;
	  }
	}
	return true;
  }

  void Clear() {
	outputFolder = getDownloadsFolder();
	outputFileSuffix.clear();
	outputFilePrefix.clear();
	outputFileSeparator = "_";
	bodies.clear();
	overwriteExistingFiles = true;
	includeComponentName = true;
  }

  bool SaveToInputs(ac::Ptr<ac::CommandInputs> inputs) {
	if (!inputs)
	  return false;
	ac::Ptr<ac::SelectionCommandInput> bodiesInput = inputs->itemById(kBodiesInput);
	ac::Ptr<ac::StringValueCommandInput> outputFileSuffixInput = inputs->itemById(kOutputFileSuffixInput);
	ac::Ptr<ac::StringValueCommandInput> outputFilePrefixInput = inputs->itemById(kOutputFilePrefixInput);
	ac::Ptr<ac::StringValueCommandInput> outputFileSeparatorInput = inputs->itemById(kOutputFileSeparatorInput);
	ac::Ptr<ac::TextBoxCommandInput> outputFolderInput = inputs->itemById(kOutputFolderInput);
	ac::Ptr<ac::BoolValueCommandInput> outputOverwriteInput = inputs->itemById(kOutputFileOverwriteInput);
	ac::Ptr<ac::BoolValueCommandInput> includeComponentNameInput = inputs->itemById(kIncludeComponentNameInput);

	if (bodiesInput) {
	  bodiesInput->addSelectionFilter(ac::SelectionFilters::SolidBodies);
	  bodiesInput->clearSelection();
	  for (auto &&b : bodies) {
		bodiesInput->addSelection(b);
	  }
	}
	if (outputFileSuffixInput) {
	  outputFileSuffixInput->value(outputFileSuffix);
	}
	if (outputFilePrefixInput) {
	  outputFilePrefixInput->value(outputFilePrefix);
	}
	if (outputFileSeparatorInput) {
	  outputFileSeparatorInput->value(outputFileSeparator);
	}
	if (outputFolderInput) {
	  outputFolderInput->text(outputFolder.string());
	}
	if (outputOverwriteInput) {
	  outputOverwriteInput->value(overwriteExistingFiles);
	}
	if (includeComponentNameInput) {
	  includeComponentNameInput->value(includeComponentName);
	}
	return true;
  }

  bool LoadFromInputs(ac::Ptr<ac::CommandInputs> inputs) {
	ac::Ptr<ac::SelectionCommandInput> bodiesInput = inputs->itemById(kBodiesInput);
	ac::Ptr<ac::StringValueCommandInput> outputFileSuffixInput = inputs->itemById(kOutputFileSuffixInput);
	ac::Ptr<ac::StringValueCommandInput> outputFilePrefixInput = inputs->itemById(kOutputFilePrefixInput);
	ac::Ptr<ac::StringValueCommandInput> outputFileSeparatorInput = inputs->itemById(kOutputFileSeparatorInput);
	ac::Ptr<ac::TextBoxCommandInput> outputFolderInput = inputs->itemById(kOutputFolderInput);
	ac::Ptr<ac::BoolValueCommandInput> outputOverwriteInput = inputs->itemById(kOutputFileOverwriteInput);
	ac::Ptr<ac::BoolValueCommandInput> includeComponentNameInput = inputs->itemById(kIncludeComponentNameInput);

	if (!bodiesInput || !bodiesInput->isValid() || !outputFolderInput || !outputFolderInput->isValid()) {
	  return false;
	}

	outputFolder = outputFolderInput->text();
	outputFileSuffix = outputFileSuffixInput ? outputFileSuffixInput->value() : outputFileSuffix;
	outputFilePrefix = outputFilePrefixInput ? outputFilePrefixInput->value() : outputFilePrefix;
	overwriteExistingFiles = outputOverwriteInput ? outputOverwriteInput->value() : overwriteExistingFiles;
	includeComponentName = includeComponentNameInput ? includeComponentNameInput->value() : includeComponentName;
	outputFileSeparator = outputFileSeparatorInput ? outputFileSeparatorInput->value() : outputFileSeparator;

	bodies.clear();
	bodies.reserve(bodiesInput->selectionCount());
	for (int i = 0; i < bodiesInput->selectionCount(); ++i) {
	  ac::Ptr<af::BRepBody> body = filterOnlyBRepBodies(bodiesInput->selection(i));
	  if (body) {
		bodies.emplace_back(std::move(body));
	  }
	}
	return true;
  }

  bool SaveToAttributes(ac::Ptr<ac::Attributes> attributes) {
	if (!attributes)
	  return false;
	attributes->add(kAttributeGroup, kAttributeOutputFolder, outputFolder.string());
	attributes->add(kAttributeGroup, kAttributeOutputFileSuffix, outputFileSuffix);
	attributes->add(kAttributeGroup, kAttributeOutputFilePrefix, outputFilePrefix);
	attributes->add(kAttributeGroup, kAttributeOutputFileSeparator, outputFileSeparator);
	attributes->add(kAttributeGroup, kAttributeOverwrite, overwriteExistingFiles ? "true" : "false");
	attributes->add(kAttributeGroup, kAttributeIncludeComponentName, includeComponentName ? "true" : "false");
	return true;
  }

  bool LoadFromAttributes(ac::Ptr<ac::Attributes> attributes) {
	if (!attributes)
	  return false;

	auto outputFileSuffixAttribute = attributes->itemByName(kAttributeGroup, kAttributeOutputFileSuffix);
	if (outputFileSuffixAttribute)
	  outputFileSuffix = outputFileSuffixAttribute->value();

	auto outputFilePrefixAttribute = attributes->itemByName(kAttributeGroup, kAttributeOutputFilePrefix);
	if (outputFilePrefixAttribute)
	  outputFilePrefix = outputFilePrefixAttribute->value();

	auto outputFileSeparatorAttribute = attributes->itemByName(kAttributeGroup, kAttributeOutputFileSeparator);
	if (outputFileSeparatorAttribute)
	  outputFileSeparator = outputFileSeparatorAttribute->value();

	auto outputFolderAttribute = attributes->itemByName(kAttributeGroup, kAttributeOutputFolder);
	if (outputFolderAttribute)
	  outputFolder = outputFolderAttribute->value();

	auto outputOverwriteAttribute = attributes->itemByName(kAttributeGroup, kAttributeOverwrite);
	if (outputOverwriteAttribute)
	  overwriteExistingFiles = outputOverwriteAttribute->value() == "true";

	auto includeComponentNameAttribute = attributes->itemByName(kAttributeGroup, kAttributeIncludeComponentName);
	if (includeComponentNameAttribute)
	  includeComponentName = includeComponentNameAttribute->value() == "true";

	return true;
  }
};

bool BuildElements(ac::Ptr<ac::CommandInputs> inputs) {
  auto params = ExporterParameters();
  if (!inputs)
	return false;

  // Selection
  ac::Ptr<ac::SelectionCommandInput> bodiesInput = inputs->addSelectionInput(kBodiesInput, "Select Bodies", "Select bodies to export");
  if (!bodiesInput)
	return false;

  bodiesInput->addSelectionFilter(ac::SelectionFilters::SolidBodies);
  bodiesInput->setSelectionLimits(1, 0);
  bodiesInput->tooltip("Select bodies to export to STL files");
  bodiesInput->tooltipDescription("Select bodies to export to STL files");

  // Directory
  ac::Ptr<ac::BoolValueCommandInput> directoryButton = inputs->addBoolValueInput(kOutputFolderTriggerInput, "Output Folder", false, "", true);
  if (!directoryButton)
		return false;
  directoryButton->tooltip("Select Output Folder");
  directoryButton->tooltipDescription("Select Output Folder");
  inputs->addTextBoxCommandInput(kOutputFolderInput, "", "", 1, true);

  // File suffix
  auto outputFileSuffix = inputs->addStringValueInput(kOutputFileSuffixInput, "Output File Suffix", "");
  if (!outputFileSuffix)
	return false;
  outputFileSuffix->tooltip("Output File Suffix");
  outputFileSuffix->tooltipDescription("Output File Suffix");

  // File Prefix
  auto outputFilePrefix = inputs->addStringValueInput(kOutputFilePrefixInput, "Output File Prefix", "");
  if (!outputFilePrefix)
	return false;
  outputFilePrefix->tooltip("Output File Prefix");
  outputFilePrefix->tooltipDescription("Output File Prefix");

  // File Separator
  auto outputFileSeparator = inputs->addStringValueInput(kOutputFileSeparatorInput, "Output File Separator", kDefaultSeparator);
  if (!outputFileSeparator)
	return false;
  outputFileSeparator->tooltip("Output File Separator");
  outputFileSeparator->tooltipDescription("Output File Separator");

  // Overwrite
  auto outputOverwrite = inputs->addBoolValueInput(kOutputFileOverwriteInput, "Overwrite Existing Files", true, "", true);
  if (!outputOverwrite)
	return false;
  outputOverwrite->tooltip("Overwrite Existing Files");
  outputOverwrite->tooltipDescription("Overwrite Existing Files");

  // Include Component Name
  auto includeComponentName = inputs->addBoolValueInput(kIncludeComponentNameInput, "Include Component Name", true, "", true);
  if (!includeComponentName)
	return false;
  includeComponentName->tooltip("Include Component Name");
  includeComponentName->tooltipDescription("Include Component Name");

  return true;
}
// Validate Inputs
class OnValidateEventHandler : public ac::ValidateInputsEventHandler {
 public:
  void notify(const ac::Ptr<ac::ValidateInputsEventArgs> &eventArgs) override {
	auto inputs = eventArgs->inputs();
	if (!inputs)
	  return;

	auto firingEvent = eventArgs->firingEvent();
	if (!firingEvent)
	  return;

	ExporterParameters params;
	if (!params.LoadFromInputs(inputs)) {
	  eventArgs->areInputsValid(false);
	  return;
	}
	eventArgs->areInputsValid(params.Validate());
  }
};

class OnActivateEventHandler : public ac::CommandEventHandler {
 public:
  void notify(const ac::Ptr<ac::CommandEventArgs> &eventArgs) override {
	auto app = ac::Application::get();
	if (!app)
	  return;
	auto ui = app->userInterface();
	if (!ui)
	  return;
	auto doc = app->activeDocument();
	if (!doc)
	  return;
	ac::Ptr<af::Design> design = app->activeProduct();
	if (!design) {
	  ui->messageBox("No active design",
					 "Error",
					 ac::MessageBoxButtonTypes::OKButtonType,
					 ac::MessageBoxIconTypes::CriticalIconType);
	  return;
	}
	auto cmd = eventArgs->command();
	if (!cmd)
	  return;
	auto inputs = cmd->commandInputs();
	ExporterParameters params;
	params.LoadFromAttributes(design->attributes());
	params.SaveToInputs(inputs);
  }
};
class OnExecuteEventHandler : public ac::CommandEventHandler {
 public:
  void notify(const ac::Ptr<ac::CommandEventArgs> &eventArgs) override {
	auto command = eventArgs->command();
	if (!command)
	  return;
	auto firing = eventArgs->firingEvent();
	if (!firing)
	  return;
	auto inputs = command->commandInputs();
	if (!inputs)
	  return;

	ExporterParameters params;
	params.LoadFromInputs(inputs);

	auto app = ac::Application::get();
	if (!app)
	  return;
	auto ui = app->userInterface();
	if (!ui)
	  return;
	ac::Ptr<af::Design> design = app->activeProduct();
	if (!design)
	  return;

	if (!params.Validate()) {
	  ui->messageBox("Invalid Inputs",
					 "Error",
					 ac::MessageBoxButtonTypes::OKButtonType,
					 ac::MessageBoxIconTypes::CriticalIconType);
	  return;
	}

	if (!fs::exists(params.outputFolder) && !fs::create_directories(params.outputFolder)) {
	  ui->messageBox("Invalid Output folder: " + params.outputFolder.string(),
					 "Error",
					 ac::MessageBoxButtonTypes::OKButtonType,
					 ac::MessageBoxIconTypes::CriticalIconType);
	  return;
	}

	std::string fileName;
	fileName.reserve(256);
	for (auto &&body : params.bodies) {
	  fs::path filePath = params.outputFolder;
	  fileName.clear();
	  if (!params.outputFilePrefix.empty()) {
		fileName += params.outputFilePrefix;
		fileName += params.outputFileSeparator;
	  }

	  if (params.includeComponentName) {
		auto c = body->parentComponent();
		if (c) {
		  fileName += c->name();
		  fileName += params.outputFileSeparator;
		}
	  }

	  fileName += body->name();

	  if (!params.outputFileSuffix.empty()) {
		fileName += params.outputFileSeparator;
		fileName += params.outputFileSuffix;
	  }
	  fileName += ".stl";
	  filePath /= fileName;
	  if (fs::exists(filePath) && !params.overwriteExistingFiles) {
		ui->messageBox("File already exists: " + filePath.string(),
					   "Error",
					   ac::MessageBoxButtonTypes::OKButtonType,
					   ac::MessageBoxIconTypes::CriticalIconType);
		continue;
	  }

	  auto exportManager = design->exportManager();

	  if (!exportManager) {
		ui->messageBox("Export Manager not available",
					   "Error",
					   ac::MessageBoxButtonTypes::OKButtonType,
					   ac::MessageBoxIconTypes::CriticalIconType);
		return;
	  }
	  auto stlExportOptions = exportManager->createSTLExportOptions(body, filePath.string());
	  stlExportOptions->sendToPrintUtility(false);
	  stlExportOptions->meshRefinement(af::MeshRefinementHigh);
	  if (!exportManager->execute(stlExportOptions)) {
		ui->messageBox("Failed to export: " + filePath.string(),
					   "Error",
					   ac::MessageBoxButtonTypes::OKButtonType,
					   ac::MessageBoxIconTypes::CriticalIconType);
		continue;
	  }
	}
	params.SaveToAttributes(design->attributes());
  }
};

class OnInputChangedEventHandler : public ac::InputChangedEventHandler {
 public:
  void notify(const ac::Ptr<ac::InputChangedEventArgs> &eventArgs) override {
	auto inputs = eventArgs->inputs();
	if (!inputs)
	  return;
	auto input = eventArgs->input();
	if (!input)
	  return;
	auto app = ac::Application::get();
	if (!app)
	  return;
	auto ui = app->userInterface();
	if (!ui)
	  return;
	if (input->id() == kOutputFolderTriggerInput) {
	  ac::Ptr<ac::TextBoxCommandInput> folder = inputs->itemById(kOutputFolderInput);
	  ac::Ptr<ac::FolderDialog> dialog = ui->createFolderDialog();
	  if (!dialog)
		return;
	  dialog->title(kFileDialogTitle);
	  dialog->initialDirectory(folder->text().empty() ? getDownloadsFolder().c_str() : folder->text());
	  ac::DialogResults result = dialog->showDialog();
	  if (result == ac::DialogResults::DialogOK) {
		folder->text(dialog->folder());
	  }
	}
  }
};

class OnCommandCreatedEventHandler : public ac::CommandCreatedEventHandler {
 public:
  void notify(const ac::Ptr<ac::CommandCreatedEventArgs> &eventArgs) override {
	if (!eventArgs)
	  return;
	auto command = eventArgs->command();
	if (!command)
	  return;

	auto onexec = command->execute();
	if (!onexec || !onexec->add(&m_executeHandler))
	  return;

	auto onact = command->activate();
	if (!onact || !onact->add(&m_activateHandler))
	  return;

	auto onval = command->validateInputs();
	if (!onval || !onval->add(&m_validateHandler))
	  return;

	auto onchange = command->inputChanged();
	if (!onchange || !onchange->add(&m_inputChangedHandler))
	  return;

	BuildElements(command->commandInputs());
  }
 private:
  OnExecuteEventHandler m_executeHandler;
  OnValidateEventHandler m_validateHandler;
  OnInputChangedEventHandler m_inputChangedHandler;
  OnActivateEventHandler m_activateHandler;
} commandCreatedHandler;

bool CreatePanel(adsk::core::Ptr<adsk::core::UserInterface> ui) {
  auto ws = ui->workspaces();
  if (!ws)
	return false;

  auto env = ws->itemById("FusionSolidEnvironment");
  if (!env)
	return false;

  if (!env->toolbarPanels())
	return false;


  auto panel = env->toolbarPanels()->itemById(kPanelName);
  if (!panel || !panel->controls())
	return false;

  if (panel->controls()->itemById(kCommandId)) {
	ui->messageBox("Panel already exists",
				   "Warning",
				   ac::MessageBoxButtonTypes::OKButtonType,
				   ac::MessageBoxIconTypes::CriticalIconType);
	adsk::terminate();
	return true;
  }

  auto cmdDefs = ui->commandDefinitions();
  if (!cmdDefs)
	return false;

  auto cmdDef = cmdDefs->itemById(kCommandId);
  if (!cmdDef) {
	// TODO: Icon
	cmdDef = cmdDefs->addButtonDefinition(kCommandId, kCommandName, kCommandDescription);
	if (!cmdDef) {
	  ui->messageBox("Failed to create command definition",
					 "Error",
					 ac::MessageBoxButtonTypes::OKButtonType,
					 ac::MessageBoxIconTypes::CriticalIconType);
	  return false;
	}
  }
  auto commandCreated = cmdDef->commandCreated();
  if (!commandCreated || !commandCreated->add(&commandCreatedHandler))
	return false;

  ac::Ptr<ac::ToolbarControl> panelControl = panel->controls()->addCommand(cmdDef);
  if (!panelControl)
	return false;

  panelControl->isVisible(true);

  return true;
}
bool DestroyPanel(adsk::core::Ptr<adsk::core::UserInterface> ui) {

  if (!ui)
	return true;

  if (ui->commandDefinitions() && ui->commandDefinitions()->itemById(kCommandId)) {
	ui->commandDefinitions()->itemById(kCommandId)->deleteMe();
  }
  if (ui->allToolbarPanels()) {
	auto panel = ui->allToolbarPanels()->itemById(kPanelName);
	if (panel && panel->controls() && panel->controls()->itemById(kCommandId))
	  panel->controls()->itemById(kCommandId)->deleteMe();
  }
  return true;
}
