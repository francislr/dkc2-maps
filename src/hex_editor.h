
#pragma once

#include "hexedit.h"

class Buffer;

class HexEditor :
  public Window
{
public:
  HexEditor(Buffer& buffer);
  ~HexEditor();

  bool Create(LPCSTR lpWindowName, int x, int y,
    int width, int height, Window *parent);

  void OnSize(WORD width, WORD height);
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  static bool Register();

private:
  Buffer& buffer_;
  HexEdit hex_edit_;

  static const TCHAR *ClassName;
};
