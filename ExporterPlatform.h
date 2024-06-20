#ifndef STLHELPER__EXPORTER_PLATFORM_H_
#define STLHELPER__EXPORTER_PLATFORM_H_
#pragma once
#include <filesystem>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <combaseapi.h>
#endif

enum class KnownFolders {
  Home,
  Downloads,
  Documents,
  Desktop,
};

#ifdef _WIN32
REFKNOWNFOLDERID getKnownFolderId(KnownFolders folder) {
		switch (folder) {
			case KnownFolders::Home:
				return FOLDERID_Profile;
			case KnownFolders::Downloads:
				return FOLDERID_Downloads;
			case KnownFolders::Documents:
				return FOLDERID_Documents;
			case KnownFolders::Desktop:
				return FOLDERID_Desktop;
			default:
				throw std::runtime_error("Unknown known folder.");
		}
	}
	fs::path getKnownFolderPath(REFKNOWNFOLDERID folderId) {
		PWSTR path = NULL;
		HRESULT result = SHGetKnownFolderPath(folderId, 0, NULL, &path);

		if (SUCCEEDED(result)) {
			fs::path folderPath(path);
			CoTaskMemFree(path);
			return folderPath;
		} else {
			throw std::runtime_error("Unable to determine the folder path.");
		}
	}
#endif

fs::path getKnownFolderPath(KnownFolders folder) {
#ifdef _WIN32
  return getKnownFolderPath(getKnownFolderId(folder));
#else
  fs::path home = std::getenv("HOME");
  switch (folder) {
	case KnownFolders::Home: return home;
	case KnownFolders::Downloads: return home / "Downloads";
	case KnownFolders::Documents: return home / "Documents";
	case KnownFolders::Desktop: return home / "Desktop";
	default: throw std::runtime_error("Unknown known folder.");
  }
#endif
}

fs::path getHomeFolder() {
  return getKnownFolderPath(KnownFolders::Home);
}

fs::path getDownloadsFolder() {
  return getKnownFolderPath(KnownFolders::Downloads);
}

fs::path getDocumentsFolder() {
  return getKnownFolderPath(KnownFolders::Documents);
}

fs::path getDesktopFolder() {
  return getKnownFolderPath(KnownFolders::Desktop);
}

#endif //STLHELPER__EXPORTER_PLATFORM_H_
