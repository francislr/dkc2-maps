
#include "stdafx.h"

#include "level_properties.h"
#include "rom_file.h"

LevelProperties::LevelProperties()
{
}

LevelProperties::~LevelProperties()
{
}

bool LevelProperties::Load(CartFile& cart, UINT level_id)
{
  CartFile::Scope scope(cart);
  if (!cart.FollowPtr2(0x3D0000, level_id * 2)) {
    return false;
  }
  if (!cart.ReadWord(type_)) {
    return false;
  }
  if (type_ == 0x0004 || type_ == 0x0005) {
    return true;
  }
  if (type_ == 0x0001) {
    if (!cart.ReadByte(bonus_type_)) {
      return false;
    }
  }
  if (!cart.Readf("HBBBBBBB",
    &style_, &map_index_, &param_[0],
    &param_[1], &param_[2], &param_[3],
    &param_[4], &param_[5]))
  {
    return false;
  }
  entries_.clear();
  while (entries_.size() < 128) {
    /*
     Known flags
      0x0002 - Look at left
      0x0030
      0x00C0
    */
    BYTE p;
    if (!cart.ReadByte(p)) {
      return false;
    }
    if (p == 0xff) {
      break ;
    }
    Entry entry;
    entry.p = p;
    if (!cart.Readf("HH",
      &entry.x, &entry.y))
    {
      return false;
    }
    entries_.push_back(entry);
  }
  exits_.clear();
  while (exits_.size() < 128) {
    WORD p;
    if (!cart.ReadWord(p)) {
      return false;
    }
    if (p == 0xffff) {
      break ;
    }
    exits_.push_back(p);
  }
  return true;
}
