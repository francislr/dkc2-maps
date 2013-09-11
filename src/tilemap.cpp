
#include "stdafx.h"

#include "tilemap.h"
#include "level_properties.h"
#include "level_style.h"
#include "rom_file.h"
#include "tile.h"

int RareDecompress(Buffer& dst, const Buffer& src);

TileParts::TileParts()
{
  tile_count_ = 0;
}

TileParts::~TileParts()
{
}

bool TileParts::Load(CartFile& cart, HDC hdc,
  const LevelProperties& prop, const LevelStyle& style)
{
  CartFile::Scope scope(cart);

  /* Load the position of the tile parts */

  if (!cart.FollowPtr3(0x35BB2E, style.map_id_ * 3)) {
    return false;
  }
  Buffer tile_map(false);
  int tile_map_size;
  if (!tile_map.Alloc(32768)) {
    return false;
  }
  tile_map_size = RareDecompress(tile_map, cart);
  if (tile_map_size <= 0 || (tile_map_size % 32)) {
    return false;
  }
  tile_count_ = tile_map_size / 32;
  if (!cart.FollowPtr2(0x3D819A, style.graphics_ * 2)) {
    return false;
  }

  /* Load the tile graphics */

  Buffer gfx_data(true);
  BYTE bank;
  WORD addr, op, size, size1, size2;
  UINT offset;
  DWORD addr1 = 0, addr2 = 0;
  bool compressed = false;

  while (1) {
    if (!cart.Readf("BHHH",
      &bank, &addr, &op, &size))
    {
      return false;
    }
    if (bank == 0) {
      break ;
    }
    offset = (bank << 16) | addr;
    if (offset <= 0xC00000) {
      return false;
    }
    offset -= 0xC00000;
    switch (op) {
    case 0xA000:
      addr1 = offset;
      compressed = true;
      break ;
    case 0x2000:
      addr1 = offset;
      compressed = false;
      size1 = size;
      break ;
    case 0x4420:
      addr2 = offset;
      compressed = false;
      size2 = size;
      break ;
    }
  }
  if (addr1 == 0) {
    return false;
  }
  cart.Seek(addr1);
  if (!gfx_data.Alloc(1024)) {
    return false;
  }

  int gfx_size;
  UINT gfx_count;

  if (compressed) {
    gfx_size = RareDecompress(gfx_data, cart);
    if (gfx_size <= 0) {
      return false;
    }
  }
  else {
    if (size1 >= 0x4840) {
      return false;
    }
    gfx_size = 0x4840 + size2;
    gfx_data.Write(0, cart, size1);
    if (addr2 != 0) {
      cart.Seek(addr2);
      gfx_data.Seek(0x4840);
      gfx_data.Write(0x4840, cart, size2);
    }
  }

  if (gfx_size % 32 != 0) {
    return false;
  }
  gfx_count = gfx_size / 32;

  TilePalette palette;
  DWORD def_palette;
  def_palette = style.palette_;
  if (!def_palette) {
    if (!cart.Read(0x35BC2A + style.map_id_ * 2, &def_palette, 2)) {
      return false;
    }
  }
  def_palette |= 0x3D0000;
  palette.LoadPalette(cart, def_palette);
  switch (style.routine1_) {
  case 0x00:
    palette.LoadSubPalette(cart, 0x3D1710, 0, 128);
    break ;
  case 0x01:
    palette.LoadSubPalette(cart, 0x3D268E, 16, 16);
    break ;
  case 0x02:
    palette.LoadPalette(cart, 0x3D2AEE);
    break ;
  case 0x03:
    palette.LoadPalette(cart, 0x3D29EE);
    break ;
  case 0x04:
    palette.LoadSubPalette(cart, 0x3D324E, 0, 128);
    break ;
  case 0x05:
    palette.LoadSubPalette(cart, 0x3D304E, 0, 128);
    break ;
  case 0x06:
    palette.LoadSubPalette(cart, 0x3D2EEE, 0, 128);
    break ;
  case 0x07:
    palette.LoadSubPalette(cart, 0x3D15F0, 12, 16);
    break ;
  case 0x08:
    palette.LoadSubPalette(cart, 0x3D0DD0, 0, 16);
    break ;
  case 0x0A:
    palette.LoadSubPalette(cart, 0x3D326E, 16, 16);
    break ;
  case 0x0C:
    palette.LoadSubPalette(cart, 0x3D2BEE, 0, 128);
    break ;
  case 0x0D:
    palette.LoadSubPalette(cart, 0x3D07F0, 0, 128);
    break ;
  case 0x0E:
    palette.LoadSubPalette(cart, 0x3D268E, 112, 16);
    break ;
  case 0x10:
    palette.LoadSubPalette(cart, 0x3D3A4E, 0, 128);
    break ;
  case 0x12:
    palette.LoadSubPalette(cart, 0x3D2DCE, 0, 16);
    break ;
  case 0x14:
    palette.LoadPalette(cart, 0x3D1610);
    break ;
  }
  // Set transparency
  for (UINT i = 0; i < 128; i += 16) {
    palette.SetTransparent(i, true);
  }
  if (!CreateDC(hdc) ||
    !CreateBM(hdc, tile_count_ * 32, 32) ||
    !CreateMask(hdc))
  {
    return false;
  }
  tile_map.Seek(0);
  for (UINT i = 0; i < tile_count_; i++) {
    for (UINT j = 0; j < 16; j++) {
      UINT x = ((j % 4) * 8) + (i * 32),
           y = (j / 4) * 8;
      WORD tile_info;
      tile_map.ReadWord(tile_info);
      WORD gfx_index = (tile_info & 0x03FF);
      BYTE pal_index = (tile_info & 0x1C00) >> 6;
      bool flip_horz = (tile_info & 0x4000) == 0x4000,
           flip_vert = (tile_info & 0x8000) == 0x8000;
      if (gfx_index >= gfx_count) {
        gfx_index = 0;
      }
      BYTE tile_data[32];
      gfx_data.Seek(gfx_index * 32);
      gfx_data.Read(tile_data, 32);
      DrawTile8x8(*this, palette,
        pal_index, x, y, tile_data,
        flip_horz, flip_vert
      );
    }
  }
  return true;
}

