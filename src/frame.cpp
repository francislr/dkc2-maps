
#include "stdafx.h"
#include <afxres.h>
#include <string>
#include <tchar.h>

#include "resource.h"
#include "frame.h"
#include "dialog.h"
#include "editor.h"

#include "level.h"

/**
 * Menus to add to the main menu
 * when a cartridge is opened.
 */
const MENUDEF kCartMenus[] = {
  { MAKEINTRESOURCE(IDR_MENUEDITORS), "Editor" },
  { NULL, NULL },
};

/* Position to insert the menu */
const UINT kCartMenuOffset = 1;

const TCHAR *MainFrame::ClassName = TEXT("MainFrame");

const TCHAR *MainFrame::FileExtensions = TEXT("" \
  "Super Nintendo ROM Cartridge\0" "*.smc\0" \
  "All Files\0" "*\0" \
  "\0" // Sentinel
);

const TCHAR *MainFrame::DefExt = TEXT(".smc");


MainFrame::MainFrame(const TCHAR* cmd_line) :
  file_picker_(cart_path_, FileExtensions, DefExt),
  editor_host_(*this), cmd_line_(cmd_line), hex_editor_(cart_)
{
  menu_ = NULL;
  HexEditor::Register();
}

MainFrame::~MainFrame()
{
}

bool MainFrame::Create(LPCSTR lpWindowName, int x, int y,
  int width, int height, Window *parent)
{
  if (!InitMenu()) {
    return false;
  }
  DWORD dwExStyles = WS_EX_WINDOWEDGE;
  DWORD dwStyles = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
  if (!Window::Create(ClassName, lpWindowName, dwExStyles, dwStyles,
    x, y, width, height, menu_, parent)) {
    return false;
  }
  if (!editor_host_.Create(NULL, 0, 0, width, height, this)) {
    return false;
  }
  if (!InitToolbar() || !InitStatus()) {
    Destroy();
    return false;
  }
  OnSize(width, height);
  //cart_.Open("test", error);
  //std::string test = "C:\\dev\\";

  Show();

  if (_tcslen(cmd_line_) > 0) {
    cart_path_ = cmd_line_;
    std::string::size_type k = 0;
    while((k = cart_path_.find('"', k)) != std::string::npos) {
      cart_path_.erase(k, 1);
    }
    OpenFile(false, false);
  }
  return true;
}

bool MainFrame::InitMenu()
{
  if (menu_ != NULL) {
    // already initialized
    return true;
  }
  menu_ = LoadMenu(kInstance, MAKEINTRESOURCE(IDR_MAINMENU));
  return menu_ != NULL;
}

bool MainFrame::InitToolbar()
{
  if (!toolbar_.Create(TBSTYLE_EX_DOUBLEBUFFER,
    CCS_TOP | TBSTYLE_TOOLTIPS,
    IDR_TOOLBAR, *this)) {
    return false;
  }
  toolbar_.SetMaxTextRows(0);
  HIMAGELIST hToolImg, hDisabledImg;
  hToolImg = ImageList_LoadImage(kInstance,
    MAKEINTRESOURCE(IDB_TOOLBAR),
    16,
    4,
    RGB(255, 255, 255),
    IMAGE_BITMAP,
    LR_CREATEDIBSECTION
  );
  hDisabledImg = ImageList_LoadImage(kInstance,
    MAKEINTRESOURCE(IDB_TOOLBARDISABLED),
    16,
    4,
    RGB(255, 255, 255),
    IMAGE_BITMAP,
    LR_CREATEDIBSECTION
  );
  toolbar_.SetImgList(hToolImg, hDisabledImg);

  TBBUTTON buttons[] = {
    { 0, ID_FILE_OPEN, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
      (UINT_PTR) TEXT("Open a ROM") },
    { 1, ID_FILE_SAVE, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, 0,
      (UINT_PTR) TEXT("Save the ROM") },
    { 0, 0, 0, TBSTYLE_SEP, 0, 0, 0, 0 },
  };
  toolbar_.AddButtons(ARRAY_SIZE(buttons), buttons);
  toolbar_.Show();
  return true;
}

bool MainFrame::InitStatus()
{
  if (!status_.Create(NULL, CCS_BOTTOM | SBARS_SIZEGRIP | WS_VISIBLE,
    IDR_STATUS, *this)) {
    return false;
  }
  int parts[] = { 0, 0, -1 };
  status_.SetParts(3, parts);
  return true;
}

bool MainFrame::OpenFile(bool new_file, bool force_close)
{
  if (!CloseFile()) {
    if (!force_close) {
      return false;
    }
  }
  if (new_file) {
    if (!file_picker_.Open(TEXT("Open a file"), this)) {
      return false;
    }
  }
  std::string error;
  if (!cart_.Open(cart_path_, error)) {
    MessageBox(
      GetHWND(),
      error.c_str(),
      TEXT("Unable to open the cartridge image"),
      MB_ICONERROR
    );
    return false;
  }
  OnFileOpen();
  return true;
}

