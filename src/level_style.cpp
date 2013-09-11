
#include "stdafx.h"

#include "level_style.h"
#include "rom_file.h"

LevelStyle::LevelStyle()
{
}

LevelStyle::~LevelStyle()
{
}

bool LevelStyle::Load(CartFile& cart, DWORD addr)
{
  CartFile::Scope scope(cart);
  addr |= 0x3D0000; // Bank
  cart.Seek(addr);
  if (!cart.Readf("HHBBHHHBB",
    &routine1_, // 0x00
    &routine2_, // 0x02
    &music_,    // 0x04
    NULL,       // 0x05
    &palette_,  // 0x06
    NULL,       // 0x08
    NULL,       // 0x0A
    &fg_scroll_,// 0x0C
    &graphics_  // 0x0D
  )) {
    return false;
  }
  cart.Skip(4);
  if (!cart.ReadByte(map_id_)) {
    return false;
  }

  return true;
}

