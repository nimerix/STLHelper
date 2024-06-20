#pragma once

namespace xpt {
    struct STLHelperFileTaskInfo {
        fs::path filePath;
        aPtr<aBRepBody> body;
        aPtr<aComp> component;
        bool overwrite{false};
    };

    struct STLHelperConfig {
        fs::path outputFolder;
        std::string outputFileSuffix;
        std::vector<aPtr<aBRepBody>> bodies;
        bool outputOverwrite;
    };

    struct STLHelperError {
        std::string message{};
        std::string title{"Error"};
        bool isError{false};
        bool isFatal{false};
    };

    struct STLHelperInputs {
        aPtr<acore::SelectionCommandInput> bodiesInput;
        aPtr<acore::StringValueCommandInput> outputFileSuffixInput;
        aPtr<acore::BoolValueCommandInput> outputOverwriteInput;
        aPtr<acore::TextBoxCommandInput> outputFolderInput;
    };
}