#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include "qrcodegen.hpp"
#include "qr_renderer.hpp"

void printUsage(const char *progName)
{
    std::cerr << "Usage: " << progName << " <text to encode> [OPTIONS]\n\n"
              << "Options:\n"
              << "  -o, --output <file>   Output PNG file (default: qrcode.png)\n"
              << "  -s, --scale <n>       Scale (pixels per module, default: 10, range: 1-100)\n"
              << "  -h, --help            Show this help and exit\n";
}

int main(int argc, char **argv)
{
    // Parse command line arguments
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    const char *text = nullptr;
    const char *outFile = "qrcode.png";
    int scale = 10;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: Missing argument for option '" << arg << "'\n";
                return 1;
            }
            outFile = argv[++i];
        }
        else if (arg == "-s" || arg == "--scale")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: Missing argument for option '" << arg << "'\n";
                return 1;
            }
            const char *valStr = argv[++i];
            char *endptr = nullptr;
            errno = 0;
            long val = std::strtol(valStr, &endptr, 10);
            if (errno != 0 || endptr == valStr || *endptr != '\0' || val < 1 || val > 100)
            {
                std::cerr << "Error: Invalid scale '" << valStr << "'. Scale must be an integer between 1 and 100.\n";
                return 1;
            }
            scale = static_cast<int>(val);
        }
        else if (!arg.empty() && arg[0] == '-')
        {
            std::cerr << "Error: Unrecognized option '" << arg << "'\n";
            printUsage(argv[0]);
            return 1;
        }
        else
        {
            if (text != nullptr)
            {
                std::cerr << "Error: Unexpected argument '" << arg << "'\n";
                printUsage(argv[0]);
                return 1;
            }
            text = argv[i];
        }
    }

    if (text == nullptr)
    {
        printUsage(argv[0]);
        return 1;
    }

    try
    {
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text, qrcodegen::QrCode::Ecc::LOW); // Generate QR code
        int ret = qrgen::savePng(qr, scale, outFile);                                            // Save as PNG
        if (ret != 0)
        {
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}