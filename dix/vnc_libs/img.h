#include "scrnintstr.h"
#include "pixmapstr.h"
#include <jpeglib.h>

void extractRectRGB(ScreenPtr pScreen,
                    int x, int y, int rect_w, int rect_h,
                    unsigned char *outBuf);

unsigned char *compress_image_to_jpeg(unsigned char *input_image_data,
                                      int width, int height,
                                      int *out_size,
                                      int quality);


// Extract RGB data for a rectangle from ScreenPtr into buffer
// Buffer must be pre-allocated with size = rect_w * rect_h * 3 bytes (RGB)
void extractRectRGB(ScreenPtr pScreen,
                    int x, int y, int rect_w, int rect_h,
                    unsigned char *outBuf)
{
    // Get screen pixmap (backing framebuffer)
    PixmapPtr pPix = (*pScreen->GetScreenPixmap)(pScreen);
    char *fb_base  = (char *) pPix->devPrivate.ptr;     // framebuffer base
    int stride     = pPix->devKind;                 // bytes per row
    int bpp        = pPix->drawable.bitsPerPixel;   // bits per pixel

    if (bpp != 24 && bpp != 32) {
        fprintf(stderr, "Unsupported bpp=%d (only 24/32 supported)\n", bpp);
        return;
    }

    int bytesPerPixel = bpp / 8;

    for (int row = 0; row < rect_h; row++) {
        // Source pointer in framebuffer
        unsigned char *src =
            (unsigned char *)(fb_base + (y + row) * stride + x * bytesPerPixel);

        // Destination pointer in output RGB buffer
        unsigned char *dst = outBuf + row * rect_w * 3;

        // Copy one scanline pixel by pixel
        for (int col = 0; col < rect_w; col++) {
            unsigned char b = src[col * bytesPerPixel + 0];
            unsigned char g = src[col * bytesPerPixel + 1];
            unsigned char r = src[col * bytesPerPixel + 2];
            // ignore alpha if 32bpp
            dst[col * 3 + 0] = r;
            dst[col * 3 + 1] = g;
            dst[col * 3 + 2] = b;
        }
    }
}


unsigned char *compress_image_to_jpeg(unsigned char *input_image_data,
                                      int width, int height,
                                      int *out_size,
                                      int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    unsigned char *jpeg_data = NULL;
    unsigned long jpeg_size = 0;

    // Error handler
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Output to memory
    jpeg_mem_dest(&cinfo, &jpeg_data, &jpeg_size);

    // Image parameters
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;      // R, G, B
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *row_pointer =
            &input_image_data[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    *out_size = (int)jpeg_size;

    jpeg_destroy_compress(&cinfo);

    return jpeg_data;  // caller must free() this
}
