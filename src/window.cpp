
#include "stdafx.h"
#include <commctrl.h>

#include "window.h"
#include "rom_file.h"

HINSTANCE kInstance;

void GetErrorMessage(std::string& message, DWORD code)
{
  message.clear();
  if (!code) {
    code = GetLastError();
  }
  TCHAR *lpMsg;
  DWORD msgLen;
  msgLen = FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    code,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR) &lpMsg,
    0,
    NULL
  );
  if (!msgLen) {
    return ;
  }
  message.append(lpMsg, msgLen);
  LocalFree(lpMsg);
}

Window::Window()
{
  hwnd_ = NULL;
  child_window_ = false;
  panel_ = NULL;
}

Window::Window(HWND hwnd)
{
  hwnd_ = hwnd;
  child_window_ = true;
  panel_ = NULL;
}

Window::~Window()
{
  if (!child_window_) {
    Destroy();
  }
  DestroyPanel();
}

bool Window::Create(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwExStyles,
  DWORD dwStyles, int x, int y, int width, int height, HMENU menu,
  Window *parent)
{
  Destroy();
  HWND hParent = NULL;
  child_window_ = false;
  if (parent) {
      hParent = parent->GetHWND();
  }
  hwnd_ = CreateWindowEx(dwExStyles, lpClassName, lpWindowName, dwStyles,
      x, y, width, height, hParent, menu, kInstance, this);
  if (hwnd_ != NULL) {
    SetWindowLong(hwnd_, GWL_USERDATA, (LONG) this);
  }
  return hwnd_ != NULL;
}

void Window::OnCreate()
{
}

void Window::OnSize(WORD width, WORD height)
{
  if (panel_) {
    panel_->OnSize(width, height);
  }
}

void Window::OnCommand(DWORD id, DWORD code)
{
  if (panel_) {
    panel_->OnCommand(id, code);
  }
}

void Window::OnMouseDown(DWORD key, int x, int y)
{
  if (panel_) {
    panel_->OnMouseDown(key, x, y);
  }
}

void Window::OnMouseUp(DWORD key, int x, int y)
{
  if (panel_) {
    panel_->OnMouseUp(key, x, y);
  }
}

void Window::OnMouseMove(int x, int y)
{
  if (panel_) {
    panel_->OnMouseMove(x, y);
  }
}

void Window::OnMouseDoubleClick(DWORD key, int x, int y)
{
  if (panel_) {
    panel_->OnMouseDoubleClick(key, x, y);
  }
}

bool Window::OnKeyDown(DWORD key_code)
{
  if (panel_) {
    return panel_->OnKeyDown(key_code);
  }
  return false;
}

bool Window::OnQueryCursor(DWORD dwHitTest, const POINT& pt)
{
  if (panel_) {
    return panel_->OnQueryCursor(dwHitTest, pt);
  }
  return false;
}

void Window::OnPaint(HDC hdc, const RECT& rcPaint)
{
  if (panel_) {
    panel_->OnPaint(hdc, rcPaint);
  }
}

void Window::OnPaintArea(const RECT& pos, HDC hdc)
{
  if (panel_) {
    panel_->OnPaintArea(pos, hdc);
  }
}

bool Window::OnEraseBackground(HDC hdc)
{
  if (panel_) {
    return panel_->OnEraseBackground(hdc);
  }
  return false;
}

bool Window::OnDrawItem(DWORD id, DRAWITEMSTRUCT* item)
{
  if (panel_) {
    return panel_->OnDrawItem(id, item);
  }
  return false;
}

bool Window::OnMesureItem(DWORD id, MEASUREITEMSTRUCT* mesure)
{
  if (panel_) {
    return panel_->OnMesureItem(id, mesure);
  }
  return false;
}

void Window::OnTimer(DWORD id)
{
  if (panel_) {
    panel_->OnTimer(id);
  }
}

