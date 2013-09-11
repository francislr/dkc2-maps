
#pragma once

#include "dialog.h"
#include "frame.h"
#include "editor.h"
#include "bitmap.h"
#include "sprite.h"
#include "tilemap.h"
#include "level_properties.h"
#include "level_style.h"
#include "level_tilemap.h"
#include "sprite_property.h"
#include "level_sprite.h"
#include "undo_buffer.h"

const extern TCHAR* kLevelDefault;
const extern TCHAR* kLevelsName[];

class LevelEditor :
  public ScrollSurface
{
public:
  LevelEditor(MainFrame& frame, CartFile& cart);
  ~LevelEditor();

  virtual void OnCreate();
  virtual void OnCommand(DWORD id, DWORD code);
  virtual bool OnDestroy();
  virtual void OnPaintArea(const RECT& pos, HDC hdc);

  void OnSelectionRelease(RECT& zone);
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  /* Shows the level picker and loads the level */
  bool PickLevel();

  bool LoadLevel(DWORD level_id);
  virtual bool SaveFile();

  /* Renders the next frame for animations */
  void NextFrame();

  SpritePropertyPool& GetSpriteProps();
  SpritePool& GetSprites();

private:
  DWORD level_id_;
  UndoBuffer undo_buffer_;

  SelectionMarquee selection_;
  DragDrop dragdrop_;

  /* Loaded graphic sprites */
  SpritePool sprites_;

  /* Level sprite */
  LevelSpriteManager level_sprites_;

  /* Level sprite properties */
  SpritePropertyPool level_properties_;

  /* Level parameters, entries & exits */
  LevelProperties level_style_;

  TileParts tile_parts_;
  TileMap tile_map_;
};

/**
 * Enables the user to pick a level from
 * a list of predifined name.
 */
class LevelPickerDialog : public Dialog
{
public:
  LevelPickerDialog(DWORD& level_id);
  virtual void OnCreate();
  virtual void OnCommand(DWORD id, DWORD code);
  virtual INT_PTR DialogMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  void SetLevelID(DWORD level_id);

  virtual void OnOK();

private:
  ListBox level_list_;
  DWORD& level_id_;
  DWORD level_id_selected_;
};
