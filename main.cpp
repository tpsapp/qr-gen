#include <cstring>
#include <fstream>
#include <cstdint>
#include <vector>
#include <iostream>
#include "qrcodegen.hpp"
#include "tinypngout.hpp"

void printUsage(const char *progName)
{
    fprintf(stderr,
            "Usage: %s <text to encode> [OPTIONS]\n\n"
            "Options:\n"
            "  -o, --output <file>   Output PNG file (default: qrcode.png)\n"
            "  -s, --scale <n>       Scale (pixels per module, default: 10)\n"
            "  -h, --help            Show this help and exit\n",
            progName);
}

int savePng(const qrcodegen::QrCode &qr, int scale, const char *filename)
{
    const int border = 4;                           // Border size in modules
    const int size = qr.getSize();                  // Size of QR code in modules
    const int pixels = (size + border * 2) * scale; // Total image size in pixels

    std::ofstream ofs(filename, std::ios::binary); // Open output file
    if (!ofs)
    {
        fprintf(stderr, "Failed to open output file %s\n", filename);
        return 1;
    }

    try
    {
        TinyPngOut png(static_cast<std::uint32_t>(pixels), static_cast<std::uint32_t>(pixels), ofs); // Create PNG writer

        std::vector<std::uint8_t> row(static_cast<size_t>(pixels) * 3); // RGB row buffer

        for (int py = 0; py < pixels; ++py)
        {
            for (int px = 0; px < pixels; ++px)
            {
                // Map output pixel to module coordinates
                int mx = px / scale - border; // Module x coordinate
                int my = py / scale - border; // Module y coordinate
                bool dark = false;            // Is the module dark?
                if (mx >= 0 && mx < size && my >= 0 && my < size)
                    dark = qr.getModule(mx, my); // Query module color

                std::uint8_t v = dark ? 0u : 255u;         // Pixel value (black or white)
                size_t off = static_cast<size_t>(px) * 3u; // Offset in row buffer
                row[off + 0] = v;                          // R
                row[off + 1] = v;                          // G
                row[off + 2] = v;                          // B
            }

            // TinyPngOut::write expects number of pixels (not bytes)
            png.write(row.data(), static_cast<size_t>(pixels));
        }
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "Error writing PNG: %s\n", e.what());
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    // Parse command line arguments
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
    {
        printUsage(argv[0]);
        return 0;
    }

    const char *text = argv[1];         // Text to encode
    const char *outFile = "qrcode.png"; // Default output file
    int scale = 10;                     // Default scale

    for (int i = 2; i < argc; ++i)
    {
        // Parse options
        if ((std::strcmp(argv[i], "-o") == 0 || std::strcmp(argv[i], "--output") == 0) && i + 1 < argc)
        {
            outFile = argv[++i]; // Output file name
        }
        else if ((std::strcmp(argv[i], "-s") == 0 || std::strcmp(argv[i], "--scale") == 0) && i + 1 < argc)
        {
            scale = std::atoi(argv[++i]); // Scale factor
            if (scale <= 0)
                scale = 10; // Reset to default if invalid
        }
        else
        {
            printUsage(argv[0]);
            return 1;
        }
    }

    try
    {
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text, qrcodegen::QrCode::Ecc::LOW); // Generate QR code
        int ret = savePng(qr, scale, outFile);                                                   // Save as PNG
        if (ret != 0)
        {
            fprintf(stderr, "Failed to save PNG\n");
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }

    return 0;
}