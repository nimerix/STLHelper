//
// Created by Michael Pearce on 6/19/24.
//
#include "ExporterTools.h"
#include "ExporterError.h"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <Cam/CamAll.h>

void PrintMessage(const ExporterError &error) {

}
void PrintMessage(std::string_view message, std::string_view title, bool isFatal) {
	auto app = adsk::core::Application::get();
	if (!app)
		return;
	auto ui = app->userInterface();
	if (!ui)
		return;


	ui->messageBox(message.data(), title.data(), adsk::core::MessageBoxButtonTypes::OKButtonType,
				   isFatal ? adsk::core::MessageBoxIconTypes::CriticalIconType : adsk::core::MessageBoxIconTypes::InformationIconType);

}
