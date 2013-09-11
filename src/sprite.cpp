
#include "stdafx.h"
#include <algorithm>

#include "sprite.h"
#include "rom_file.h"

SpriteTile::SpriteTile()
{
  SetRect(&base_, 0, 0, 0, 0);
  width_ = 0,
  height_ = 0;
}

SpriteTile::~SpriteTile()
{
}

bool Sprite_Draw16x16Tile()
{
  return true;
}

bool SpriteTile::Load(CartFile& cart, HDC hdc, UINT sprite_id, UINT pal_id)
{
  CartFile::Scope scope(cart);
  BYTE n16x16, n8x8, start8x8, n8x8b,
       start8x8b, start16x16b, n16x16b;
  BitmapSurface& bm = *this;
  /* Load the sub palette */
  TilePalette palette;
  if (!palette.LoadSpriteSubPal(cart, pal_id)) {
    return false;
  }
  /* Get the address of the gfx by index, including the bank */
  DWORD addr = 0x000000;
  if (!cart.Read(0x3C8000 + sprite_id, &addr, 3)) {
    return false;
  }
  /* ROM address to file address */
  addr = ROM2F(addr);
  cart.Seek(addr);
  /* Read the sprite header */
  if (!cart.Readf("BBBBBBBB",
    &n16x16, &n8x8, &start8x8, &n8x8b, &start8x8b,
    &start16x16b, NULL, &n16x16b))
  {
    return false;
  }
  UINT count = n16x16 + n8x8 + n8x8b;
  std::vector<POINT> position;
  UINT i;
  for (i = 0; i < count; i++) {
    POINT pos = {};
    if (!cart.Readf("BB", &pos.x, &pos.y)) {
      return false;
    }
    pos.x -= 128;
    pos.y -= 128;
    position.push_back(pos);
  }
  SetRect(&base_, 256, 256, -256, -256);
  for (i = 0; i < count; i++) {
    UINT wh = i < n16x16 ? 16 : 8;
		RECT size = {
      position[i].x, // left
      position[i].y, // top
      position[i].x + wh, // right
      position[i].y + wh // bottom
    };
    if (size.left < base_.left) {
	    base_.left = size.left;
    }
    if (size.right > base_.right) {
      base_.right = size.right;
    }
    if (size.top < base_.top) {
      base_.top = size.top;
    }
    if (size.bottom > base_.bottom) {
      base_.bottom = size.bottom;
    }
  }
  width_ = abs(base_.right - base_.left),
  height_ = abs(base_.top - base_.bottom);
  if (!CreateDC(hdc)) {
    return false;
  }
  if (!CreateBM(hdc, width_, height_)) {
    return false;
  }
  if (!CreateMask(hdc_)) {
    return false;
  }
  ResetMask();
  int index = 0;

  if (n16x16 != 0) {
    for (i = 0; i < n16x16; i++, index++) {
      POINT pos = position[index];
      pos.x -= base_.left;
      pos.y -= base_.top;
      UINT x1 = (i % 8) * 64,
           x2 = (i / 8) * 1024;
      {
        CartFile::Scope scope(cart);
        cart.Skip(x1 + x2);
        DrawTile8x8(bm, palette, 0, pos.x, pos.y,
          cart, false, false);
        DrawTile8x8(bm, palette, 0, pos.x + 8, pos.y,
          cart, false, false);
      }
      {
        CartFile::Scope scope(cart);
        int spr_lot = n16x16 - n16x16b / 2;
        if (i < spr_lot) {
          cart.Skip(x1 + x2 + 512);
        }
        else {
          cart.Skip(start16x16b * 32 + (i - spr_lot) * 64);
        }
        DrawTile8x8(bm, palette, 0, pos.x, pos.y + 8,
          cart, false, false);
        DrawTile8x8(bm, palette, 0, pos.x + 8, pos.y + 8,
          cart, false, false);
      }
    }
  }
  if (n8x8 != 0) {
    CartFile::Scope scope(cart);
    cart.Skip(start8x8 * 32);
    for (i = 0; i < n8x8; i++, index++) {
      POINT pos = position[index];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0, pos.x, pos.y,
        cart, false, false);
    }
  }
  if (n8x8b != 0) {
    CartFile::Scope scope(cart);
    cart.Skip(start8x8b * 32);
    for (i = 0; i < n8x8b; i++, index++) {
      POINT pos = position[index];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0, pos.x, pos.y,
        cart, false, false);
    }
  }

  /*if (n8x8 != 0) {
    CartFile::Scope scope(cart);
    cart.Skip(start8x8 * 32);
    for (i = 0; i < n8x8; i++) {
      POINT pos = pos8x8[i];
      pos.x -= base_.left;
      pos.y -= base_.top;
      //DrawTile8x8(bm, palette, 0, pos.x, pos.y,
      //  cart, false, false);
    }
  }*/


