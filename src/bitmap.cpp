
#include "stdafx.h"

#include "bitmap.h"

BitmapSurface::BitmapSurface()
{
  hdc_ = NULL;
  hbm_ = NULL;
  hBmMask_ = NULL;
  width_ = 0,
  height_ = 0;
}


BitmapSurface::~BitmapSurface()
{
  Destroy();
}

bool BitmapSurface::CreateDC(HDC hdc)
{
  Destroy();
  hdc_ = CreateCompatibleDC(hdc);
  return hdc_ != NULL;
}

bool BitmapSurface::CreateBM(HDC hdc, int width, int height)
{
  DestroyBM();
  hbm_ = CreateCompatibleBitmap(hdc, width, height);
  if (!hbm_) {
    return false;
  }
  width_ = width,
  height_ = height;
  hPixBm_ = (HBITMAP) SelectObject(hdc_, (HGDIOBJ) hbm_);
  return true;
}

bool BitmapSurface::CreateMask(HDC hdc)
{
  if (!hbm_) {
    return false;
  }
  DestroyMask();
  hBmMask_ = CreateBitmap(width_, height_, 1, 1, NULL);
  return true;
}

bool BitmapSurface::CreateDIB(BITMAPINFO16& bi, int width, int height,
  void** bmData)
{
  DestroyBM();
  if (!hdc_) {
    return false;
  }
  bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
  bi.bmiHeader.biWidth = width;
  bi.bmiHeader.biHeight = height;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 4;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = BI_RGB;
  bi.bmiHeader.biXPelsPerMeter = 0;
  bi.bmiHeader.biYPelsPerMeter = 0;
  bi.bmiHeader.biClrUsed = 16;
  bi.bmiHeader.biClrImportant = 16;
  hbm_ = CreateDIBSection(NULL,
    (BITMAPINFO*) &bi,
    DIB_RGB_COLORS,
    bmData,
    NULL,
    0
  );
  if (!hbm_) {
    return false;
  }
  hPixBm_ = (HBITMAP) SelectObject(hdc_, (HGDIOBJ) hbm_);
  return true;
}

bool BitmapSurface::SetBits(int x, int y, int width, int height, void* bmData)
{
  if (!hdc_) {
    return false;
  }
  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;
  bmi.bmiHeader.biClrImportant = 0;
  bmi.bmiHeader.biClrUsed = 0;
  bmi.bmiHeader.biXPelsPerMeter = 0;
  bmi.bmiHeader.biYPelsPerMeter = 0;
  return SetDIBitsToDevice(hdc_,
    x, y, width, height,
    0, 0, 0, height,
    bmData, (BITMAPINFO*) &bmi,
    DIB_RGB_COLORS
  ) == height;
}

bool BitmapSurface::SetMaskBits(int x, int y, int width, int height, void* bmData)
{
  HDC hdc;
  hdc = CreateCompatibleDC(hdc_);
  if (!hdc) {
    return false;
  }
  HGDIOBJ oldBm;
  oldBm = SelectObject(hdc, (HGDIOBJ) hBmMask_);
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 1;
  bmi.bmiHeader.biCompression = BI_RGB;
  int n = SetDIBitsToDevice(hdc,
    x, y, width, height,
    0, 0, 0, height,
    bmData, (BITMAPINFO*) &bmi,
    DIB_RGB_COLORS
  );
  SelectObject(hdc, (HGDIOBJ) oldBm);
  DeleteDC(hdc);
  return true;
}

bool BitmapSurface::ResetMask()
{
  if (!hBmMask_) {
    return false;
  }
  HDC hdc;
  hdc = CreateCompatibleDC(NULL);
  if (!hdc) {
    return false;
  }
  HGDIOBJ oldBm;
  oldBm = SelectObject(hdc, (HGDIOBJ) hBmMask_);
  BOOL b = BitBlt(hdc, 0, 0, width_, height_, hdc, 0, 0, WHITENESS);
  SelectObject(hdc, (HGDIOBJ) oldBm);
  DeleteDC(hdc); 
  return true;
}

bool BitmapSurface::Draw(HDC hdc, int to_x, int to_y, int width, int height,
  int from_x, int from_y)
{
  if (!hbm_) {
    return false;
  }
  if (width < 0 || height < 0) {
    int srcWidth = width,
        srcHeight = height;
    if (width < 0) {
      width = -width;
      from_x += width - 1;
    }
    if (height < 0) {
      height = -height;
      from_y += height - 1;
    }
    StretchBlt(hdc, to_x, to_y, width, height,
      hdc_, from_x, from_y, srcWidth, srcHeight, SRCCOPY);
    return true;
  }
  if (hBmMask_) {
    return MaskBlt(hdc, to_x, to_y, width, height, hdc_,
      from_x, from_y, hBmMask_, from_x, from_y, MAKEROP4(0x00AA0029, SRCCOPY)) == TRUE;
  }
  return BitBlt(hdc, to_x, to_y, width, height, hdc_, from_x, from_y, SRCCOPY) == TRUE;
}

bool BitmapSurface::DrawDC(HDC hdc, int to_x, int to_y,
  int from_x, int from_y)
{
  return Draw(hdc, to_x, to_y, width_, height_, from_x, from_y);
}

