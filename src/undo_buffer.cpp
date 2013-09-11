
#include "stdafx.h"
#include "undo_buffer.h"

bool UndoAction::Redo()
{
  return Do();
}

UndoBuffer::UndoBuffer()
{
  undo_index_ = -1;
}

UndoBuffer::~UndoBuffer()
{
}

bool UndoBuffer::Do(Ref<UndoAction> action)
{
  if (!action) {
    return false;
  }
  if (!action->Do()) {
    return false;
  }
  // Deletes all the action after the current
  // action index.
  ActionList::iterator i;
  UINT n;
  i = begin();
  for (n = 0; n < undo_index_ && i != end(); n++, ++i);
  while (i != end()) {
    i = erase(i);
  }
  //for (n = undo_index_ + 1; n < size() && i != end(); n++) {
  //}
  push_back(action);
  undo_index_ = size();
  return true;
}

bool UndoBuffer::Undo()
{
  if (undo_index_ > (int) size() || undo_index_ < 1) {
    return false;
  }
  Ref<UndoAction> action;
  action = GetActionFromIndex(undo_index_ - 1);
  if (!action) {
    return false;
  }
  if (!action->Undo()) {
    return false;
  }
  undo_index_--;
  return true;
}

bool UndoBuffer::CanUndo() const
{
  return (int) size() > undo_index_ && undo_index_ >= 0;
}

bool UndoBuffer::Redo()
{
  if (undo_index_ < 0 || (int) undo_index_ >= size()) {
    return false;
  }
  Ref<UndoAction> action;
  action = GetActionFromIndex(undo_index_);
  if (!action) {
    return false;
  }
  if (!action->Redo()) {
    return false;
  }
  undo_index_++;
  return true;
}

bool UndoBuffer::CanRedo() const
{
  return undo_index_ + 1 < (int) size() && undo_index_ + 1 >= 0;
}

Ref<UndoAction> UndoBuffer::GetActionFromIndex(int index)
{
  ActionList::iterator i;
  int n;
  for (i = begin(), n = 0; i != end(); ++i, n++) {
    if (n == index) {
      return *i;
    }
  }
  return Ref<UndoAction>();
}
