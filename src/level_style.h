
#pragma once

class CartFile;

class LevelStyle
{
public:
  LevelStyle();
  ~LevelStyle();

  bool Load(CartFile& cart, DWORD addr);

  WORD routine1_;
  WORD routine2_;

  BYTE music_;
  WORD palette_;
  BYTE fg_scroll_;
  BYTE graphics_;
  BYTE map_id_;
};
