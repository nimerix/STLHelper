#pragma once
#include "common.h"
#include "types.h"

fs::path getHomeFolder() {
#ifdef _WIN32
    return fs::path(std::getenv("USERPROFILE"));
#else
    return fs::path(std::getenv("HOME"));
#endif
}

fs::path getDownloadsFolder() {
    return getHomeFolder() / "Downloads";
}

template <typename T>
aPtr<T> getSelection(aPtr<aSel> selection) {
    aPtr<T> res = nullptr;
    if (selection && selection->entity()->objectType() == aBRepBody::classType())
        res = selection->entity();
    return res;
}

bool handleInputs(aPtr<acore::CommandInputs>& inputs, STLHelperInputs& hIn, STLHelperConfig & res, STLHelperError * err = nullptr) {
     hIn = {
            inputs->itemById(kBodiesInput),
            inputs->itemById(kOutputFileSuffixInput),
            inputs->itemById(kOutputFileOverwriteInput),
            inputs->itemById(kOutputFolderTextBoxInput)
    };

    // check for selection
    if (!hIn.bodiesInput || !hIn.bodiesInput->isValid()) {
        if (err) {
            err->message = "Selection input is not valid";
            err->title = "Invalid Input";
            err->isError = true;
            err->isFatal = true;
        }
        return false;
    }

    // check output directory
    if (hIn.outputFolderInput && hIn.outputFolderInput->isValid()) {
        res.outputFolder = hIn.outputFolderInput->text();
    } else {
        res.outputFolder = getDownloadsFolder();
    }
    if (!fs::exists(res.outputFolder)) {
        if (err) {
            err->message = "Output folder ";
            err->message += res.outputFolder.string();
            err->message += " does not exist";
            err->isError = true;
            err->isFatal = true;
        }
        return false;
    }
    if (!hIn.outputFileSuffixInput || hIn.outputFileSuffixInput->value().empty()) {
        res.outputFileSuffix = "";
    } else {
        res.outputFileSuffix = "_";
        res.outputFileSuffix += hIn.outputFileSuffixInput->value();
    }

    // Collect Bodies
    res.bodies.clear();
    res.bodies.reserve(hIn.bodiesInput->selectionCount());
    for (auto i = 0; i < hIn.bodiesInput->selectionCount(); ++i) {
        auto body = getSelection<aBRepBody>(hIn.bodiesInput->selection(i));
        if (!body) {
            if (err) {
                err->message = "Selection ";
                err->message += std::to_string(i);
                err->message += " is not a valid body";
                err->isError = true;
                err->isFatal = true;
            }
            return false;
        }
        res.bodies.emplace_back(std::move(body));
    }
    return true;
}

bool generateFileList(STLHelperConfig const& conf, std::vector<STLHelperFileTaskInfo> & res, STLHelperError * err = nullptr) {
    if (conf.bodies.empty()) {
        if (err) {
            err->message = "No bodies to export";
            err->isError = true;
            err->isFatal = true;
        }
        return false;
    }
    res.clear();
    res.reserve(conf.bodies.size());
    for (auto&& b : conf.bodies) {
        STLHelperFileTaskInfo task;
        task.body = b;
        task.filePath = conf.outputFolder;
        std::string myFileName = "";
        task.component = b->parentComponent();
        if (task.component) {
            myFileName = task.component->name();
            myFileName += "_";
        }
        myFileName += b->name();
        if (!conf.outputFileSuffix.empty())
            myFileName += conf.outputFileSuffix;
        myFileName += ".stl";
        std::replace(myFileName.begin(), myFileName.end(), ' ', '_');
        task.filePath /= myFileName;
        // HACK
        task.overwrite = true;// conf.outputOverwrite;
        res.emplace_back(std::move(task));
    }
    return true;
}

bool exportSTLFiles(std::vector<STLHelperFileTaskInfo> const& tasks, aPtr<aDesign> design, STLHelperError * err = nullptr) {
    for (auto&& t : tasks) {
        // check Design
        if (!design) {
            if (err) {
               err->message = "No active design";
               err->isError = true;
               err->isFatal = true;
            }
            return false;
        }
        // check existing file
        if (fs::exists(t.filePath) && !t.overwrite) {
            if (!err)
                continue;
            err->message = "File ";
            err->message += t.filePath.string();
            err->message += " already exists";
            err->isError = true;
            err->isFatal = true;
            return false;
        }
        // check body pointer
        if (t.body == nullptr) {
            if (!err)
                continue;
            err->message = "Body is does not exist";
            err->isError = true;
            err->isFatal = true;
            return false;
        }

        if (t.component == nullptr) {
            if (!err) continue;
            err->message = "Body does not have a parent component";
            err->isError = true;
            err->isFatal = true;
            return false;
        }
        auto exportMgr = design->exportManager();
        if (!exportMgr) {
            if (err) {
                err->message = "Export manager is not available";
                err->isError = true;
                err->isFatal = true;
            }
            return false;
        }

        aPtr<afusion::STLExportOptions> stlExportOptions = exportMgr->createSTLExportOptions(t.body, t.filePath);
        if (!stlExportOptions) {
            if (err) {
                err->message = "Failed to create STL export options for file ";
                err->message += t.filePath.string();
                err->isError = true;
                err->isFatal = true;
            }
            return false;
        }

        stlExportOptions->sendToPrintUtility(false);
        stlExportOptions->meshRefinement(afusion::MeshRefinementHigh);
        exportMgr->execute(stlExportOptions);
    }
    return true;
}