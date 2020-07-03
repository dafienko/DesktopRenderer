#include "winUtil.h"
#include <gl/GL.h>
#include <windows.h>
#include <math.h>

typedef struct colorIndexState {
	GLfloat amb[3];     /* ambient color / bottom of ramp */
	GLfloat diff[3];    /* diffuse color / middle of ramp */
	GLfloat spec[3];    /* specular color / top of ramp */
	GLfloat ratio;      /* ratio of diffuse to specular in ramp */
	GLint indexes[3];   /* where ramp was placed in palette */
} colorIndexState;

colorIndexState colors[] = {
	{
		{ 0.0F, 0.0F, 0.0F },
		{ 0.1F, 0.6F, 0.3F },
		{ 1.0F, 1.0F, 1.0F },
		0.75F, { 0, 0, 0 },
	},
	{
		{ 0.0F, 0.0F, 0.0F },
		{ 0.0F, 0.2F, 0.5F },
		{ 1.0F, 1.0F, 1.0F },

		0.75F, { 0, 0, 0 },
	},
	{
		{ 0.0F, 0.05F, 0.05F },
		{ 0.6F, 0.0F, 0.8F },
		{ 1.0F, 1.0F, 1.0F },
		0.75F, { 0, 0, 0 },
	},
};

#define NUM_COLORS (sizeof(colors) / sizeof(colors[0]))

