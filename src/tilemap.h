
#pragma once

#include <map>

#include "bitmap.h"

class CartFile;
class LevelProperties;
class LevelStyle;
class TilePalette;

class TileParts :
  public BitmapSurface
{
public:
  TileParts();
  ~TileParts();

  bool Load(CartFile& cart, HDC hdc, const LevelProperties& prop,
    const LevelStyle& style);

  UINT tile_count_;
};

class Tile
{
public:
  DWORD part_id;
  bool flip_horz,
       flip_vert;
};

class TileMap
{
public:
  TileMap();
  ~TileMap();

  bool Load(CartFile& cart, const LevelProperties& prop,
    const LevelStyle& style);
  bool Draw(BitmapSurface& bm, TileParts& parts,
    const RECT& clip);

  DWORD tilemap_addr_;
  WORD tilemap_type_;
  WORD map_x_,
       map_y_;
  WORD map_width_,
       map_height_;
  WORD map_left_,
       map_top_,
       map_right_,
       map_bottom_;
  WORD map_code_;
  DWORD map_id_,
        map_index_;

  typedef std::map<DWORD, Tile> Map;
  Map map_;
};
