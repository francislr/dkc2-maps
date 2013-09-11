
#include "stdafx.h"

#include "editor.h"
#include "editor_tool.h"
#include "rom_file.h"


EditorObject::EditorObject()
{
  static UINT inc_id = 0;
  unique_id_ = inc_id;
  inc_id++;
  selected_ = false;
}

bool EditorObject::Select()
{
  if (selected_) {
    return false;
  }
  selected_ = true;
  return true;
}

bool EditorObject::Unselect()
{
  if (!selected_) {
    return false;
  }
  selected_ = false;
  return true;
}

bool EditorObject::Serialize(Buffer& dest_buf)
{
  return false;
}

bool EditorObject::Deserialize(Buffer& src_buf)
{
  return false;
}

bool EditorObject::Invalidate(RedrawRegion& redraw)
{
  RECT zone;
  GetRect(zone);
  zone.left -= 2, zone.top -2,
  zone.right += 2, zone.bottom += 2;
  redraw.Invalidate(&zone);
  return true;
}

bool EditorObject::operator ==(Ref<EditorObject> obj)
{
  return obj->unique_id_ == unique_id_;
}

bool EditorObject::IsSelected() const
{
  return selected_;
}
