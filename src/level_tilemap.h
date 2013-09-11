
#pragma once

#include "bitmap.h"

class CartFile;
class LevelProperties;
class LevelStyle;

class LevelTileParts
{
public:
  bool Load(CartFile& cart, const LevelProperties& prop,
    const LevelStyle& style);

};

class LevelTilemap :
  public BitmapSurface
{
public:
  bool Load(CartFile& cart, const LevelProperties& prop,
    const LevelStyle& style
  );

  DWORD tilemap_addr_;
  WORD tilemap_type_;
  WORD map_x_,
       map_y_;
  WORD map_left_,
       map_top_,
       map_right_,
       map_bottom_;
  WORD map_code_;
};
