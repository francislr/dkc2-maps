
#pragma once

#include "stdafx.h"
#include <map>

/**
 * Reference counting for resource management
 */
class RefCount
{
public:
  RefCount() {
    ref_count_ = 0;
  }
  virtual ~RefCount() {
  }

  /* Increase reference count */
  void IncRef() {
    ref_count_++;
  }

  /**
   * Decrease reference count. When it reaches zero,
   * the resource is released.
   */
  void DecRef() {
    ref_count_--;
    if (ref_count_ <= 0) {
      delete this;
    }
  }

  /* Get the reference count */
  int GetRefCount() {
    return ref_count_;
  }

private:
  int ref_count_;
};

template<class T >
class Ref
{
public:
  Ref() {
    ptr_ = NULL;
  }
  Ref(T *ptr) {
    ptr_ = NULL;
    operator =(ptr);
  }
  Ref(const Ref<T>& ref) {
    ptr_ = ref.ptr_;
    if (ptr_) {
      ptr_->IncRef();
    }
  }
  ~Ref() {
    if (ptr_) {
      ptr_->DecRef();
      ptr_ = NULL;
    }
  }

  virtual Ref<T>& operator =(Ref<T>& ref) {
    return operator =(ref.ptr_);
  }

  virtual Ref<T>& operator =(T *ptr) {
    if (ptr) {
      ptr->IncRef();
    }
    if (ptr_) {
      ptr_->DecRef();
    }
    ptr_ = ptr;
    return *this;
  }

  T *operator *() const {
    return ptr_;
  }

  T *operator &() const {
    return ptr_;
  }

  T *operator ->() const {
    return ptr_;
  }

  operator bool() const {
    return ptr_ != NULL;
  }

private:
  T *ptr_;
};

template<class T>
class ResourcePool
{
public:
  ResourcePool() { }
  ~ResourcePool() { }

  /**
   * Releases the resource that has only
   * one reference.
   */
  void GarbageCollect() {
    ResMap::iterator i;
    for (i = resources_.begin(); i != resources_.end(); ++i) {
      UINT ref_count = i->second->GetRefCount();
      if (ref_count == 1) {
        resources_.erase(i);
      }
    }
  }

  /**
   * Allocate a new resource, or returns an
   * existing one if it already allocated.
   */
  Ref<T> New(DWORD id) {
    ResMap::iterator res_it;
    res_it = resources_.find(id);
    if (res_it != resources_.end()) {
      // Already exist
      return res_it->second;
    }
    Ref<T> resource = new T();
    if (!resource) {
      return Ref<T>();
    }
    if (!OnResourceAlloc(id, *resource)) {
      // Failed initialization
      return Ref<T>();
    }
    resources_[id] = resource;
    return resource;
  }

protected:
  /**
   * Called when a new resource is allocated
   * so super class can initialize the reousrce.
   * Returns false to cancel the allocation.
   */
  virtual bool OnResourceAlloc(DWORD id, T* resource) {
    return true;
  }

  /* Called when a resource is to be released */
  virtual void OnResourceRelease(DWORD id, T* resource) {
  }

  typedef std::map<DWORD, Ref<T>> ResMap;
  /* Allocated resource tracking */
  ResMap resources_;
};
