
#pragma once

#include <vector>

class CartFile;

class LevelProperties
{
public:
  LevelProperties();
  ~LevelProperties();

  bool Load(CartFile& cart, UINT level_id);

  typedef struct {
    BYTE p;
    WORD x;
    WORD y;
  } Entry;

  WORD type_;
  BYTE bonus_type_;
  WORD style_;
  BYTE map_index_;
  BYTE param_[6];
  std::vector<Entry> entries_;
  std::vector<WORD> exits_;
};
