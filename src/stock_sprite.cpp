
#include "stdafx.h"

#include "stock_sprite.h"
#include "level_sprite.h"
#include "sprite_property.h"
#include "rom_file.h"

bool StockSprite::Load(CartFile& cart)
{
  // 0x00F540
  CartFile::Scope scope(cart);

  cart.Seek(0x00F540);
  SpritePropertyPool props(cart);

  for (UINT i = 0; i < 64; i++) {
    WORD string_id, cast_type, prop_id;
    if (!cart.Readf("HHH",
      &string_id, &cast_type, &prop_id))
    {
      return false;
    }
    if (cast_type == 0xffff) {
      break ;
    }
    if (cast_type != 0x0140) {
      // not a character presentation
      continue ;
    }
    SceneCast c;
    DWORD addr = 0x000000;
    if (!cart.Read(0x00F6CE + string_id * 2, &addr, 2)) {
      return false;
    }
    if (!cart.ReadString(addr, c.name)) {
      return false;
    }
    Ref<SpriteProperty> prop = props.NewFromID2(prop_id);
    if (!prop) {
      return false;
    }
    SpriteGraphicDef gfx;
    if (!prop->GetGraphics(cart, gfx)) {
      continue ;
    }
    c.animation_id = gfx.anim_id;
    c.palette_id = gfx.palette_id;
    c.sprite_id = gfx.sprite_id;
    characters.push_back(c);
  }
  return true;
}

bool StockSprite::GetName(std::string& in_name,
  WORD animation_id, WORD sprite_id)
{
  std::vector<SceneCast>::iterator i;
  for (i = characters.begin(); i != characters.end(); ++i) {
    if ((animation_id != 0 && i->animation_id == animation_id) ||
        (sprite_id != 0 && i->sprite_id == sprite_id))
    {
      in_name = i->name;
      return true;
    }
  }
  return false;
}

bool StockSprite::GetName(std::string& in_name,
    Ref<SpriteProperty> prop)
{
  SpriteGraphicDef gfx;
//  if (!prop->GetGraphics(gfx))
//  {
//    return false;
//  }
  return GetName(in_name, gfx.anim_id, gfx.sprite_id);
}