LRESULT Window::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  PAINTSTRUCT ps;
  switch (msg) {
  case WM_SIZE:
    OnSize(LOWORD(lParam), HIWORD(lParam));
    return 0;
  case WM_COMMAND:
    OnCommand(LOWORD(wParam), HIWORD(wParam));
    return 0;
  case WM_PAINT:
    hdc = BeginPaint(hwnd_, &ps);
    OnPaint(hdc, ps.rcPaint);
    EndPaint(hwnd_, &ps);
    return 0;
  case WM_ERASEBKGND:
    if (OnEraseBackground((HDC) wParam)) {
      return 0;
    }
    break ;
  case WM_LBUTTONDOWN:
    OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
  case WM_LBUTTONUP:
    OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
  case WM_MOUSEMOVE:
    OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
  case WM_LBUTTONDBLCLK:
    OnMouseDoubleClick(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return 0;
  case WM_KEYDOWN:
    if (OnKeyDown(wParam)) {
      return 0;
    }
    break ;
  case WM_SETCURSOR:
    {
      DWORD cursor_pos = GetMessagePos();
      POINT pt = {
        GET_X_LPARAM(cursor_pos),
        GET_Y_LPARAM(cursor_pos)
      };
      ScreenToClient(hwnd_, &pt);
      if (OnQueryCursor(LOWORD(lParam), pt)) {
        return TRUE;
      }
    }
    break;
  case WM_DRAWITEM:
    if (OnDrawItem(wParam, (LPDRAWITEMSTRUCT) lParam)) {
      return TRUE;
    }
    break ;
  case WM_MEASUREITEM:
    if (OnMesureItem(wParam, (LPMEASUREITEMSTRUCT) lParam)) {
      return TRUE;
    }
    break ;
  case WM_TIMER:
    OnTimer(wParam);
    break ;
  case WM_NCDESTROY:
    hwnd_ = NULL;
    break ;
  }
  if (panel_) {
    return panel_->OnMessage(msg, wParam, lParam);
  }
  return DefWindowProc(hwnd_, msg, wParam, lParam);
}

void Window::SetPanel(Panel *panel)
{
  DestroyPanel();
  panel_ = panel;
  if (panel_) {
    panel_->hwnd_ = hwnd_;
    panel_->parent_ = this;
    panel_->child_window_ = true;
    panel_->OnCreate();
  }
}

void Window::SetPosition(int x, int y, int width, int height)
{
  SetWindowPos(hwnd_, NULL, x, y, width, height, SWP_NOZORDER);
}

