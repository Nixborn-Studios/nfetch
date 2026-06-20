#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <cstdlib>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

int main()
{
#ifdef _WIN32

    const char* LocalAppData = std::getenv("LOCALAPPDATA");

    fs::path InstallDir = LocalAppData
        ? fs::path(LocalAppData) / "nfetch"
        : fs::path("C:\\nfetch");

    fs::create_directories(InstallDir);

    fs::copy_file(
        "nfetch.exe",
        InstallDir / "nfetch.exe",
        fs::copy_options::overwrite_existing
    );

    fs::copy(
        "assets",
        InstallDir / "assets",
        fs::copy_options::recursive |
        fs::copy_options::overwrite_existing
    );

    std::string PathCommand =
        "setx PATH \"%PATH%;" + InstallDir.string() + "\"";

    system(PathCommand.c_str());

    std::cout << "Installed to " << InstallDir << std::endl;

#else

    if (geteuid() != 0)
    {
        std::cerr << "Run as root\n";
        return 1;
    }

    fs::path BinaryPath = "/usr/local/bin/nfetch";
    fs::path SharePath = "/usr/share/nfetch";

    if (!fs::exists("nfetch"))
    {
        std::cerr << "Missing file: nfetch\n";
        return 1;
    }

    fs::create_directories(SharePath);

    fs::copy_file(
        "nfetch",
        BinaryPath,
        fs::copy_options::overwrite_existing
    );

    fs::permissions(
        BinaryPath,
        fs::perms::owner_exec |
        fs::perms::group_exec |
        fs::perms::others_exec,
        fs::perm_options::add
    );

    fs::copy(
        "assets",
        SharePath / "assets",
        fs::copy_options::recursive |
        fs::copy_options::overwrite_existing
    );

    std::cout << "Installed to " << BinaryPath << std::endl;

#endif

    return 0;
}