TileMap::TileMap()
{
}

TileMap::~TileMap()
{
}

bool TileMap::Load(CartFile& cart, const LevelProperties& prop,
    const LevelStyle& style)
{
  map_.clear();
  map_id_ = style.map_id_;
  map_index_ = prop.map_index_;

  if (!cart.Read(0x35BC54 +  map_id_ * 2, &tilemap_type_, 2)) {
    return false;
  }

  if (!cart.FollowPtr2(0x35BC7E, map_id_ * 2, 0x35)) {
    return false;
  }
  if (!cart.ReadAddr(tilemap_addr_, 0x35BAEF + map_id_ * 3)) {
    return false;
  }
  if (!cart.Readf("HH", &map_x_, &map_y_)) {
    return false;
  }
  cart.Skip(map_index_ * 10);
  if (!cart.Readf("HHHHH",
    &map_left_, &map_right_,
    &map_top_, &map_bottom_,
    &map_code_))
  {
    return false;
  }
  map_width_ = map_right_ - map_left_,
  map_height_ = map_bottom_ - map_top_;

  cart.Seek(tilemap_addr_);
  Buffer tilemap(true);
  int tilemap_size;

  if (tilemap_type_ & 0x0400) {
    tilemap.Alloc(1024);
    tilemap_size = RareDecompress(tilemap, cart);
    if (tilemap_size <= 0) {
      return false;
    }
  }
  else {
    tilemap_size = 256;
    tilemap.Write(0, cart, tilemap_size);
  }
  bool vert = false;
  switch (map_code_) {
  case 0x0004:
  case 0x0005:
  case 0x0006:
  case 0x0007:
    vert = true;
  }
  tilemap.Seek(0);
  for (int x = 0, ix = map_left_; ix < map_right_; x++, ix += 32)
  {
    for (int y = 0, iy = map_top_; iy < map_bottom_; y++, iy += 32)
    {
      Tile tile;
      WORD tile_info;
      int bx = ix,
          by = iy,
          bh = map_height_;
      DWORD index = MAKELONG(bx, by);
      if (vert) {
        tilemap.Seek(((bx / 32) + (by / 32) * (map_x_ / 32)) * 2);
        index = MAKELONG(bx, by);
      }
      else {
        tilemap.Seek(((bx / 32) * (map_y_ / 32)  + (by / 32)) * 2);
      }
      tilemap.ReadWord(tile_info);
      tile.flip_horz = (tile_info & 0x4000) == 0x4000;
      tile.flip_vert = (tile_info & 0x8000) == 0x8000;
      tile.part_id = (tile_info & 0x03FF);
      map_[index] = tile;
    }
  }
  return true;
}

bool TileMap::Draw(BitmapSurface& bm, TileParts& parts,
    const RECT& clip)
{
  if (map_.size() == 0) {
    return true;
  }
  int ix = -clip.left;
  ix -= ix % 32;
  for (; ix < clip.right; ix += 32)
  {
    int iy = -clip.top;
    iy -= iy % 32;
    for (; iy < clip.bottom; iy += 32)
    {
      int to_x = ix + clip.left,
          to_y = iy + clip.top;
      Map::iterator i;
      i = map_.find(MAKELONG(ix + map_left_, iy + map_top_));
      if (i == map_.end()) {
        break ;
      }
      Tile& tile = i->second;
      int src_x = tile.part_id * 32,
          src_y = 0;

      int tile_width = 32,
          tile_height = 32;
      if (tile.flip_horz) {
        tile_width *= -1;
        //src_x += 32;
      }
      if (tile.flip_vert) {
        tile_height *= -1;
        //src_y += 32;
      }
      parts.Draw(bm, to_x, to_y,
        tile_width, tile_height, src_x, src_y);
    }
  }
  return true;
}
