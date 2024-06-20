//
// Created by Michael Pearce on 6/19/24.
//
#ifndef STLHELPER_EXPORTERUI_H_
#define STLHELPER_EXPORTERUI_H_
#pragma once

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <Cam/CamAll.h>

bool CreatePanel(adsk::core::Ptr<adsk::core::UserInterface> ui);
bool DestroyPanel(adsk::core::Ptr<adsk::core::UserInterface> ui);
#endif //STLHELPER_EXPORTERUI_H_
