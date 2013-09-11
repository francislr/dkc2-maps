
#include "stdafx.h"
#include <vector>

#include "tile.h"
#include "bitmap.h"

TilePalette::TilePalette()
{
  for (UINT i = 0; i < 16; i++) {
    transparent_[i] = 0x00000000;
  }
}

bool TilePalette::LoadSpriteSubPal(CartFile& cart, UINT id)
{
  CartFile::Scope scope(cart);
  DWORD addr;
  addr = 0x003D0000;

  if (!cart.Read(0x003D5FEE + (id * 2), &addr, 2)) {
    return false;
  }
  cart.Seek(addr);
  color_[0] = 0x00000000;
  for (int i = 1; i < 16; i++) {
    WORD color;
    if (!cart.ReadWord(color)) {
      return false;
    }
    color_[i] = color;
  }
  SetTransparent(0, true);
  return true;
}

bool TilePalette::LoadPalette(CartFile& cart, UINT addr)
{
  CartFile::Scope scope(cart);
  cart.Seek(addr);
  if (!cart.Read(color_, 2 * 128)) {
    return false;
  }
  return true;
}

bool TilePalette::LoadSubPalette(CartFile& cart, UINT addr,
    UINT start, UINT count)
{
  CartFile::Scope scope(cart);
  cart.Seek(addr);
  if (start + count >= 128) {
    return false;
  }
  if (!cart.Read(&color_[start], 2 * count)) {
    return false;
  }
  return true;
}

void TilePalette::GetQuad(UINT index, DWORD& color) const
{
  if (index >= 128) {
    return ;
  }
  color = color_[index];
  BYTE r, g, b;
  r = (BYTE) ((color & 0x001F));
  g = (BYTE) ((color & 0x03E0) >> 5);
  b = (BYTE) ((color & 0x7C00) >> 10);
  // Convert color from 5 bits to 8 bits
  r = (BYTE) (r / 32.0 * 256);
  g = (BYTE) (g / 32.0 * 256);
  b = (BYTE) (b / 32.0 * 256);
  color = MAKEQUAD(r, g, b);
}

void TilePalette::SetColor(UINT index, WORD color)
{
  if (index >= 128) {
    return ;
  }
  color_[index] = color;
}

void TilePalette::SetTransparent(UINT index, bool set)
{
  if (index >= 128) {
    return ;
  }
  UINT byte_index = index / 8,
       bit_pos = index % 8;
  BYTE mask = transparent_[byte_index];
  if (set) {
    mask |= 1 << bit_pos;
  }
  else {
    mask &= ~(1 << bit_pos);
  }
  transparent_[byte_index] = mask;
}

bool TilePalette::IsTransparent(UINT index) const
{
  if (index >= 128) {
    return true;
  }
  UINT byte_index = index / 8,
       bit_pos = index % 8;
  return (transparent_[byte_index] & (1 << bit_pos)) != 0;
}

bool DrawTile8x8(BitmapSurface& bm, TilePalette& palette,
  UINT pal_index, int x, int y, CartFile& cart,
  bool flip_horz, bool flip_vert
)
{
  BYTE tile_data[32];
  if (!cart.Read(tile_data, 32)) {
    return false;
  }
  DrawTile8x8(bm, palette, pal_index, x, y, tile_data,
    flip_horz, flip_vert);
}

bool DrawTile8x8(BitmapSurface& bm, TilePalette& palette,
  UINT pal_index, int x, int y, BYTE *data,
  bool flip_horz, bool flip_vert)
{
  UINT i, j, c, pix_x, pix_y;
  BYTE bp1, bp2, bp3, bp4;
  DWORD bmData[64];
  DWORD bmMaskData[8] = {};
  for (i = 0; i < 8; i++) {
		bp1 = data[i * 2 + 0x00], bp2 = data[i * 2 + 0x01];
		bp3 = data[i * 2 + 0x10], bp4 = data[i * 2 + 0x11];
    for (j = 0; j < 8; j++) {
      /* Get the color index */
      c = ((bp1 & 0x80) >> 7) | ((bp2 & 0x80) >> 6) |
          ((bp3 & 0x80) >> 5) | ((bp4 & 0x80) >> 4);
      c += pal_index;
      DWORD color;
      /* Get the color from index */
      pix_x = (flip_horz) ? 7 - j : j;
      pix_y = (flip_vert) ? 7 - i : i;
      if (palette.IsTransparent(c)) {
        // Pixel is transparent
        bmMaskData[pix_y] |= 1 << (7 - pix_x);
        bmData[(pix_y * 8) + pix_x] = 0;
      }
      else {
        // Pixel is visible
        palette.GetQuad(c, color);
        bmData[(pix_y * 8) + pix_x] = color;
      }
      bp1 <<= 1, bp2 <<= 1, bp3 <<= 1, bp4 <<= 1;
    }
  }
  bm.SetMaskBits(x, y, 8, 8, bmMaskData);
  return bm.SetBits(x, y, 8, 8, bmData);
}
