
#include "stdafx.h"
#include <windowsx.h>

#include "resource.h"
#include "tile.h"
#include "level.h"
#include "editor_tool.h"

const MENUDEF kLevelMenus[] = {
  { MAKEINTRESOURCE(IDR_MENU_LEVEL), "Level" },
  { MAKEINTRESOURCE(IDR_LEVEL_EDIT), "Edit" },
  { NULL, NULL },
};

const UINT kLevelMenuOffset = 2;

TBBUTTON kLevelButtons[] = {
  { 2, ID_EDIT_SPRITE, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
    (UINT_PTR) TEXT("Edit Sprite") },
  { 3, ID_EDIT_BACKGROUND, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
    (UINT_PTR) TEXT("Edit Background") },
  { 4, ID_EDIT_BANANA, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
    (UINT_PTR) TEXT("Edit Banana") },
  { 0, 0, 0, TBSTYLE_SEP, 0, 0, 0, 0 },
  { 5, ID_EDIT_UNDO, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
    (UINT_PTR) TEXT("Undo") },
  { 6, ID_EDIT_REDO, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
    (UINT_PTR) TEXT("Redo") },
};


LevelEditor::LevelEditor(MainFrame& frame, CartFile& cart) :
  ScrollSurface(frame, cart), selection_(*this), dragdrop_(*this),
  sprites_(cart), level_properties_(cart)
{
  SpritePropWindow::Register();
  EditPropWindow::Register();
  level_id_ = 0x03;
}

LevelEditor::~LevelEditor()
{
  BitmapSurface::Destroy();
}

void LevelEditor::OnCreate()
{
//  kMainAccel = LoadAccelerators(kInstance, MAKEINTRESOURCE(IDR_EDITORACCL));
  HDC hdc;
  RECT rect;
  hdc = GetDC();
  Window::GetRect(rect);
  CreateDC(hdc);
  CreateBM(hdc, rect.right, rect.bottom);
  ScrollSurface::OnCreate();
  SetTimer(hwnd_, 1, 30, NULL);
  frame_.AddMenus(kLevelMenus, kLevelMenuOffset);

  frame_.AddButtons(ARRAY_SIZE(kLevelButtons), kLevelButtons);

  ReleaseDC(hwnd_, hdc);
  LoadLevel(level_id_);
}

void LevelEditor::OnCommand(DWORD id, DWORD code)
{
  switch (id)
  {
  case ID_LEVEL_CHANGE:
    PickLevel();
    break ;
  case ID_LEVEL_NEXT:
    if (level_id_ >= 0xC0) {
      break ;
    }
    LoadLevel(level_id_ + 1);
    break ;
  case ID_LEVEL_PREVIOUS:
    if (level_id_ <= 0x01) {
      break ;
    }
    LoadLevel(level_id_ - 1);
    break ;
  }
  Editor::OnCommand(id, code);
}

bool LevelEditor::OnDestroy()
{
  DestroyPanel();
  KillTimer(hwnd_, 1);
  frame_.RemoveMenus(kLevelMenus, kLevelMenuOffset);
  frame_.RemoveButtons(2, ARRAY_SIZE(kLevelButtons) + 1);
  kMainAccel = NULL;
  return ScrollSurface::OnDestroy();
}

void LevelEditor::OnPaintArea(const RECT& pos, HDC hdc)
{

  tile_map_.Draw(*this, tile_parts_, pos);
  level_sprites_.Draw(*this, pos);
  Window::OnPaintArea(pos, hdc);
}

LRESULT LevelEditor::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_TIMER:
    NextFrame();
    return 0;
  }
  return ScrollSurface::OnMessage(msg, wParam, lParam);
}

bool LevelEditor::PickLevel()
{
  DWORD level_id = level_id_;
  LevelPickerDialog level_picker(level_id);
  if (level_picker.CreateModal(IDD_LEVELPICKER, *this) != -1) {
    LoadLevel(level_id);
  }
  return true;
}

