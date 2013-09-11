
#pragma once

#include "window.h"

class Dialog : public Window
{
public:
  Dialog();
  ~Dialog();

  /* Creates a modeless dialog box */
  bool Create(WORD resourceID, Window *parent);

  /* Creates a modal dialog box */
  DWORD CreateModal(WORD resourceID, Window& parent);

  /* OK Signal received */
  virtual void OnOK();

  /* Cancel Siganal received */
  virtual void OnCancel();

  /* Destroy this modal dialog */
  void End(UINT_PTR ret);

  /* Get a control text by its identifier */
  bool GetItemText(DWORD id, std::string& text);
  bool GetItemUInt(DWORD id, UINT& value);
  bool GetItemInt(DWORD id, INT& value);

  /* Set a control text by its identifier */
  void SetItemText(DWORD id, const std::string& text);
  void SetItemUInt(DWORD id, UINT value);
  void SetItemInt(DWORD id, INT value);
  void AddListItem(DWORD id, const TCHAR* string);
  void GetItem(DWORD id, Window& window);

  virtual INT_PTR DialogMessage(UINT msg, WPARAM wParam, LPARAM lParam);

protected:

private:
  static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam);
};