void Window::SetSize(int width, int height)
{
  SetWindowPos(hwnd_, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void Window::SetDefaultFont()
{
  SendMessage(hwnd_, WM_SETFONT,
    (WPARAM) GetStockObject(DEFAULT_GUI_FONT),
    FALSE
  );
}

void Window::SetRedraw(bool redraw)
{
  UINT flag = redraw ? 1 : 0;
  SendMessage(hwnd_, WM_SETREDRAW, flag, 0);
}

bool Window::SetClipboardData(const TCHAR *format, Buffer& src_buf)
{
  if (!OpenClipboard(hwnd_)) {
    return false;
  }
  UINT uFormat = RegisterClipboardFormat(format);
  UINT mem_size = src_buf.GetDataSize();
  if (mem_size > 10000) {
    CloseClipboard();
    return false;
  }
  HGLOBAL hClipData = GlobalAlloc(GMEM_MOVEABLE, mem_size);
  if (hClipData == NULL) {
    CloseClipboard();
    return false;
  }
  BYTE *data = (BYTE*) GlobalLock(hClipData);
  if (data == NULL) {
    GlobalFree(hClipData);
    CloseClipboard();
    return false;
  }
  src_buf.Read(0, data, mem_size);
  GlobalUnlock(hClipData);
  ::SetClipboardData(uFormat, hClipData);
  CloseClipboard();
  return true;
}

void Window::Show()
{
  ShowWindow(hwnd_, SW_SHOW);
}

void Window::Hide()
{
  ShowWindow(hwnd_, SW_HIDE);
}

void Window::SetFocus()
{
  ::SetFocus(hwnd_);
}

void Window::Invalidate(const RECT *rect)
{
  InvalidateRect(hwnd_, rect, FALSE);
}

void Window::Redraw()
{
  RedrawWindow(hwnd_, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void Window::Update()
{
  UpdateWindow(hwnd_);
}

void Window::CaptureMouse()
{
  SetCapture(hwnd_);
}

void Window::ReleaseMouse()
{
  ReleaseCapture();
}

void Window::Destroy()
{
  if (!hwnd_) {
    return ;
  }
  DestroyWindow(hwnd_);
  hwnd_ = NULL;
}

void Window::DestroyPanel()
{
  if (!panel_) {
    return ;
  }
  panel_->OnDestroy();
  delete panel_;
  panel_ = NULL;
}

HWND Window::GetHWND()
{
  return hwnd_;
}

HDC Window::GetDC()
{
  return ::GetDC(hwnd_);
}

void Window::DestroyDC(HDC hdc)
{
  ReleaseDC(hwnd_, hdc);
}

void Window::GetRect(RECT& rect)
{
  GetClientRect(hwnd_, &rect);
}

int Window::GetText(TCHAR *dst_buffer, int dst_buffer_count)
{
  return GetWindowText(hwnd_, dst_buffer, dst_buffer_count);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg,
  WPARAM wParam, LPARAM lParam)
{
  Window *self;
  self = (Window*) GetWindowLong(hwnd, GWL_USERDATA);
  if (msg == WM_CREATE) {
    CREATESTRUCT *ccs = (LPCREATESTRUCT) lParam;
    self = (Window*) ccs->lpCreateParams;
    if (self) {
      self->OnCreate();
      return 0;
    }
  }
  if (!self) {
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return self->OnMessage(msg, wParam, lParam);
}

bool Window::Register(LPCSTR lpClassName, HBRUSH hbrBackground, HICON hIcon,
  DWORD style)
{
  WNDCLASS wc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hbrBackground = hbrBackground;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = hIcon;
  wc.hInstance = kInstance;
  wc.lpfnWndProc = Window::WndProc;
  wc.lpszClassName = lpClassName;
  wc.lpszMenuName = NULL;
  wc.style = style;
  return RegisterClass(&wc) != NULL;
}

RedrawRegion::RedrawRegion(Window& window) :
  window_(window)
{
  invalidated_ = false;
}

RedrawRegion::~RedrawRegion()
{
  if (invalidated_) {
    window_.Update();
  }
}

void RedrawRegion::Invalidate(const RECT *rect)
{
  window_.Invalidate(rect);
  invalidated_ = true;
}

ToolbarWindow::ToolbarWindow()
{
  toolbar_img_ = NULL;
  disabled_img_ = NULL;
}

ToolbarWindow::~ToolbarWindow()
{
  DestroyToolbarImg();
}

bool ToolbarWindow::Create(DWORD dwExStyles, DWORD dwStyles, DWORD id,
  Window& parent)
{
  dwStyles |= WS_CHILD;
  if (!Window::Create(TOOLBARCLASSNAME, NULL, dwExStyles, dwStyles,
    0, 0, 0, 0, (HMENU) id, &parent)) {
    return false;
  }
  SendMessage(hwnd_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), NULL);
  return true;
}

int ToolbarWindow::AddBitmap(DWORD count, LPTBADDBITMAP bitmaps)
{
  return SendMessage(hwnd_, TB_ADDBITMAP, 0, (LPARAM) bitmaps);
}

bool ToolbarWindow::AddButtons(DWORD count, LPTBBUTTON buttons)
{
  return SendMessage(hwnd_, TB_ADDBUTTONS, count, (LPARAM) buttons)
    == TRUE;
}

bool ToolbarWindow::DeleteButton(DWORD index)
{
  return SendMessage(hwnd_, TB_DELETEBUTTON, index, 0) != FALSE;
}

void ToolbarWindow::AutoSize()
{
  SendMessage(hwnd_, TB_AUTOSIZE, 0, 0);
}

void ToolbarWindow::SetMaxTextRows(DWORD rows)
{
  SendMessage(hwnd_, TB_SETMAXTEXTROWS, rows, 0);
}

void ToolbarWindow::SetImgList(HIMAGELIST hToolImg, HIMAGELIST hDiabledImg)
{
  DestroyToolbarImg();
  if (hToolImg) {
    toolbar_img_ = hToolImg;
  }
  if (hDiabledImg) {
    disabled_img_ = hDiabledImg;
  }

  SendMessage(hwnd_, TB_SETIMAGELIST, 0, (LPARAM) hToolImg);
  SendMessage(hwnd_, TB_SETDISABLEDIMAGELIST, 0, (LPARAM) hDiabledImg);
}

void ToolbarWindow::DestroyToolbarImg()
{
  if (toolbar_img_) {
    ImageList_Destroy(toolbar_img_);
    toolbar_img_ = NULL;
  }
  if (disabled_img_) {
    ImageList_Destroy(disabled_img_);
    disabled_img_ = NULL;
  }
}

StatusWindow::StatusWindow()
{
}

StatusWindow::~StatusWindow()
{
}

bool StatusWindow::Create(DWORD dwExStyles, DWORD dwStyles, DWORD id,
  Window& parent)
{
  dwStyles |= WS_CHILD;
  return Window::Create(STATUSCLASSNAME, NULL, dwExStyles, dwStyles,
    0, 0, 0, 0, (HMENU) id, &parent);
}

void StatusWindow::SetParts(DWORD count, int *partsWidth)
{
  SendMessage(hwnd_, SB_SETPARTS, count, (LPARAM) partsWidth);
}

void StatusWindow::AutoSize()
{
  SendMessage(hwnd_, WM_SIZE, 0, 0);
}

PickFileWindow::PickFileWindow(std::string& path, const TCHAR *filters,
  const TCHAR *def_ext) :
  path_(path), def_ext_(def_ext)
{
  filters_ = filters;
}

PickFileWindow::~PickFileWindow()
{
}

bool PickFileWindow::Open(const TCHAR* title, Window *parent)
{
  TCHAR path[MAX_PATH] = {},
        file_name[64];
  OPENFILENAME ofn = {};
  ofn.lStructSize = sizeof(ofn);
  if (parent) {
    ofn.hwndOwner = parent->GetHWND();
  }
  ofn.hInstance = kInstance;
  ofn.lpstrFilter = filters_;

  ofn.lpstrFile = path;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFileTitle = file_name;
  ofn.nMaxFileTitle = 64;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  ofn.lpstrDefExt = def_ext_;
  if (title) {
    ofn.lpstrTitle = title;
  }
  if (!GetOpenFileName(&ofn)) {
    return false;
  }
  path_.clear();
  path_.append(ofn.lpstrFile, ofn.nMaxFile);
  return true;
}

bool ListBox::Create(DWORD id, DWORD dwExStyles,
  DWORD dwStyles, int x, int y, int width, int height,
  Window& parent)
{
  dwStyles |= WS_CHILD;
  return Window::Create(WC_LISTBOX, NULL, dwExStyles,
    dwStyles, x, y, width, height, (HMENU) id, &parent);
}

int ListBox::AddString(const TCHAR *str)
{
  return SendMessage(hwnd_,
    LB_ADDSTRING,
    0, (LPARAM) str
  );
}

DWORD ListBox::GetItemData(int index)
{
  return SendMessage(hwnd_,
    LB_GETITEMDATA,
    index, 0
  );
}

int ListBox::SetItemData(int index, DWORD value)
{
  return SendMessage(hwnd_,
    LB_SETITEMDATA,
    index, value
  );
}

int ListBox::GetSelIndex()
{
  return SendMessage(hwnd_, LB_GETCURSEL, 0, 0);
}

void ListBox::SetCount(UINT count)
{
  SendMessage(hwnd_,
    LB_SETCOUNT,
    count, 0
  );
}

void ListBox::SetSel(int index)
{
  SendMessage(hwnd_, LB_SETCURSEL, index, 0);
}

bool ListView::Create(DWORD id, DWORD dwExStyles,
  DWORD dwStyles, int x, int y,
  int width, int height, Window& parent
)
{
  dwStyles |= WS_CHILD;
  return Window::Create(WC_LISTVIEW, NULL, dwExStyles,
    dwStyles, x, y, width, height, (HMENU) id, &parent);
}

void ListView::SetCount(UINT count)
{
  SendMessage(hwnd_, LVM_SETITEMCOUNT, count, 0);
}

bool TreeView::Create(DWORD id, DWORD dwExStyles,
  DWORD dwStyles, int x, int y,
  int width, int height, Window& parent)
{
  dwStyles |= WS_CHILD;
  return Window::Create(WC_TREEVIEW, NULL, dwExStyles,
    dwStyles, x, y, width, height, (HMENU) id, &parent);
}

HTREEITEM TreeView::InsertItem(TVINSERTSTRUCT *tvinsert)
{
  return (HTREEITEM) TreeView_InsertItem(hwnd_, tvinsert);
}

void TreeView::DeleteAll()
{
  SendMessage(hwnd_, TVM_DELETEITEM, 0, (LPARAM) TVI_ROOT);
}
