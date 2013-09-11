
#pragma once

#include <list>

#include "ref_count.h"

class UndoAction : public RefCount
{
public:
  virtual bool Do() = 0;
  virtual bool Undo() = 0;
  virtual bool Redo();
};

class UndoBuffer : public std::list<Ref<UndoAction> >
{
public:
  UndoBuffer();
  ~UndoBuffer();

  /**
   * Executes an action and adds it to
   * the undo buffer history.
   */
  virtual bool Do(Ref<UndoAction> action);

  /**
   * Undo the last action in the history.
   */
  virtual bool Undo();
  bool CanUndo() const;

  /**
   * Redo the action from the current action index.
   */
  virtual bool Redo();
  bool CanRedo() const;

  /**
   * Get an action from the undo buffer by its index.
   */
  Ref<UndoAction> GetActionFromIndex(int index);

private:
  /**
   * Current position for the undo method.
   */
  int undo_index_;

  typedef std::list<Ref<UndoAction> > ActionList;
};
