
#pragma once

#include "window.h"

struct BITMAPINFO16
{
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColor[16];
};

class BitmapSurface
{
public:
  BitmapSurface();
  virtual ~BitmapSurface();
  bool CreateDC(HDC hdc);
  bool CreateBM(HDC hdc, int width, int height);
  bool CreateMask(HDC hdc);
  bool CreateDIB(BITMAPINFO16& bi, int width, int height,
    void** bmData);
  bool ResetMask();
  bool SetBits(int x, int y, int width, int height, void* bmData);
  bool SetMaskBits(int x, int y, int width, int height, void* bmData);
  virtual bool Draw(HDC hdc, int to_x, int to_y, int width, int height,
    int from_x, int from_y);
  virtual bool DrawDC(HDC hdc, int to_x, int to_y,
    int from_x, int from_y);
  virtual bool Draw(BitmapSurface& bm, int to_x, int to_y,
    int from_x, int from_y);
  virtual bool Draw(BitmapSurface& bm, int to_x, int to_y,
    int width, int height, int from_x, int from_y);
  virtual bool DrawSized(HDC hdc, int to_x, int to_y,
    int& width, int& height,
    int from_x, int from_y,
    bool proportional
  );
  virtual bool DrawFlipped(BitmapSurface& bm, int to_x, int to_y,
    int from_x, int from_y,
    bool flip_horz, bool flip_vert
  );

  void DrawSelection(const RECT& zone, COLORREF color1,
    COLORREF color2, bool clip);

  bool Resize(int width, int height, int factor);
  void DestroyDC();
  void DestroyBM();
  void DestroyMask();
  void Destroy();

protected:
  UINT width_, height_;
  HDC hdc_;
  HBITMAP hbm_;
  HBITMAP hBmMask_;
  HBITMAP hPixBm_;

private:
};
