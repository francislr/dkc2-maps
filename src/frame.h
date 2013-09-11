
#pragma once

#include "window.h"
#include "dialog.h"
#include "rom_file.h"
#include "editor.h"
#include "hex_editor.h"

class Editor;

class MainFrame : public Window
{
public:
  MainFrame(const TCHAR *cmd_line);
  ~MainFrame();

  bool Create(LPCSTR lpWindowName, int x, int y,
    int width, int height, Window *parent);

  /* Creates the menu */
  bool InitMenu();

  bool InitToolbar();
  bool InitStatus();

  bool OpenFile(bool new_file, bool force_close);
  bool SaveFile();
  bool CloseFile();

  void SetEditor(Editor *editor);
  void DestroyEditor();

  virtual void OnCreate();

  virtual void OnCommand(DWORD id, DWORD code);
  virtual bool OnEraseBackground(HDC hdc);

  /* Main frame has been resized */
  virtual void OnSize(WORD width, WORD height);

  /* Receives window messages */
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  /* Called when a file has been opened and loaded into memory*/
  void OnFileOpen();

  /* Called when a file has been closed */
  bool OnFileClose();

  /* Adds multiple menu to 'hMenu' at position */
  void AddMenus(const MENUDEF *menus, UINT position);

  /* Removes multiple menu to 'hMenu' at position */
  void RemoveMenus(const MENUDEF *menus, UINT position);

  /* Adds multiple buttons to the toolbar */
  bool AddButtons(DWORD count, LPTBBUTTON buttons);

  void RemoveButtons(DWORD offset, DWORD count);

  /* Registers the frame window class */
  static bool Register();

private:
  EditorHost editor_host_;

  Editor *editor_;

  /* Path of the actual opened file */
  std::string cart_path_;

  ToolbarWindow toolbar_;
  StatusWindow status_;

  /* ROM Cartridge file management */
  CartFile cart_;

  /* Handle for the window menu */
  HMENU menu_;

  /* Change the path by using the default file picker window */
  PickFileWindow file_picker_;

  /* Hex editor window */
  HexEditor hex_editor_;

  /* Class name of the window class */
  static const TCHAR *ClassName;

  /* File extensions filter used by the file picker */
  static const TCHAR *FileExtensions;

  /* Default extensions filter selected */
  static const TCHAR *DefExt;

  /* Command line supplied to the application */
  const TCHAR *cmd_line_;
};

/**
 * Dialog box to choose the emulator path
 */
class SetupEmuDialog : public Dialog
{
public:
  SetupEmuDialog(std::string& path);
  virtual void OnCreate();

  virtual void OnOK();

private:
  std::string& path_;
};
