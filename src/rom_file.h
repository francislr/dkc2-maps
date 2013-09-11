
#pragma once

#include <string>

/* Convert a HiROM address to a file address */
#define ROM2F(x)  (x - 0xC00000)

/* Convert a file address to a HiROM address */
#define F2ROM(x)  (x + 0xC00000)

class Buffer
{
public:
  Buffer(bool growable);
  ~Buffer();

  bool Alloc(DWORD size);
  bool Grow(DWORD size);
  void Release();

  /**
   * Fills 'buf' with data from 'offset'
   * Returns true on success.
   */
  bool Read(UINT offset, void *buf, UINT buf_size) const;
  bool Read(void *buf, UINT buf_size) const;
  bool ReadByte(BYTE& byte) const;
  bool ReadWord(WORD& word) const;
  bool ReadDword(DWORD& dword) const;

  /* Reads a null-terminated string */
  bool ReadString(UINT offset, std::string& string) const;

  /**
   * Reads data from 'buffer_' and stores according
   * to 'format' into the arguments pointers.
   * Valid formats are:
   *   B  BYTE    H  WORD    I  DWORD
   */
  bool Readf(const char *format, ...) const;

  template<typename T >
  bool ReadT(T& value) const {
    return Read(&value, sizeof(value));
  }
  BYTE operator [] (UINT index) const {
    BYTE b = 0;
    Read(index, &b, 1);
    return b;
  }

  bool Write(UINT offset, void *buf, UINT buf_size);
  bool Write(UINT offset, const Buffer& source, UINT size);
  bool Write(void *buf, UINT buf_size);
  bool MemSet(BYTE c, UINT size);
  bool WriteByte(BYTE byte);
  bool WriteWord(WORD word);
  bool WriteDword(DWORD dword);
  bool MoveDataTo(UINT from, UINT to, UINT size);

  /**
   * Writes formatted data to 'buffer_'
   */
  bool Writef(const char *format, ...);

  template<typename T >
  bool WriteT(T value) {
    return Write(&value, sizeof(value));
  }

 /* Change the current address by reading a value from a table */
  bool FollowPtr2(UINT base, UINT offset);
  bool FollowPtr2(UINT base, UINT offset, UINT bank);
  bool FollowPtr3(UINT base, UINT offset);

  bool ReadAddr(DWORD& addr, UINT offset);
  bool ReadAddr(DWORD& addr);

  /**
   * Changes the cursor position to 'offset'
   */
  void Seek(UINT offset) const;
  void Skip(int offset) const;

  UINT GetPosition() const;
  BYTE *GetData();
  UINT GetDataSize();

  void WriteDebug(const char *file);

protected:
  BYTE *data_;
  UINT data_size_;

  /**
   * Buffer can be resized when a write
   * operation has not enough memory
   */
  bool growable_;

  /**
   * Position in the buffer. Used and increased by operations
   * that doesn't specify an offset.
   */
  mutable UINT pos_;

public:
  /**
   * Saves the position of the cartridge and
   * restores it when this object is destroyed.
   */
  class Scope
  {
  public:
    Scope(const Buffer& buffer) : buffer_(buffer) {
      // Save the actual position
      pos_ = buffer.GetPosition();
    }
    ~Scope() {
      // Restore the position
      buffer_.Seek(pos_);
    }

    int GetDiff() const {
      return buffer_.GetPosition() - pos_;
    }

    UINT GetPosition() const {
      return pos_;
    }
  private:
    const Buffer& buffer_;
    UINT pos_;
  };
};

class CartFile : public Buffer
{
public:
  CartFile();
  ~CartFile();

  bool Open(const std::string& file_name, std::string& error);
  bool Save(const std::string& file_name, std::string& error);
  bool SaveAs();
  bool Close();

  /**
   * Checks for the validity of the file.
   */
  bool IsValid() const;

  typedef Buffer::Scope Scope;
};
