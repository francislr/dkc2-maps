
#pragma once

#include <windowsx.h>

extern HINSTANCE kInstance;
class Panel;
class Dialog;
class Buffer;

/* Get a localized error string from an error code */
void GetErrorMessage(std::string& message, DWORD code = 0);

class Window
{
public:
  Window();
  Window(HWND hwnd);
 virtual ~Window();

  bool Create(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwExStyles,
    DWORD dwStyles, int x, int y, int width, int height, HMENU menu,
    Window *parent);

  /* Window has been created */
  virtual void OnCreate();

  /* Window has been resized */
  virtual void OnSize(WORD width, WORD height);

  /* Control has been 'clicked'  */
  virtual void OnCommand(DWORD id, DWORD code);

  virtual void OnMouseDown(DWORD key, int x, int y);
  virtual void OnMouseUp(DWORD key, int x, int y);
  virtual void OnMouseMove(int x, int y);
  virtual void OnMouseDoubleClick(DWORD key, int x, int y);
  virtual bool OnKeyDown(DWORD key_code);
  virtual bool OnQueryCursor(DWORD dwHitTest, const POINT& pt);

  /* Handles the paint message */
  virtual void OnPaint(HDC hdc, const RECT& rcPaint);
  virtual void OnPaintArea(const RECT& pos, HDC hdc);
  virtual bool OnEraseBackground(HDC hdc);
  virtual bool OnDrawItem(DWORD id, DRAWITEMSTRUCT* item);
  virtual bool OnMesureItem(DWORD id, MEASUREITEMSTRUCT* item);
  virtual void OnTimer(DWORD id);

  /* Receives window messages */
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  /* Sets the sub window */
  void SetPanel(Panel *panel);

  void SetPosition(int x, int y, int width, int height);
  void SetSize(int width, int height);
  void SetDefaultFont();
  void SetRedraw(bool redraw);
  bool SetClipboardData(const TCHAR *format, Buffer& src_buf);

  void Show();
  void Hide();
  void SetFocus();
  void Invalidate(const RECT *rect);
  void Redraw();
  void Update();
  void CaptureMouse();
  void ReleaseMouse();

  void Destroy();
  void DestroyPanel();
  HDC GetDC();
  void DestroyDC(HDC hdc);
  virtual void GetRect(RECT& rect);
  int GetText(TCHAR *dst_buffer, int dst_buffer_count);
  HWND GetHWND();
  bool HasPanel() const { return panel_ != NULL; };
  bool IsValid() const { return hwnd_ != NULL; }

  /* Extract Window class and call its 'OnMessage' method. */
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam);

  /* Registers the window class */
  static bool Register(LPCSTR lpClassName,  HBRUSH hbrBackground,
    HICON hIcon = NULL, DWORD style = 0);

protected:
  Panel *panel_;
  bool child_window_;
  HWND hwnd_;

  friend Dialog;
};

/**
 * Menu structure for dynamic menus
 */
struct MENUDEF
{
  /* Resource ID of the menu */
  LPSTR resource;

  /* Name of the menu, when displayed */
  TCHAR *name;
};

class RedrawRegion
{
public:
  RedrawRegion(Window& window);
  ~RedrawRegion();

  /* Invalidate a rectangle in window,
   * and sets the invalidated flag
   * so the window can be redrawn when
   * this object is destroyed. */
  virtual void Invalidate(const RECT *zone);

protected:
  Window& window_;
  bool invalidated_;
};

class Panel : public Window
{
public:
  virtual bool OnDestroy() { return true; }

protected:
  Window *parent_;
  friend Window;
};

class ToolbarWindow : public Window
{
public:
  ToolbarWindow();
  ~ToolbarWindow();

  bool Create(DWORD dwExStyles, DWORD dwStyles, DWORD id, Window& parent);
  int AddBitmap(DWORD count, LPTBADDBITMAP bitmaps);
  bool AddButtons(DWORD count, LPTBBUTTON buttons);
  bool DeleteButton(DWORD index);

  /* Resize the toolbar to fit in the parent container */
  void AutoSize();
  void SetMaxTextRows(DWORD rows);

  /**
   * Sets the image list of the toolbar.
   * The Toolbar takes ownership of the image list.
   */
  void SetImgList(HIMAGELIST hToolImg, HIMAGELIST hDisabledImg);
  void DestroyToolbarImg();

private:
  HIMAGELIST toolbar_img_;
  HIMAGELIST disabled_img_;
};

class StatusWindow : public Window
{
public:
  StatusWindow();
  ~StatusWindow();

  bool Create(DWORD dwExStyles, DWORD dwStyles, DWORD id, Window& parent);
  void SetParts(DWORD count, int *partsWidth);
  void AutoSize();
};

class PickFileWindow
{
public:
  PickFileWindow(std::string& path, const TCHAR *filters = NULL,
    const TCHAR *def_ext = NULL);
  ~PickFileWindow();

  bool Open(const TCHAR* title, Window *parent);
  bool Save(const TCHAR* title, Window *parent);

private:
  std::string& path_;
  const TCHAR *filters_;
  const TCHAR *def_ext_;
};

class ListBox :
  public Window
{
public:
  bool Create(DWORD id, DWORD dwExStyles,
    DWORD dwStyles, int x, int y,
    int width, int height, Window& parent
  );
  int AddString(const TCHAR *str);
  DWORD GetItemData(int index);
  int SetItemData(int index, DWORD value);
  int GetSelIndex();
  void SetCount(UINT count);
  void SetSel(int index);
};

class ListView :
  public Window
{
public:
  bool Create(DWORD id, DWORD dwExStyles,
    DWORD dwStyles, int x, int y,
    int width, int height, Window& parent
  );
  void SetCount(UINT count);
};

class TreeView :
  public Window
{
public:
  bool Create(DWORD id, DWORD dwExStyles,
    DWORD dwStyles, int x, int y,
    int width, int height, Window& parent
  );
  HTREEITEM InsertItem(TVINSERTSTRUCT *tvinsert);
  void DeleteAll();
};
