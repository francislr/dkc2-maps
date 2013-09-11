
#pragma once

#include <vector>

#include "window.h"
#include "ref_count.h"
#include "animation.h"
#include "editor_tool.h"
#include "stock_sprite.h"
#include "sprite_property.h"

class CartFile;
class LevelEditor;
class SpritePropertyPool;
class SpriteProperty;
class MainFrame;

class LevelSprite : public EditorObject, public Animation
{
public:
  LevelSprite(SpritePropertyPool& prop_pool, SpritePool& sprite_pool);
  ~LevelSprite();

  bool Load(CartFile& cart);
  bool Serialize(Buffer& dest_buf);
  bool Deserialize(Buffer& src_buf);
  virtual bool Draw(BitmapSurface& surface, const RECT& pos);
  virtual bool IntersectBox(RECT& box);
  virtual bool IntersectPoint(POINT& pt);
  virtual void GetRect(RECT& rect);
  virtual bool Move(int x, int y);
  bool NextFrame(RedrawRegion& redraw);
  Ref<SpriteProperty> GetProperty();

  WORD param_;
  WORD x_,
       y_;
  WORD prop_id_;
  bool valid_;
private:
  SpritePropertyPool& prop_pool_;
  SpritePool& sprite_pool_;
  Ref<SpriteProperty> property_;
};

class LevelSpriteManager :
  public EditorObjectManager<LevelSprite>
{
public:
  LevelSpriteManager();
  ~LevelSpriteManager();

  bool Load(CartFile& cart, SpritePool& sprites,
    SpritePropertyPool& properties, UINT level_id);
  bool Save(CartFile& cart);
  bool NextFrame(const RECT& pos, RedrawRegion& redraw);

private:
  DWORD level_id_;
};

class SpritePropWindow :
  public Window
{
public:
  SpritePropWindow(CartFile& cart, LevelEditor& level_editor);
  ~SpritePropWindow();

  bool Create(LPCSTR lpWindowName, int x, int y,
    int width, int height);
  virtual void OnCreate();
  void OnSize(WORD width, WORD height);
  virtual bool OnDrawItem(DWORD id, DRAWITEMSTRUCT* item);
  virtual bool OnMesureItem(DWORD id, MEASUREITEMSTRUCT* item);

  static bool Register();
  static const TCHAR *ClassName;

private:
  CartFile& cart_;
  LevelEditor& level_editor_;
  StockSprite stock_sprite_;
  ListBox sprite_list_;
};

class LevelSpriteEditor :
  public EditorTool<LevelSprite>
{
public:
  LevelSpriteEditor(EditorObjectManager<LevelSprite>& objects,
    CartFile& cart, LevelEditor& level_editor, MainFrame& frame);
  ~LevelSpriteEditor();

  virtual void OnCreate();
  virtual bool OnDestroy();
  virtual void OnMouseDoubleClick(DWORD key, int x, int y);
  void ShowProperty();

  virtual void OnCommand(DWORD id, DWORD code);

private:
  CartFile& cart_;
  LevelEditor& level_editor_;
  SpritePropWindow sprite_window_;
  EditPropWindow prop_window_;
  MainFrame& frame_;
};