bool BitmapSurface::Draw(BitmapSurface& bm, int to_x, int to_y,
    int from_x, int from_y)
{
  return Draw(bm.hdc_, to_x, to_y, width_, height_, from_x, from_y);
}

bool BitmapSurface::Draw(BitmapSurface& bm, int to_x, int to_y,
    int width, int height, int from_x, int from_y)
{
  return Draw(bm.hdc_, to_x, to_y, width, height, from_x, from_y);
}

bool BitmapSurface::DrawSized(HDC hdc, int to_x, int to_y,
    int& width, int& height,
    int from_x, int from_y, bool proportional)
{
  if (proportional) {
    width = ((float) width_ / height_) * width;
    if (height > height_) {
      width = width_,
      height = height_;
    }
  }

  SelectObject(hdc_, (HGDIOBJ) hBmMask_);
  StretchBlt(hdc, to_x, to_y, width, height,
      hdc_, from_x, from_y, width_, height_, SRCAND);
  SelectObject(hdc_, (HGDIOBJ) hbm_);
  SetStretchBltMode(hdc_, HALFTONE);
  return StretchBlt(hdc, to_x, to_y, width, height,
    hdc_, from_x, from_y, width_, height_, SRCPAINT) != FALSE;
}

bool BitmapSurface::DrawFlipped(BitmapSurface& bm, int to_x, int to_y,
  int from_x, int from_y,
  bool flip_horz, bool flip_vert)
{
  SetStretchBltMode(bm.hdc_, COLORONCOLOR);
  int width = width_,
      height = height_;
  if (flip_horz) {
    //from_x += width - 1;
    to_x += width;
    width *= -1;
  }
  if (flip_vert) {
   to_y += height;
   height *= -1;
  }
  SelectObject(hdc_, (HGDIOBJ) hBmMask_);
  StretchBlt(bm.hdc_, to_x, to_y, width, height,
      hdc_, from_x, from_y, width_, height_, SRCAND);
  SelectObject(hdc_, (HGDIOBJ) hbm_);
  return StretchBlt(bm.hdc_, to_x, to_y, width, height,
    hdc_, from_x, from_y, width_, height_, SRCPAINT) != FALSE;
}

void BitmapSurface::DrawSelection(const RECT& zone, COLORREF color1,
  COLORREF color2, bool clip)
{
  DrawFocusRect(hdc_, &zone);
  /*
  HPEN marqueePen = CreatePen(PS_DOT, 1, color1);
  HPEN oldPen = (HPEN) SelectObject(hdc_, (HGDIOBJ) marqueePen);
  COLORREF oldColor = SetBkColor(hdc_, color2);
  HBRUSH oldBrush = (HBRUSH) SelectObject(hdc_, (HGDIOBJ) GetStockObject(BLACK_BRUSH));
  MoveToEx(hdc_, zone.left, zone.bottom, NULL);
  LineTo(hdc_, zone.left, zone.top);
  LineTo(hdc_, zone.right + 1, zone.top);
  LineTo(hdc_, zone.right + 1, zone.bottom + 1);
  LineTo(hdc_, zone.left, zone.bottom + 1);
  SelectObject(hdc_, (HGDIOBJ) oldPen);
  SetBkColor(hdc_, oldColor);
  SelectObject(hdc_, (HGDIOBJ) oldBrush);
  DeleteObject((HGDIOBJ) marqueePen);
  if (clip) {
	  ExcludeClipRect(hdc_, zone.left, zone.top, zone.right, zone.top + 1);
	  ExcludeClipRect(hdc_, zone.left, zone.top, zone.left + 1, zone.bottom);
	  ExcludeClipRect(hdc_, zone.right, zone.top, zone.right + 1, zone.bottom);
	  ExcludeClipRect(hdc_, zone.left, zone.bottom, zone.right, zone.bottom + 1);
  }*/
}

bool BitmapSurface::Resize(int width, int height, int factor)
{
  if (factor) {
    width -= (width % factor),
    height -= (height % factor);
    width += factor,
    height += factor;
  }
  if (width_ == width && height_ == height) {
    return true;
  }
  HDC hdc;
  hdc = GetDC(NULL);
  bool result;
  result = CreateBM(hdc, width, height);
  ReleaseDC(NULL, hdc);
  return result;
}

void BitmapSurface::DestroyDC()
{
  if (hdc_) {
    if (hbm_) {
      SelectObject(hdc_, (HGDIOBJ) hPixBm_);
    }
    DeleteDC(hdc_);
    hdc_ = NULL;
  }
}

void BitmapSurface::DestroyBM()
{
  if (hbm_) {
    if (hdc_) {
      SelectObject(hdc_, (HGDIOBJ) hPixBm_);
    }
    DeleteObject((HGDIOBJ) hbm_);
    hbm_ = NULL;
  }
}

void BitmapSurface::DestroyMask()
{
  if (hBmMask_) {
    DeleteObject((HGDIOBJ) hBmMask_);
    hBmMask_ = NULL;
  }
}

void BitmapSurface::Destroy()
{
  DestroyDC();
  DestroyBM();
  DestroyMask();
}