HBITMAP setupDIB(HDC hDC)
{
	BITMAPINFO* bmInfo;
	BITMAPINFOHEADER* bmHeader;
	UINT usage;
	VOID* base;
	int bmiSize;
	int bitsPerPixel;

	bmiSize = sizeof(*bmInfo);
	bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL);

	switch (bitsPerPixel) {
	case 8:
		/* bmiColors is 256 WORD palette indices */
		bmiSize += (256 * sizeof(WORD)) - sizeof(RGBQUAD);
		break;
	case 16:
		/* bmiColors is 3 WORD component masks */
		bmiSize += (3 * sizeof(DWORD)) - sizeof(RGBQUAD);
		break;
	case 24:
	case 32:
	default:
		/* bmiColors not used */
		break;
	}

	bmInfo = (BITMAPINFO*)calloc(1, bmiSize);
	bmHeader = &bmInfo->bmiHeader;

	bmHeader->biSize = sizeof(*bmHeader);
	bmHeader->biWidth = 1920;
	bmHeader->biHeight = 1080;
	bmHeader->biPlanes = 1;			/* must be 1 */
	bmHeader->biBitCount = bitsPerPixel;
	bmHeader->biXPelsPerMeter = 0;
	bmHeader->biYPelsPerMeter = 0;
	bmHeader->biClrUsed = 0;			/* all are used */
	bmHeader->biClrImportant = 0;		/* all are important */

	switch (bitsPerPixel) {
	case 8:
		bmHeader->biCompression = BI_RGB;
		bmHeader->biSizeImage = 0;
		usage = DIB_PAL_COLORS;
		/* bmiColors is 256 WORD palette indices */
		{
			WORD* palIndex = (WORD*)&bmInfo->bmiColors[0];
			int i;

			for (i = 0; i < 256; i++) {
				palIndex[i] = i;
			}
		}
		break;
	case 16:
		bmHeader->biCompression = BI_RGB;
		bmHeader->biSizeImage = 0;
		usage = DIB_RGB_COLORS;
		/* bmiColors is 3 WORD component masks */
		{
			DWORD* compMask = (DWORD*)&bmInfo->bmiColors[0];

			compMask[0] = 0xF800;
			compMask[1] = 0x07E0;
			compMask[2] = 0x001F;
		}
		break;
	case 24:
	case 32:
	default:
		bmHeader->biCompression = BI_RGB;
		bmHeader->biSizeImage = 0;
		usage = DIB_RGB_COLORS;
		/* bmiColors not used */
		break;
	}

	HBITMAP hBitmap = CreateDIBSection(hDC, bmInfo, usage, &base, NULL, 0);
	if (hBitmap == NULL) {
		(void)MessageBox(NULL,
			"Failed to create DIBSection.",
			"OpenGL application error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	HBITMAP hOldBitmap = SelectObject(hDC, hBitmap);

	free(bmInfo);

	return hOldBitmap;
}


void setupPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* size of this pfd */
	1,				/* version num */
	PFD_SUPPORT_OPENGL,		/* support OpenGL */
	0,				/* pixel type */
	0,				/* 8-bit color depth */
	0, 0, 0, 0, 0, 0,		/* color bits (ignored) */
	0,				/* no alpha buffer */
	0,				/* alpha bits (ignored) */
	0,				/* no accumulation buffer */
	0, 0, 0, 0,			/* accum bits (ignored) */
	16,				/* depth buffer */
	0,				/* no stencil buffer */
	0,				/* no auxiliary buffers */
	PFD_MAIN_PLANE,			/* main layer */
	0,				/* reserved */
	0, 0, 0,			/* no layer, visible, damage masks */
	};
	int SelectedPixelFormat;
	BOOL retVal;

	pfd.cColorBits = GetDeviceCaps(hDC, BITSPIXEL);
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags |= PFD_DOUBLEBUFFER;
	pfd.dwFlags |= PFD_DRAW_TO_BITMAP;

	SelectedPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (SelectedPixelFormat == 0) {
		(void)MessageBox(NULL,
			"Failed to find acceptable pixel format.",
			"OpenGL application error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	retVal = SetPixelFormat(hDC, SelectedPixelFormat, &pfd);
	if (retVal != TRUE) {
		(void)MessageBox(NULL,
			"Failed to set pixel format.",
			"OpenGL application error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}
}


HPALETTE setupPalette(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd;
	LOGPALETTE* pPal;
	int pixelFormat = GetPixelFormat(hDC);
	int paletteSize;

	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	/*
	** Determine if a palette is needed and if so what size.
	*/
	if (pfd.dwFlags & PFD_NEED_PALETTE) {
		paletteSize = 1 << pfd.cColorBits;
	}
	else if (pfd.iPixelType == PFD_TYPE_COLORINDEX) {
		paletteSize = 4096;
	}
	else {
		return;
	}

	pPal = (LOGPALETTE*)
		malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
	pPal->palVersion = 0x300;
	pPal->palNumEntries = paletteSize;

	if (pfd.iPixelType == PFD_TYPE_RGBA) {
		/*
		** Fill the logical paletee with RGB color ramps
		*/
		int redMask = (1 << pfd.cRedBits) - 1;
		int greenMask = (1 << pfd.cGreenBits) - 1;
		int blueMask = (1 << pfd.cBlueBits) - 1;
		int i;

		for (i = 0; i < paletteSize; ++i) {
			pPal->palPalEntry[i].peRed =
				(((i >> pfd.cRedShift)& redMask) * 255) / redMask;
			pPal->palPalEntry[i].peGreen =
				(((i >> pfd.cGreenShift)& greenMask) * 255) / greenMask;
			pPal->palPalEntry[i].peBlue =
				(((i >> pfd.cBlueShift)& blueMask) * 255) / blueMask;
			pPal->palPalEntry[i].peFlags = 0;
		}
	}
	else {
		/*
		** Fill the logical palette with color ramps.
		**
		** Set up the logical palette so that it can be realized
		** into the system palette as an identity palette.
		**
		** 1) The default static entries should be present and at the right
		**    location.  The easiest way to do this is to grab them from
		**    the current system palette.
		**
		** 2) All non-static entries should be initialized to unique values.
		**    The easiest way to do this is to ensure that all of the non-static
		**    entries have the PC_NOCOLLAPSE flag bit set.
		*/
		int numRamps = NUM_COLORS;
		int rampSize = (paletteSize - 20) / numRamps;
		int extra = (paletteSize - 20) - (numRamps * rampSize);
		int i, r;

		/*
		** Initialize static entries by copying them from the
		** current system palette.
		*/
		GetSystemPaletteEntries(hDC, 0, paletteSize, &pPal->palPalEntry[0]);

		/*
		** Fill in non-static entries with desired colors.
		*/
		for (r = 0; r < numRamps; ++r) {
			int rampBase = r * rampSize + 10;
			PALETTEENTRY* pe = &pPal->palPalEntry[rampBase];
			int diffSize = (int)(rampSize * colors[r].ratio);
			int specSize = rampSize - diffSize;

			for (i = 0; i < rampSize; ++i) {
				GLfloat* c0, * c1;
				GLint a;

				if (i < diffSize) {
					c0 = colors[r].amb;
					c1 = colors[r].diff;
					a = (i * 255) / (diffSize - 1);
				}
				else {
					c0 = colors[r].diff;
					c1 = colors[r].spec;
					a = ((i - diffSize) * 255) / (specSize - 1);
				}

				pe[i].peRed = (BYTE)(a * (c1[0] - c0[0]) + 255 * c0[0]);
				pe[i].peGreen = (BYTE)(a * (c1[1] - c0[1]) + 255 * c0[1]);
				pe[i].peBlue = (BYTE)(a * (c1[2] - c0[2]) + 255 * c0[2]);
				pe[i].peFlags = PC_NOCOLLAPSE;
			}

			colors[r].indexes[0] = rampBase;
			colors[r].indexes[1] = rampBase + (diffSize - 1);
			colors[r].indexes[2] = rampBase + (rampSize - 1);
		}

		/*
		** Initialize any remaining non-static entries.
		*/
		for (i = 0; i < extra; ++i) {
			int index = numRamps * rampSize + 10 + i;
			PALETTEENTRY* pe = &pPal->palPalEntry[index];

			pe->peRed = (BYTE)0;
			pe->peGreen = (BYTE)0;
			pe->peBlue = (BYTE)0;
			pe->peFlags = PC_NOCOLLAPSE;
		}
	}

	HPALETTE hPalette = CreatePalette(pPal);
	free(pPal);

	if (hPalette) {
		SelectPalette(hDC, hPalette, FALSE);
		RealizePalette(hDC);
	}

	return hPalette;
}