#pragma once
#include <nfd.h>
#include <optional>
#include <filesystem>
namespace filedialog {
auto open() -> std::optional<std::filesystem::path>
{
    nfdchar_t *outPath = nullptr;
    nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

    if (result == NFD_OKAY) {
        return std::filesystem::path(outPath);
    }
    return {};
}
} // namespace filedialog
