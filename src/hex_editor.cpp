
#include "stdafx.h"

#include "rom_file.h"
#include "hex_editor.h"

const TCHAR *HexEditor::ClassName = TEXT("RomEdit");

HexEditor::HexEditor(Buffer& buffer) : buffer_(buffer)
{
}

HexEditor::~HexEditor()
{
}

bool HexEditor::Create(LPCSTR lpWindowName, int x, int y,
  int width, int height, Window *parent)
{
  DWORD dwExStyles = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW;
  DWORD dwStyles = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_CLIPCHILDREN |
    WS_MAXIMIZEBOX;
  if (!Window::Create(ClassName, lpWindowName, dwExStyles, dwStyles,
    x, y, width, height, NULL, parent)) {
    return false;
  }
  RECT rcClient;
  GetRect(rcClient);
  SetDefaultFont();
  hex_edit_.Create(WS_CHILD | WS_TABSTOP,
    5, 5, rcClient.right - 10, rcClient.bottom - 10,
    this
  );
  static BYTE* data;
  data = buffer_.GetData();
  hex_edit_.SetData(&data, buffer_.GetDataSize());

  hex_edit_.Show();
  Show();
  return true;
}

void HexEditor::OnSize(WORD width, WORD height)
{
  hex_edit_.SetSize(width - 10, height - 10);
}

LRESULT HexEditor::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_ACTIVATE:
    hex_edit_.SetFocus();
    break ;
  }
  return Window::OnMessage(msg, wParam, lParam);
}

bool HexEditor::Register()
{
  HexEdit::Register();
  return Window::Register(ClassName, (HBRUSH) COLOR_WINDOW + 1);
}
