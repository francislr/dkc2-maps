
#pragma once

#include "bitmap.h"
#include "rom_file.h"

#define MAKEQUAD(r, g, b) (((BYTE)(b) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(r)) << 16))

class TilePalette
{
public:
  TilePalette();

  bool LoadSpriteSubPal(CartFile& cart, UINT id);
  bool LoadPalette(CartFile& cart, UINT addr);
  bool LoadSubPalette(CartFile& cart, UINT addr,
    UINT start, UINT count);

  /* Get 24-bit color from index */
  void GetQuad(UINT index, DWORD& color) const;
  void SetColor(UINT index, WORD color);

  /* Sets the transparent bit for a color index */
  void SetTransparent(UINT index, bool set);
  bool IsTransparent(UINT index) const;

private:
  /* Bit mask for color transparency
   * 1 transparent, 0 visible */
  BYTE transparent_[16];

  /* Color data stored in 0bbbbbgg gggrrrrr */
  WORD color_[128];
};


bool DrawTile8x8(BitmapSurface& bm, TilePalette& palette,
  UINT pal_index, int x, int y, CartFile& cart,
  bool flip_horz, bool flip_vert
);
bool DrawTile8x8(BitmapSurface& bm, TilePalette& palette,
  UINT pal_index, int x, int y, BYTE *data,
  bool flip_horz, bool flip_vert
);