bool LevelEditor::LoadLevel(DWORD level_id)
{
  level_id_ = level_id;
  HDC hdc = GetDC();

  ScrollSurface::Redraw redraw(*this);
  redraw.Invalidate(NULL);
  if (!level_sprites_.Load(cart_, sprites_, level_properties_, level_id)) {
    return false;
  }
  if (!level_style_.Load(cart_, level_id)) {
    return false;
  }

  const RECT& scroll_rect = GetScrollRect();
  level_sprites_.InvalidateAll(redraw);

  LevelStyle style;
  style.Load(cart_, level_style_.style_);

  if (!tile_parts_.Load(cart_, hdc, level_style_, style)) {
    return false;
  }
  if (!tile_map_.Load(cart_, level_style_, style)) {
    return false;
  }

  SetDimensions(tile_map_.map_width_, tile_map_.map_height_);

  SetPanel(new LevelSpriteEditor(level_sprites_, cart_, *this, frame_));
  ReleaseDC(hwnd_, hdc);

  return true;
}

bool LevelEditor::SaveFile()
{
  if (!level_sprites_.Save(cart_)) {
    return false;
  }
  return true;
}

void LevelEditor::NextFrame()
{
  ScrollSurface::Redraw redraw(*this);
  const RECT& pos = GetScrollRect();
  level_sprites_.NextFrame(pos, redraw);
  //Invalidate(NULL);
  //Redraw();
}

SpritePropertyPool& LevelEditor::GetSpriteProps()
{
  return level_properties_;
}

SpritePool& LevelEditor::GetSprites()
{
  return sprites_;
}

LevelPickerDialog::LevelPickerDialog(DWORD& level_id) :
  level_id_(level_id)
{

}

void LevelPickerDialog::OnCreate()
{
  GetItem(IDC_LEVEL_LIST, level_list_);
  level_list_.SetRedraw(false);
  TCHAR level_name[128];
  for (int i = 0; i < 192; i++) {
    const TCHAR *name = kLevelsName[i];
    if (name == NULL) {
      name = kLevelDefault;
    }
    if (_stprintf_s(level_name, 128, "%02X: %s", i, name)) {
      int index = level_list_.AddString(level_name);
      level_list_.SetItemData(index, i);
    }
  }
  SetLevelID(level_id_);
  level_list_.SetRedraw(true);
  level_list_.Redraw();
}

void LevelPickerDialog::OnCommand(DWORD id, DWORD code)
{
  switch (id) {
  case IDC_LEVEL_LIST:
    if (code == LBN_SELCHANGE) {
      int index = level_list_.GetSelIndex();
      if (index != -1) {
        DWORD level_id = level_list_.GetItemData(index);
        SetLevelID(level_id);
      }
    }
    else if (code == LBN_DBLCLK) {
      OnOK();
    }
    break ;
  case IDC_LEVEL_ID:
    if (code == EN_KILLFOCUS) {
      Window level_id_edit;
      GetItem(IDC_LEVEL_ID, level_id_edit);
      TCHAR id_str[12];
      if (level_id_edit.GetText(id_str, 12) > 0) {
        DWORD level_id;
        if (_stscanf_s(id_str, "%02X", &level_id) == 1) {
          SetLevelID(level_id);
        }
      }
    }
    break ;
  }
}

INT_PTR LevelPickerDialog::DialogMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  return Dialog::DialogMessage(msg, wParam, lParam);
}

void LevelPickerDialog::SetLevelID(DWORD level_id)
{
  level_id_selected_ = level_id;
  TCHAR level_id_str[32];
  if (_stprintf_s(level_id_str, 32, "%02X", level_id_selected_)) {
    SetItemText(IDC_LEVEL_ID, level_id_str);
  }
  level_list_.SetSel(level_id);
}

void LevelPickerDialog::OnOK()
{
  level_id_ = level_id_selected_;
  Dialog::OnOK();
}
