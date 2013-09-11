
#pragma once

#include <list>

#include "window.h"
#include "editor.h"
#include "ref_count.h"

class Window;
class Buffer;

class EditorObject : public RefCount
{
public:
  EditorObject();
  /**
   * Draws this object on the editor
   */
  virtual bool Draw(BitmapSurface& surface,
    const RECT& pos) = 0;

  /**
   * Whether a rectangle intersect with this object
   */
  virtual bool IntersectBox(RECT& box) = 0;

  /**
   * Whether a x/y intersect with this object
   */
  virtual bool IntersectPoint(POINT& pt) = 0;

  /**
   * Moves this object on the editor
   */
  virtual bool Move(int x, int y) = 0;

  /**
   * Get the rectangle of this object, relative to pos
   */
  virtual void GetRect(RECT& rect) = 0;

  /**
   * Selects/unselects this object
   * Returns whether or not the selected state
   * has changed
   */
  virtual bool Select();
  virtual bool Unselect();

  /**
   * Transform the object to a binary
   * data.
   */
  bool Serialize(Buffer& dest_buf);
  bool Deserialize(Buffer& src_buf);

  /**
   * Returns whether or not this object is selected
   */
  virtual bool IsSelected() const;

  /**
   * Marks this object rectangle to be redrawn on the window
   */
  virtual bool Invalidate(RedrawRegion& redraw);

  bool operator ==(Ref<EditorObject> obj);

protected:
  UINT unique_id_;
  bool selected_;
};

template<class T>
class EditorObjectManager
{
public:
  typedef std::list<Ref<T> > ObjectList;

  void Draw(BitmapSurface& surface, const RECT& pos)
  {
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      (*i)->Draw(surface, pos);
    }
  }

  bool Select(const RECT& zone,
    RedrawRegion& redraw)
  {
    bool selected = false;
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      RECT zonebis = zone;
      Ref<T> obj = (*i);
      if (obj->IntersectBox(zonebis)) {
        if (obj->Select()) {
          selected = true;
          obj->Invalidate(redraw);
        }
      }
    }
    return selected;
  }

  bool Select(const ObjectList& selected_objects, RedrawRegion& redraw)
  {
    bool selected = false;
    ObjectList::const_iterator si;
    for (si = selected_objects.begin(); si != selected_objects.end(); ++si) {
      ObjectList::iterator i;
      for (i = objects_.begin(); i != objects_.end(); ++i) {
        if (**i == **si) {
          Ref<T> obj = (*i);
          if (obj->Select()) {
            selected = true;
            obj->Invalidate(redraw);
          }
        }
      }
    }
    return selected;
  }

  bool Unselect(RedrawRegion& redraw)
  {
    bool unselected = false;
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      if ((*i)->Unselect()) {
        unselected = true;
        obj->Invalidate(redraw);
      }
    }
    return unselected;
  }

  bool InvalidateAll(RedrawRegion& redraw)
  {
    bool invalidated = false;
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      if (obj->Invalidate(redraw)) {
        invalidated = true;
      }
    }
    return invalidated;
  }

  Ref<T> IntersectPoint(int x, int y)
  {
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      POINT pt = { x, y };
      if (obj->IntersectPoint(pt)) {
        return obj;
      }
    }
    return Ref<T>();
  }

  bool Move(RedrawRegion& redraw, int x, int y)
  {
    bool moved = false;
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      if (obj->IsSelected()) {
        obj->Invalidate(redraw);
        if (obj->Move(x, y)) {
          obj->Invalidate(redraw);
          moved = true;
        }
      }
    }
    return moved;
  }

  void GetSelected(ObjectList& selected) {
    selected.clear();
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      if (obj->IsSelected()) {
        selected.push_back(obj);
      }
    }
  }
  Ref<T> GetSelected() {
    ObjectList::iterator i;
    for (i = objects_.begin(); i != objects_.end(); ++i) {
      Ref<T> obj = (*i);
      if (obj->IsSelected()) {
        return obj;
      }
    }
    return Ref<T>();
  }

  ObjectList objects_;
};

template<typename T>
class EditorTool : public EditTool
{
public:
  EditorTool(EditorObjectManager<T>& objects, ScrollSurface& editor) :
    objects_(objects), editor_(editor),
    EditTool(editor)
  {
  }
  typedef std::list<Ref<T> > ObjectList;

  class Action : public UndoAction
  {
  public:
    Action(ScrollSurface& editor, EditorObjectManager<T>& obj_manager) :
      editor_(editor), obj_manager_(obj_manager)
    {
      obj_manager.GetSelected(objects_);
    }
    typedef bool (Action::*ActionFunc)(RedrawRegion& redraw, Ref<T> obj);

    virtual bool ForEach(RedrawRegion& redraw, ActionFunc action_func) {
      ObjectList::iterator i;
      for (i = objects_.begin(); i != objects_.end(); ++i) {
        if (!(this->*action_func)(redraw, *i)) {
          return false;
        }
      }
      return true;
    }

    virtual bool Do() {
      ScrollSurface::Redraw redraw(editor_);
      return ForEach(redraw, &Action::ObjDo);
    }

