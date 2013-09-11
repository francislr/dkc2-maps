/**
 * Uses the "character cast" to retreive
 * the sprites name
 */

#pragma once

#include "ref_count.h"
#include <string>
#include <vector>

class CartFile;
class SpriteProperty;

class StockSprite
{
public:
  bool Load(CartFile& cart);

  typedef struct {
    WORD palette_id;
    WORD animation_id;
    WORD sprite_id;
    std::string name;
  } SceneCast;

  bool GetName(std::string& in_name,
    WORD animation_id, WORD sprite_id);
  bool GetName(std::string& in_name,
    Ref<SpriteProperty> prop);

  std::vector<SceneCast> characters;
};
