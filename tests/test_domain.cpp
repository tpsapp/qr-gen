#include "TestHarness.hpp"
#include "qr_renderer.hpp"
#include "qrcodegen.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {

// Helper: Read binary file bytes
std::vector<std::uint8_t> readFile(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        return {};
    }
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(ifs), {});
}

// Helper: Extract width & height from PNG IHDR chunk
bool getPngDimensions(const std::string &path, std::uint32_t &width, std::uint32_t &height)
{
    auto data = readFile(path);
    if (data.size() < 24)
    {
        return false;
    }

    // Verify 8-byte PNG signature
    const std::uint8_t signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    for (size_t i = 0; i < 8; ++i)
    {
        if (data[i] != signature[i])
        {
            return false;
        }
    }

    // IHDR chunk type check: bytes 12..15 must be "IHDR"
    if (data[12] != 'I' || data[13] != 'H' || data[14] != 'D' || data[15] != 'R')
    {
        return false;
    }

    // Big-endian width: bytes 16..19
    width = (static_cast<std::uint32_t>(data[16]) << 24u) |
            (static_cast<std::uint32_t>(data[17]) << 16u) |
            (static_cast<std::uint32_t>(data[18]) << 8u) |
            static_cast<std::uint32_t>(data[19]);

    // Big-endian height: bytes 20..23
    height = (static_cast<std::uint32_t>(data[20]) << 24u) |
             (static_cast<std::uint32_t>(data[21]) << 16u) |
             (static_cast<std::uint32_t>(data[22]) << 8u) |
             static_cast<std::uint32_t>(data[23]);

    return true;
}

const char *kTestTmpFile = "/tmp/qr_test_output.png";

void cleanupTmp()
{
    (void)std::remove(kTestTmpFile);
}

} // namespace

// TC_DOM_01: Happy path Version 1 QR code rendering
TEST_CASE(TC_DOM_01_HappyPathVersion1)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("Hello", qrcodegen::QrCode::Ecc::LOW);
    ASSERT_EQ(qr.getSize(), 21);

    int ret = qrgen::savePng(qr, 10, kTestTmpFile, 4);
    ASSERT_EQ(ret, 0);

    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, 290u); // (21 + 4*2) * 10
    ASSERT_EQ(h, 290u);
    cleanupTmp();
}

// TC_DOM_02: Minimum scale boundary (scale = 1)
TEST_CASE(TC_DOM_02_MinScaleBoundary)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("A", qrcodegen::QrCode::Ecc::LOW);
    ASSERT_EQ(qr.getSize(), 21);

    int ret = qrgen::savePng(qr, 1, kTestTmpFile, 4);
    ASSERT_EQ(ret, 0);

    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, 29u); // (21 + 8) * 1
    ASSERT_EQ(h, 29u);
    cleanupTmp();
}

// TC_DOM_03: Maximum scale boundary (scale = 100)
TEST_CASE(TC_DOM_03_MaxScaleBoundary)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("B", qrcodegen::QrCode::Ecc::LOW);

    int ret = qrgen::savePng(qr, 100, kTestTmpFile, 4);
    ASSERT_EQ(ret, 0);

    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, 2900u); // (21 + 8) * 100
    ASSERT_EQ(h, 2900u);
    cleanupTmp();
}

// TC_DOM_04: Quiet-zone border zero (border = 0)
TEST_CASE(TC_DOM_04_QuietZoneBorderZero)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("BorderZero", qrcodegen::QrCode::Ecc::LOW);

    int ret = qrgen::savePng(qr, 10, kTestTmpFile, 0);
    ASSERT_EQ(ret, 0);

    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, 210u); // 21 * 10
    ASSERT_EQ(h, 210u);
    cleanupTmp();
}

// TC_DOM_05: Large quiet-zone border (border = 8)
TEST_CASE(TC_DOM_05_QuietZoneBorderEight)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("BorderEight", qrcodegen::QrCode::Ecc::LOW);

    int ret = qrgen::savePng(qr, 5, kTestTmpFile, 8);
    ASSERT_EQ(ret, 0);

    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, 185u); // (21 + 16) * 5
    ASSERT_EQ(h, 185u);
    cleanupTmp();
}

