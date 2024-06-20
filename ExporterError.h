#ifndef STLHELPER__EXPORTERERROR_H_
#define STLHELPER__EXPORTERERROR_H_
#pragma once
#include <string>

struct ExporterError {
  std::string message{};
  std::string title{"Error"};
  bool isError{false};
  bool isFatal{false};
};

void PrintMessage(const ExporterError& error);
void PrintMessage(std::string_view message, std::string_view title = "Error", bool isFatal = false);

#endif //STLHELPER__EXPORTERERROR_H_
