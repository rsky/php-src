#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"


#ifdef HAVE_LIBWEBP
#include <webp/decode.h>
#include <webp/encode.h>
#include "gdhelpers.h"

const char * gdWebpGetVersionString()
{
	return "not defined";
}

gdImagePtr gdImageCreateFromWebp (FILE * inFile)
{
	gdImagePtr im;
	gdIOCtx *in = gdNewFileCtx(inFile);
	im = gdImageCreateFromWebpCtx(in);
	in->gd_free(in);

	return im;
}

static void gdImageCopyFromWebpDecoder (gdImagePtr im, WebPDecoderConfig * config)
{
	int width = config->input.width;
	int height = config->input.height;
	int x, y;
	uint8_t *row, *p;

	row = config->output.u.RGBA.rgba;
	for (y = 0; y < height; y++) {
		p = row;
		for (x = 0; x < width; x++) {
			im->tpixels[y][x] = gdTrueColorAlpha(p[0], p[1], p[2],
			                                     gdAlphaMax - (p[3] >> 1));
			p += 4;
		}
		row += config->output.u.RGBA.stride;
	}
}

gdImagePtr gdImageCreateFromWebpPtr (int size, void *data)
{
	int    width, height, ret;
	WebPDecoderConfig config;
	gdImagePtr im;

	WebPInitDecoderConfig(&config);
	config.output.colorspace = MODE_RGBA;
	ret = WebPDecode(data, size, &config);
	if (ret != VP8_STATUS_OK) {
		WebPFreeDecBuffer(&config.output);
		php_gd_error("WebP decode: fail to decode input data");
		return NULL;
	}

	width = config.input.width;
	height = config.input.height;
	im = gdImageCreateTrueColor(width, height);
	if (im) {
		gdImageCopyFromWebpDecoder(im, &config);
	}
	WebPFreeDecBuffer(&config.output);

	return im;
}

gdImagePtr gdImageCreateFromWebpCtx (gdIOCtx * infile)
{
	int    width, height, ret;
	unsigned char   *filedata;
	unsigned char   dummy[1024];
	size_t size = 0, n;
	gdImagePtr im;

	do {
		n = gdGetBuf(dummy, 1024, infile);
		size += n;
	} while (n != EOF);

	filedata = gdMalloc(size);
	if  (!filedata) {
		php_gd_error("WebP decode: alloc failed");
		return NULL;
	}

	gdGetBuf(filedata, size, infile);
	im = gdImageCreateFromWebpPtr(size, filedata);
	gdFree(filedata);

	return im;
}

void gdImageWebpEx (gdImagePtr im, FILE * outFile, int quality)
{
	gdIOCtx *out = gdNewFileCtx(outFile);
	gdImageWebpCtx(im, out, quality);
	out->gd_free(out);
}

void gdImageWebp (gdImagePtr im, FILE * outFile)
{
	gdIOCtx *out = gdNewFileCtx(outFile);
	gdImageWebpCtx(im, out, -1);
	out->gd_free(out);
}

void * gdImageWebpPtr (gdImagePtr im, int *size)
{
	void *rv;
	gdIOCtx *out = gdNewDynamicCtx(2048, NULL);
	gdImageWebpCtx(im, out, -1);
	rv = gdDPExtractData(out, size);
	out->gd_free(out);

	return rv;
}

void * gdImageWebpPtrEx (gdImagePtr im, int *size, int quality)
{
	void *rv;
	gdIOCtx *out = gdNewDynamicCtx(2048, NULL);
	gdImageWebpCtx(im, out, quality);
	rv = gdDPExtractData(out, size);
	out->gd_free(out);
	return rv;
}

static void gdImageCopyToRGBAPtr (gdImagePtr im, uint8_t * rgba)
{
	int width = im->sx;
	int height = im->sy;
	int x, y, c, a;
	uint8_t *p = rgba;

	if (gdImageTrueColor(im)) {
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				c = gdImageTrueColorPixel(im, x, y);
				a = gdTrueColorGetAlpha(c);
				*p++ = gdTrueColorGetRed(c);
				*p++ = gdTrueColorGetGreen(c);
				*p++ = gdTrueColorGetBlue(c);
				*p++ = (a == gdAlphaOpaque) ? 0xff : ((gdAlphaMax - a) << 1);
			}
		}
	} else {
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				c = gdImagePalettePixel(im, x, y);
				*p++ = im->red[c];
				*p++ = im->green[c];
				*p++ = im->blue[c];
			}
		}
	}
}

static int gdWriteWebp (const uint8_t * data, size_t data_size,
                        const WebPPicture * picture)
{
	gdIOCtx *ctx = (gdIOCtx *) picture->custom_ptr;
	return ((ctx->putBuf)(ctx, data, data_size) == data_size);
}

static int gdWebpPictureImportRGBA (gdImagePtr im, WebPPicture * picture,
                                    uint8_t * rgba, int stride)
{
	if (gdImageTrueColor(im)) {
		if (im->saveAlphaFlag) {
			if (WebPPictureImportRGBA(picture, rgba, stride)) {
				WebPCleanupTransparentArea(picture);
				return 1;
			}
			return 0;
		} else {
			return WebPPictureImportRGBX(picture, rgba, stride);
		}
	} else {
		return WebPPictureImportRGB(picture, rgba, stride);
	}
}

void gdImageWebpCtxEx (gdImagePtr im, gdIOCtx * outfile, WebPConfig * config)
{
	int width = im->sx;
	int height = im->sy;
	int stride;
	uint8_t *rgba;
	WebPPicture picture;

	stride = width * (gdImageTrueColor(im) ? 4 : 3);
	if ((rgba = (uint8_t *)gdCalloc(stride * height, sizeof(uint8_t))) == NULL) {
		php_gd_error("gd-webp error: cannot allocate RGBA buffer");
		return;
	}

	gdImageCopyToRGBAPtr(im, rgba);

	if (!WebPPictureInit(&picture)) {
		gdFree(rgba);
		php_gd_error("gd-webp error: WebP Encoder failed");
		return;
	}

	picture.use_argb = 1;
	picture.width = width;
	picture.height = height;
	picture.writer = gdWriteWebp;
	picture.custom_ptr = outfile;

	if (gdWebpPictureImportRGBA(im, &picture, rgba, stride)) {
		if (!WebPEncode(config, &picture)) {
			php_gd_error("gd-webp error: WebP Encoder failed");
		}
	} else {
		php_gd_error("gd-webp error: WebP Encoder failed");
	}

	WebPPictureFree(&picture);
	gdFree(rgba);
}

void gdImageWebpCtx (gdImagePtr im, gdIOCtx * outfile, int quality)
{
	WebPConfig config;

	WebPConfigInit(&config);
#define DEFAULT_QUALITY -1
#define MIN_QUALITY 0
#define MAX_QUALITY 100
	if (quality != DEFAULT_QUALITY) {
		if (quality < MIN_QUALITY) {
			php_gd_error("Wrong quality value %d.", quality);
			return;
		}
		if (quality > MAX_QUALITY) {
			config.lossless = 1;
		} else {
			config.quality = (float) quality;
		}
	}

	gdImageWebpCtxEx(im, outfile, &config);
}

#endif /* HAVE_LIBWEBP */