// TC_DOM_06: All ECC levels (LOW, MEDIUM, QUARTILE, HIGH)
TEST_CASE(TC_DOM_06_AllEccLevels)
{
    cleanupTmp();
    const qrcodegen::QrCode::Ecc eccLevels[] = {
        qrcodegen::QrCode::Ecc::LOW,
        qrcodegen::QrCode::Ecc::MEDIUM,
        qrcodegen::QrCode::Ecc::QUARTILE,
        qrcodegen::QrCode::Ecc::HIGH};

    for (const auto &ecc : eccLevels)
    {
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("ECC Test", ecc);
        int ret = qrgen::savePng(qr, 4, kTestTmpFile, 4);
        ASSERT_EQ(ret, 0);

        std::uint32_t w = 0, h = 0;
        ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
        ASSERT_TRUE(w > 0u);
        cleanupTmp();
    }
}

// TC_DOM_07: Large matrix rendering (Version 30+ QR code)
TEST_CASE(TC_DOM_07_LargeMatrixVersion40)
{
    cleanupTmp();
    // Build a long payload to force high QR version (2000 lowercase bytes forces V32+ > 140 modules)
    std::string longText(2000, 'x');
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(longText.c_str(), qrcodegen::QrCode::Ecc::LOW);
    ASSERT_TRUE(qr.getSize() > 100); // Verify large dimension (> 100 modules)

    int ret = qrgen::savePng(qr, 2, kTestTmpFile, 4);
    ASSERT_EQ(ret, 0);

    std::uint32_t expectedDim = static_cast<std::uint32_t>((qr.getSize() + 8) * 2);
    std::uint32_t w = 0, h = 0;
    ASSERT_TRUE(getPngDimensions(kTestTmpFile, w, h));
    ASSERT_EQ(w, expectedDim);
    ASSERT_EQ(h, expectedDim);
    cleanupTmp();
}

// TC_DOM_08: Invalid scale and border rejection guards
TEST_CASE(TC_DOM_08_InvalidInputGuards)
{
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("Guard", qrcodegen::QrCode::Ecc::LOW);

    // Scale <= 0 rejected
    ASSERT_EQ(qrgen::savePng(qr, 0, kTestTmpFile, 4), 1);
    ASSERT_EQ(qrgen::savePng(qr, -5, kTestTmpFile, 4), 1);

    // Scale > 100 rejected
    ASSERT_EQ(qrgen::savePng(qr, 101, kTestTmpFile, 4), 1);

    // Border < 0 rejected
    ASSERT_EQ(qrgen::savePng(qr, 10, kTestTmpFile, -1), 1);

    // Null filename rejected
    ASSERT_EQ(qrgen::savePng(qr, 10, nullptr, 4), 1);
}

// TC_DOM_09: PNG format compliance and magic signature
TEST_CASE(TC_DOM_09_PngFormatCompliance)
{
    cleanupTmp();
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("Magic", qrcodegen::QrCode::Ecc::LOW);
    int ret = qrgen::savePng(qr, 2, kTestTmpFile, 2);
    ASSERT_EQ(ret, 0);

    auto bytes = readFile(kTestTmpFile);
    ASSERT_TRUE(bytes.size() > 8);

    // Verify 8-byte PNG signature
    const std::uint8_t expectedSig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    for (size_t i = 0; i < 8; ++i)
    {
        ASSERT_EQ(bytes[i], expectedSig[i]);
    }
    cleanupTmp();
}

// TC_DOM_10: Adverse fault injection (/dev/full out-of-disk simulation)
TEST_CASE(TC_DOM_10_FaultInjectionDevFull)
{
    std::ifstream devFull("/dev/full");
    if (!devFull)
    {
        // /dev/full is not present in non-Linux environments; skip
        return;
    }

    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText("FullDisk", qrcodegen::QrCode::Ecc::LOW);
    int ret = qrgen::savePng(qr, 10, "/dev/full", 4);
    // Must return 1 indicating failure, not 0
    ASSERT_EQ(ret, 1);
}

int main()
{
    return test::runAllTests();
}
