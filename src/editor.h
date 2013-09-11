
#pragma once

#include "window.h"
#include "bitmap.h"
#include "undo_buffer.h"

class MainFrame;
class CartFile;

class EditorHost : public Window
{
public:
  EditorHost(MainFrame& frame);

  bool Create(LPCSTR lpWindowName, int x, int y,
    int width, int height, Window *parent);

  virtual void OnPaint(HDC hdc, const RECT& rcPaint);
  virtual bool OnEraseBackground(HDC hdc);

  /* Registers the frame window class */
  static bool Register();

private:
  MainFrame& frame_;

  /* Class name of the window class */
  static const TCHAR *ClassName;
};

class Editor : public Panel, public UndoBuffer
{
public:
  Editor(MainFrame& frame, CartFile& cart)
    : frame_(frame), cart_(cart) {
    child_window_ = true;
  }
  virtual ~Editor() { }

  virtual void OnCreate() = 0;
  virtual void OnCommand(DWORD id, DWORD code);
  bool Undo();
  bool Redo();
  virtual bool Do(Ref<UndoAction> action);

  virtual bool SaveFile();

protected:
  MainFrame& frame_;
  CartFile& cart_;
};

class ScrollSurface : public Editor, public BitmapSurface
{
public:
  ScrollSurface(MainFrame& frame, CartFile& cart);
  virtual ~ScrollSurface();

  class Redraw : public RedrawRegion
  {
  public:
    Redraw(ScrollSurface& surface);
    ~Redraw();

    virtual void Invalidate(const RECT *zone);

  private:
    ScrollSurface& surface_;
  };

  virtual void OnCreate();
  virtual void OnSize(WORD width, WORD height);
  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  virtual void OnPaint(HDC hdc, const RECT& rcPaint);
  virtual bool OnDestroy();

  virtual void OnMouseDown(DWORD key, int x, int y);
  virtual void OnMouseUp(DWORD key, int x, int y);
  virtual void OnMouseMove(int x, int y);
  virtual bool OnQueryCursor(DWORD dwHitTest, const POINT& pt);
  void GetCursorPos(POINT& pt);
  void ClientToScroll(RECT& rc);
  void ScrollToClient(RECT& rc);


  /**
   * Sets the dimensions of the surface
   */
  void SetDimensions(int width, int height);
  const RECT& GetScrollRect() const;
  virtual void GetRect(RECT& rect) const;
  void GetView(RECT& rect);
  int X() const { return x_; }
  int Y() const { return y_; }

  bool Resize(int width, int height);
  void UpdateScroll(int x, int y);
  void Scroll(int& x, int& y);

protected:
  int x_,
      y_;
  int scroll_x_,
      scroll_y_;
  int surface_width_,
      surface_height_;
  RECT scroll_rect_;
  RECT viewRc_;
};

/**
 * Provides selection marquee and drag & drop
 */
class EditTool : public Panel
{
public:
  EditTool(ScrollSurface& target);
  ~EditTool();

  /* Start the selection marquee */
  void StartSelection();
  void StopSelection();
  bool IsSelectionActive() const;
  
  /* Events for selection marquee */
  virtual void OnSelectionMove(const RECT& selection);
  virtual void OnSelectionRelease(const RECT& selection, RedrawRegion& redraw);

  /* Enable the drag & drop operation */
  void StartDrag();
  void StopDrag();
  bool IsDragging() const;

  /* Events for drag & drop */
  virtual void OnDragMove(int x, int y);
  virtual void OnDragRelease(const POINT& pt, int ix, int iy);

  virtual void OnPaintArea(const RECT& pos, HDC hdc);

  /* Called when the mouse move */
  virtual void OnMouseMove(int x, int y);
  virtual void OnMouseUp(DWORD key, int x, int y);

  virtual void OnTimer(DWORD id);

private:
  static const DWORD kTimerId;
  void Drag(int x, int y);
  void UpdateSelection();
  void DrawFocusRect(const RECT& pos, HDC hdc);
  void InvalidateFocusRect(RedrawRegion& redraw);

  ScrollSurface& target_;
  bool selection_active_;
  bool drag_active_;

  POINT dragPt_;
  int ix_;
  int iy_;
  RECT selectionRc_;
};

class SelectionMarquee
{
public:
  SelectionMarquee(Window& target);

  /* Enable the marquee to track the mouse */
  void Activate(int x, int y);
  void Desactivate(int x, int y);

  /* Draw the selection marquee on the device context */
  void Draw(HDC hdc, const RECT& pos);

  /* Called when the mouse move,
   * the selection rectangle is resized */
  void MouseMove(int x, int y);

  /* Whether or not the selection is shown. */
  bool IsActive() const;

  RECT& GetSelectionZone();

private:
  /* Selection zone */
  RECT zone_;
  RECT selection_zone_;

  /* true if selection is active and should be shown. */
  bool active_;

  int grid_;

  Window& target_;
};

class DragDrop
{
public:
  explicit DragDrop(Window& host);

  /* Enable the drag drop, track the mouse */
  void Activate(int x, int y);
  void Desactivate(int x, int y);

  /* Called when the mouse move,
   * new position is calculated */
  void MouseMove(int& x, int& y);

  /* Whether or not the drag drop is active. */
  bool IsActive() const;

private:
  /* Position of the last move */
  POINT last_move_;

  Window& host_;
  bool active_;
};
