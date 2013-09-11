
#include "stdafx.h"

#include "level_tilemap.h"
#include "level_properties.h"
#include "level_style.h"
#include "rom_file.h"
#include "tile.h"

int RareDecompress(Buffer& dst, const Buffer& src);

bool LevelTilemap::Load(CartFile& cart, const LevelProperties& prop,
    const LevelStyle& style)
{
  CartFile::Scope scope(cart);

  DWORD map_id = style.map_id_;
  DWORD map_index = prop.map_index_;

  if (!cart.Read(0x35BC54, &tilemap_type_, map_id * 2)) {
    return false;
  }

  /* Load tile graphics */
  if (!cart.FollowPtr3(0x35BB2E, map_id * 3)) {
    return false;
  }
  Buffer tiledata(false);
  int tiledata_size;
  if (!tiledata.Alloc(32768)) {
    return false;
  }
  tiledata_size = RareDecompress(tiledata, cart);
  if (tiledata_size <= 0 || (tiledata_size % 32)) {
    return false;
  }
  tiledata.WriteDebug("gfx3");

  {
    /* Graphics header */

    if (!cart.FollowPtr2(0x3D819A, style.graphics_ * 2)) {
      return false;
    }

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

    UINT tile_count;
    int gfx_size;

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
      gfx_data.MemSet(0, size1 - 0x4840);
      cart.Seek(addr1 + 0x4840);
      if (addr2 != 0) {
        gfx_data.Write(0x4840, cart, size2);
      }
    }
    if (gfx_size % 32 != 0) {
      return false;
    }
    tile_count = gfx_size / 32;

    TilePalette palette;
    DWORD def_palette;
    def_palette = style.palette_;
    if (!def_palette) {
      if (!cart.Read(0x35BC2A + map_id * 2, &def_palette, 2)) {
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

  }
  if (!cart.FollowPtr2(0x35BC7E, map_id * 2, 0x35)) {
    return false;
  }
  if (!cart.ReadAddr(tilemap_addr_, 0x35BAEF + map_id * 3)) {
    return false;
  }
  if (!cart.Readf("HH", &map_x_, &map_y_)) {
    return false;
  }
  cart.Skip(map_index * 10);
  if (!cart.Readf("HHHHH",
    &map_left_, &map_right_,
    &map_top_, &map_bottom_,
    &map_code_))
  {
    return false;
  }
  WORD cx, cy;
  map_left_ /= 32, map_right_ /= 32,
  map_top_ /= 32, map_bottom_ /= 32;
  cx = map_right_ - map_left_,
  cy = map_bottom_ - map_top_;

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
  bool vert = (map_code_ & 0x40) ? true : false;

  for (int x = 0, ix = map_left_; ix < map_right_; x++, ix++)
  {
    for (int y = 0, iy = map_top_; iy < map_bottom_; y++, iy++)
    {

    }
  }
  return true;
}
