
#include "stdafx.h"

#include "dialog.h"

Dialog::Dialog()
{
}

Dialog::~Dialog()
{
}

bool Dialog::Create(WORD resourceID, Window *parent)
{
  HWND hParent = NULL;
  if (parent) {
    hParent = parent->GetHWND();
  }
  hwnd_ = CreateDialogParam(kInstance,
    MAKEINTRESOURCE(resourceID),
    hParent,
    DialogProc,
    (LPARAM) this
  );
  if (!hwnd_) {
    return false;
  }
  return true;
}

DWORD Dialog::CreateModal(WORD resourceID, Window& parent)
{
  HWND hParent = parent.GetHWND();
  DWORD result;
  result = DialogBoxParam(kInstance,
    MAKEINTRESOURCE(resourceID),
    hParent,
    DialogProc,
    (LPARAM) this
  );
  return result;
}

void Dialog::OnOK()
{
  End(0);
}

void Dialog::OnCancel()
{
  End(-1);
}

void Dialog::End(UINT_PTR ret)
{
  EndDialog(hwnd_, ret);
}

bool Dialog::GetItemText(DWORD id, std::string& text)
{
  text.clear();
  TCHAR str[512];
  DWORD strLen;
  strLen = GetDlgItemText(hwnd_, id, str, sizeof(str));
  if (strLen == 0) {
    return false;
  }
  text.append(str, strLen);
  return true;
}

bool Dialog::GetItemUInt(DWORD id, UINT& value)
{
  BOOL translated;
  value = GetDlgItemInt(hwnd_, id, &translated, FALSE);
  return value != 0 || translated == TRUE;
}

bool Dialog::GetItemInt(DWORD id, INT& value)
{
  BOOL translated;
  value = GetDlgItemInt(hwnd_, id, &translated, TRUE);
  return value != 0 || translated == TRUE;
}

void Dialog::SetItemText(DWORD id, const std::string& text)
{
  SetDlgItemText(hwnd_, id, text.c_str());
}

void Dialog::SetItemUInt(DWORD id, UINT value)
{
  SetDlgItemInt(hwnd_, id, value, FALSE);
}

void Dialog::SetItemInt(DWORD id, INT value)
{
  SetDlgItemInt(hwnd_, id, value, TRUE);
}

void Dialog::AddListItem(DWORD id, const TCHAR* string)
{
  HWND hLbbox = GetDlgItem(hwnd_, id);
  if (!hLbbox) {
    return ;
  }
  SendMessage(hLbbox, LB_ADDSTRING, 0, (LPARAM) string);
}

void Dialog::GetItem(DWORD id, Window& window)
{
  HWND hItemWnd = GetDlgItem(hwnd_, id);
  window.hwnd_ = hItemWnd;
  window.child_window_ = true;
}

INT_PTR Dialog::DialogMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
  case WM_INITDIALOG:
    OnCreate();
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      OnOK();
      return TRUE;
    case IDCANCEL:
      OnCancel();
      return TRUE;
    }
    OnCommand(LOWORD(wParam), HIWORD(wParam));
    return TRUE;
  }
  return FALSE;
}


INT_PTR CALLBACK Dialog::DialogProc(HWND hwnd, UINT msg,
  WPARAM wParam, LPARAM lParam)
{
  Dialog *self;
  self = (Dialog*) GetWindowLong(hwnd, GWL_USERDATA);
  if (msg == WM_INITDIALOG) {
    self = (Dialog*) lParam;
    self->hwnd_ = hwnd;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG) self);
  }
  if (self) {
    return self->DialogMessage(msg, wParam, lParam);
  }
  return FALSE;
}
