
#pragma once

#include <vector>
#include <list>

#include "ref_count.h"
#include "tile.h"
#include "animation.h"

class CartFile;

class SpriteTile : public BitmapSurface
{
public:
  SpriteTile();
  virtual ~SpriteTile();

  bool Load(CartFile& cart, HDC hdc, UINT sprite_id, UINT pal_id);
  virtual bool Draw(HDC hdc, int to_x, int to_y,
    int width, int height, int from_x, int from_y);
  virtual bool Draw(BitmapSurface& bm, int to_x,
    int to_y, int from_x, int from_y);
  virtual bool DrawFlipped(BitmapSurface& bm, int to_x, int to_y,
    int from_x, int from_y,
    bool flip_horz, bool flip_vert
  );
  bool IntersectBox(RECT& box);
  bool IntersectPoint(POINT& pt);
  const RECT& GetRect() const;

private:
  UINT id_;
  UINT pal_id_;
  RECT base_;
};

class SpriteResource :
  public RefCount, public SpriteTile
{
public:
};

class SpritePool
  : public ResourcePool<SpriteResource>
{
public:
  SpritePool(CartFile& cart);

  /* Called when a sprite is allocated */
  virtual bool OnResourceAlloc(DWORD id, SpriteResource* resource);

private:
  CartFile& cart_;
};