#if 0
  BYTE *data, *p;
  data_size = n16x16 * 64;
  data = new BYTE[data_size];
  if (!data) {
    return false;
  }
  p = data;
  if (!cart.Read(data, data_size)) {
    delete[] data;
    return false;
  }
  if (n16x16 != 0) {
    for (i = 0; i < n16x16; i++) {
      POINT pos = pos16x16[i];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0, pos.x, pos.y,
        p, false, false);
      DrawTile8x8(bm, palette, 0, pos.x + 8, pos.y,
        p + 32, false, false);
      p += 64;
    }
  }
  delete[] data;
  data_size = n16x16 * 64 + n8x8 * 32 + n8x8b * 32;
  data = new BYTE[data_size];
  if (!data) {
    return false;
  }
  p = data;
  // Set the position back
  cart.Seek(addr + (start8x8 * 32));
  if (!cart.Read(data, data_size)) {
    delete[] data;
    return false;
  }
  if (n8x8 != 0) {
    for (i = 0; i < n8x8; i++) {
      POINT pos = pos8x8[i];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0, pos.x, pos.y,
        p, false, false);
      p += 32;
    }
  }
  if (n16x16 != 0) {
    for (i = 0; i < n16x16; i++) {
      POINT pos = pos16x16[i];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0, pos.x, pos.y + 8,
        p, false, false);
      DrawTile8x8(bm, palette, 0, pos.x + 8, pos.y + 8,
        p + 32, false, false);
      p += 64;
    }
  }
  if (n8x8b != 0) {
    for (i = 0; i < n8x8b; i++) {
      POINT pos = pos8x8b[i];
      pos.x -= base_.left;
      pos.y -= base_.top;
      DrawTile8x8(bm, palette, 0,
        pos.x, pos.y, p, false, false);
      p += 32;
    }
  }
  delete[] data;
#endif
  return true;
}

bool SpriteTile::Draw(HDC hdc, int to_x, int to_y,
  int width, int height, int from_x, int from_y)
{
  return BitmapSurface::Draw(hdc,
    base_.left + to_x,
    base_.top + to_y,
    width, height, from_x, from_y);
}

bool SpriteTile::Draw(BitmapSurface& bm, int to_x,
    int to_y, int from_x, int from_y)
{
  return BitmapSurface::Draw(bm,
    to_x,
    to_y,
    from_x, from_y);
}

bool SpriteTile::DrawFlipped(BitmapSurface& bm, int to_x, int to_y,
    int from_x, int from_y,
    bool flip_horz, bool flip_vert
)
{
  return BitmapSurface::DrawFlipped(bm,
    base_.left + to_x,
    base_.top + to_y,
    0, 0,
    flip_horz, flip_vert
  );
}


bool SpriteTile::IntersectBox(RECT& box)
{
  RECT dstRect;
  return IntersectRect(&dstRect, &base_, &box) != FALSE;
}

bool SpriteTile::IntersectPoint(POINT& pt)
{
  return base_.left <= pt.x && base_.right >= pt.x &&
    base_.top <= pt.y && base_.bottom >= pt.y;
}


const RECT& SpriteTile::GetRect() const
{
  return base_;
}

SpritePool::SpritePool(CartFile& cart)
  : cart_(cart)
{
}

bool SpritePool::OnResourceAlloc(DWORD id, SpriteResource* sprite)
{
  bool result;
  HDC hdc = GetDC(NULL);
  result = sprite->Load(cart_, hdc, LOWORD(id), HIWORD(id));
  DeleteDC(hdc);
  return result;
}
