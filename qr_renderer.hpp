#pragma once

#include "qrcodegen.hpp"

namespace qrgen {

/*
 * Renders the provided QR code to an uncompressed 24-bit RGB PNG file.
 *
 * Parameters:
 *   qr:       The qrcodegen::QrCode instance to render.
 *   scale:    Integer scaling factor (pixels per QR module). Must be in [1, 100].
 *   filename: Output path of the destination PNG file.
 *   border:   Quiet-zone border size in QR modules (default: 4). Must be >= 0.
 *
 * Returns:
 *   0 on success, or 1 if an error occurred (e.g., invalid scale/border,
 *   filesystem permission error, disk full / broken pipe).
 */
int savePng(const qrcodegen::QrCode &qr,
            int scale,
            const char *filename,
            int border = 4);

} // namespace qrgen