bool MainFrame::SaveFile()
{
  if (editor_) {
    if (!editor_->SaveFile()) {
      return false;
    }
  }
  std::string error;
  if (!cart_.Save(cart_path_, error)) {
    MessageBox(
      GetHWND(),
      error.c_str(),
      TEXT("Unable to save the cartridge image"),
      MB_ICONERROR
    );
    return false;
  }
  return true;
}

bool MainFrame::CloseFile()
{
  /*int answer = MessageBox(GetHWND(),
    TEXT("Would you like to save the modifications to the ROM cartridge ?"),
    TEXT("Exit"),
    MB_YESNOCANCEL | MB_ICONQUESTION
  );
  if (answer == IDCANCEL) {
    return false;
  }*/
  DestroyEditor();
  hex_editor_.Destroy();
  OnFileClose();
  cart_.Release();
  return true;
}

void MainFrame::SetEditor(Editor *editor)
{
  DestroyEditor();
  if (editor) {
    editor_host_.SetPanel(editor);
    editor_ = editor;
  }
}

void MainFrame::DestroyEditor()
{
  editor_ = NULL;
  editor_host_.DestroyPanel();
  editor_host_.Invalidate(NULL);
  editor_host_.Update();
}

void MainFrame::OnCreate()
{
}

void MainFrame::OnCommand(DWORD id, DWORD code)
{
  switch (id) {
  case ID_FILE_OPEN:
    OpenFile(true, false);
    break ;
  case ID_FILE_SAVE:
    SaveFile();
    break ;
  case ID_FILE_CLOSE:
    CloseFile();
    break ;
  case ID_FILE_HEXEDITOR:
    if (cart_.IsValid()) {
      hex_editor_.Create("Hex Editor",
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 500, this);
    }
    break ;
  case ID_EDITOR_LEVEL:
    SetEditor(new LevelEditor(*this, cart_));
    break ;
  }
  editor_host_.OnCommand(id, code);
}

bool MainFrame::OnEraseBackground(HDC hdc)
{
  return false;
}

void MainFrame::OnSize(WORD width, WORD height)
{
  toolbar_.AutoSize();
  status_.AutoSize();
  RECT tbRect, sRect;
  toolbar_.GetRect(tbRect);
  status_.GetRect(sRect);
  int bottom = sRect.top - sRect.bottom - tbRect.bottom;
  editor_host_.SetPosition(0, tbRect.bottom + 2, width, height + bottom - 2);
  Window::OnSize(width, height);
}

LRESULT MainFrame::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return Window::OnMessage(msg, wParam, lParam);
}

void MainFrame::OnFileOpen()
{
  AddMenus(kCartMenus, kCartMenuOffset);
  SetEditor(new LevelEditor(*this, cart_));
}

bool MainFrame::OnFileClose()
{
  RemoveMenus(kCartMenus, kCartMenuOffset);
  return true;
}

void MainFrame::AddMenus(const MENUDEF *menus, UINT position)
{
  MENUITEMINFO mii = {};
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STRING | MIIM_SUBMENU;
  while (menus->resource != NULL) {
    HMENU hSubMenu = LoadMenu(kInstance, menus->resource);
    mii.hSubMenu = hSubMenu;
    mii.dwTypeData = menus->name;
    InsertMenuItem(menu_, position, TRUE, &mii);
    menus++;
  }
  DrawMenuBar(hwnd_);
}

void MainFrame::RemoveMenus(const MENUDEF *menus,
  UINT position)
{
  while (menus->resource != NULL) {
    RemoveMenu(menu_, position, MF_BYPOSITION);
    menus++;
  }
  DrawMenuBar(hwnd_);
}

bool MainFrame::AddButtons(DWORD count, LPTBBUTTON buttons)
{
  return toolbar_.AddButtons(count, buttons);
}

void MainFrame::RemoveButtons(DWORD offset, DWORD count)
{
  for (UINT i = 0; i < count; i++) {
    toolbar_.DeleteButton(offset);
  }
}

bool MainFrame::Register()
{
  EditorHost::Register();
  HICON hAppIcon = LoadIcon(kInstance, MAKEINTRESOURCE(IDI_APPICON));
  return Window::Register(ClassName, (HBRUSH) COLOR_WINDOW + 1, hAppIcon);
}

SetupEmuDialog::SetupEmuDialog(std::string& path) : path_(path)
{
}

void SetupEmuDialog::OnCreate()
{
  SetItemText(ID_EMULATOR_PATH, path_);
}

void SetupEmuDialog::OnOK()
{
  GetItemText(ID_EMULATOR_PATH, path_);
  Dialog::OnOK();
}
