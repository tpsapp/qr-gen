#include "qr_renderer.hpp"
#include "tinypngout.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace qrgen {

int savePng(const qrcodegen::QrCode &qr,
            int scale,
            const char *filename,
            int border)
{
    if (scale < 1 || scale > 100)
    {
        std::cerr << "Error: Scale must be an integer between 1 and 100.\n";
        return 1;
    }
    if (border < 0)
    {
        std::cerr << "Error: Border must be non-negative.\n";
        return 1;
    }
    if (filename == nullptr)
    {
        std::cerr << "Error: Output filename cannot be null.\n";
        return 1;
    }

    const int size = qr.getSize();                  // Size of QR code in modules
    const int modulesCount = size + border * 2;     // Total modules across width/height
    const int pixels = modulesCount * scale;        // Total image size in pixels

    std::ofstream ofs(filename, std::ios::binary); // Open output file
    if (!ofs)
    {
        std::cerr << "Failed to open output file " << filename << "\n";
        return 1;
    }

    try
    {
        TinyPngOut png(static_cast<std::uint32_t>(pixels), static_cast<std::uint32_t>(pixels), ofs);

        // Pre-allocate single RGB scanline buffer for IMP-01 row caching
        std::vector<std::uint8_t> scanline(static_cast<size_t>(pixels) * 3u);

        // Iterate through each horizontal QR module row
        for (int my = -border; my < size + border; ++my)
        {
            // Rasterize one full module row into the scanline buffer
            for (int mx = -border; mx < size + border; ++mx)
            {
                bool dark = false;
                if (mx >= 0 && mx < size && my >= 0 && my < size)
                {
                    dark = qr.getModule(mx, my);
                }

                std::uint8_t v = dark ? 0u : 255u;
                size_t pxBase = static_cast<size_t>(mx + border) * static_cast<size_t>(scale);

                for (int s = 0; s < scale; ++s)
                {
                    size_t off = (pxBase + static_cast<size_t>(s)) * 3u;
                    scanline[off + 0] = v;
                    scanline[off + 1] = v;
                    scanline[off + 2] = v;
                }
            }

            // Stream the pre-computed scanline vertically 'scale' times
            for (int s = 0; s < scale; ++s)
            {
                png.write(scanline.data(), static_cast<size_t>(pixels));
            }
        }

        ofs.flush();
        if (!ofs.good())
        {
            std::cerr << "Error writing PNG: Disk full or I/O failure while writing " << filename << "\n";
            return 1;
        }
        ofs.close();
        if (!ofs.good())
        {
            std::cerr << "Error writing PNG: I/O failure while closing " << filename << "\n";
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing PNG: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

} // namespace qrgen