    virtual bool Undo() {
      ScrollSurface::Redraw redraw(editor_);
      obj_manager_.Unselect(redraw);
      obj_manager_.Select(objects_, redraw);
      return ForEach(redraw, &Action::ObjUndo);
    }

    virtual bool Redo() {
      ScrollSurface::Redraw redraw(editor_);

      obj_manager_.Unselect(redraw);
      obj_manager_.Select(objects_, redraw);
      return ForEach(redraw, &Action::ObjRedo);
    }

    virtual bool ObjDo(RedrawRegion& redraw, Ref<T> obj)
    {
      return true;
    }
    virtual bool ObjUndo(RedrawRegion& redraw, Ref<T> obj)
    {
      return true;
    }
    virtual bool ObjRedo(RedrawRegion& redraw, Ref<T> obj)
    {
      return ObjDo(redraw, obj);
    }

  protected:
    ScrollSurface& editor_;
    EditorObjectManager<T>& obj_manager_;
    ObjectList objects_;
  };

  class MoveAction : public Action
  {
  public:
    MoveAction(ScrollSurface& editor, EditorObjectManager<T>& obj_manager,
      int dx, int dy) :
        Action(editor, obj_manager) {
      dx_ = dx,
      dy_ = dy;
    }

    virtual bool Undo()
    {
      ScrollSurface::Redraw redraw(editor_);
      obj_manager_.Unselect(redraw);
      obj_manager_.Select(objects_, redraw);
      obj_manager_.Move(redraw, -dx_, -dy_);
      return true;
    }

    virtual bool Redo()
    {
      ScrollSurface::Redraw redraw(editor_);
      obj_manager_.Unselect(redraw);
      obj_manager_.Select(objects_, redraw);
      obj_manager_.Move(redraw, dx_, dy_);
      return true;
    }

  private:
    int dx_, dy_;
  };

  virtual void OnMouseDown(DWORD key, int x, int y)
  {
    ScrollSurface::Redraw redraw(editor_);
    /* Get the scrolling area */
    const RECT& pos = editor_.GetScrollRect();

    Ref<T> obj;
    /* Get the object pointed by the cursor */
    obj = objects_.IntersectPoint(x, y);

    if (obj && obj->IsSelected()) {
      /* Enables drag & drop if it points to a selected object */
      //dragdrop_.Activate(x, y);
      StartDrag();
    }
    else if (obj) { // Points to an object, not selected
      if (!(key & MK_SHIFT)) {
        /* Shift key is not pressed,
         * unselects all other selected objects */
        objects_.Unselect(redraw);
      }
      /* Selects the object and enable drag & drop */
      obj->Select();
      //dragdrop_.Activate(x, y);
      StartDrag();
    }
    else {
      if (!(key & MK_SHIFT)) {
        if (objects_.Unselect(redraw)) {
          editor_.Update();
        }
      }
      StartSelection();
      //selection_.Activate(x, y);
    }
  }

  /*
  virtual void OnMouseUp(DWORD key, int x, int y)
  {
    RedrawRegion redraw(editor_);
    const RECT pos = editor_.GetScrollRect();
    if (dragdrop_.IsActive()) {
      dragdrop_.Desactivate(x, y);
    }
    else if (selection_.IsActive()) {
      selection_.Desactivate(x, y);
    }
  }
  */

  /*
  virtual void OnMouseMove(int x, int y)
  {
    RedrawRegion redraw(editor_);
    const RECT pos = editor_.GetScrollRect();
    if (dragdrop_.IsActive()) {
      dragdrop_.MouseMove(x, y);
      objects_.Move(pos, redraw, x, y);
    }
    else if (selection_.IsActive()) {
      selection_.MouseMove(x, y);
      objects_.Unselect(pos, redraw);
      RECT& zone = selection_.GetSelectionZone();
      objects_.Select(zone, pos, redraw);
    }
  }

  */

  virtual bool OnQueryCursor(DWORD dwHitTest, const POINT& pt)
  {
    if (IsDragging()) {
      SetCursor(LoadCursor(NULL, IDC_SIZEALL));
      return true;
    }

    const RECT pos = editor_.GetScrollRect();
    if (objects_.IntersectPoint(pt.x, pt.y)) {
      SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }
    else {
      SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
    return true;
  }

  virtual void OnSelectionRelease(const RECT& selection, RedrawRegion& redraw)
  {
    const RECT pos = editor_.GetScrollRect();
    objects_.Unselect(redraw);
    objects_.Select(selection, redraw);
  }

  virtual void OnDragMove(int x, int y)
  {
    ScrollSurface::Redraw redraw(editor_);
    //RedrawRegion redraw(editor_);
    //const RECT pos = editor_.GetScrollRect();
    objects_.Move(redraw, x, y);
  }

  virtual void OnDragRelease(const POINT& pt, int ix, int iy)
  {
    Ref<UndoAction> move;
    move = new MoveAction(editor_, objects_, ix, iy);
    editor_.Do(move);
  }

  virtual LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam)
  {
    return Window::OnMessage(msg, wParam, lParam);
  }

  ScrollSurface& editor_;

protected:
  EditorObjectManager<T>& objects_;

private:
};
