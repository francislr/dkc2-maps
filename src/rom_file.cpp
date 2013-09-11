
#include "stdafx.h"
#include <stdarg.h>

#include "window.h" // for GetErrorMessage
#include "rom_file.h"

// Size of a HiROM Image (4 MB)
const DWORD HIROM_FILE_SIZE = 0x400000;

Buffer::Buffer(bool growable)
{
  growable_ = growable;
  data_ = NULL;
  data_size_ = 0;
  pos_ = 0;
}

Buffer::~Buffer()
{
  Release();
}

bool Buffer::Alloc(DWORD size)
{
  Release();

  data_ = new BYTE[size];
  if (!data_) {
    return false;
  }
  data_size_ = size;
  return true;
}

bool Buffer::Grow(DWORD size)
{
  if (!data_ || size == 0) {
    return false;
  }
  if (data_size_ >= size) {
    return true;
  }
  size += 30720;
  BYTE *new_data = new BYTE[size];
  if (!new_data) {
    return false;
  }
  memcpy(new_data, data_, data_size_);
  delete data_;
  data_ = new_data;
  data_size_ = size;
  return true;
}

void Buffer::Release()
{
  if (data_) {
    delete data_;
    data_ = NULL;
  }
}

bool Buffer::Read(UINT offset, void *buf, UINT buf_size) const
{
  if (offset + buf_size > data_size_) {
    return false;
  }
  memcpy(buf, &data_[offset], buf_size);
  return true;
}

bool Buffer::Read(void *buf, UINT buf_size) const
{
  if (!Read(pos_, buf, buf_size)) {
    return false;
  }
  pos_ += buf_size;
  return true;
}

bool Buffer::ReadByte(BYTE& byte) const
{
  return ReadT<BYTE>(byte);
}

bool Buffer::ReadWord(WORD& word) const
{
  return ReadT<WORD>(word);
}

bool Buffer::ReadDword(DWORD& dword) const
{
  return ReadT<DWORD>(dword);
}

bool Buffer::ReadString(UINT offset, std::string& string) const
{
  Scope scope(*this);
  Seek(offset);
  BYTE c;
  if (!ReadByte(c)) {
    return false;
  }
  do {
    string.append(1, c);
    if (!ReadByte(c)) {
      return false;
    }
  } while (c != '\0');
}

bool Buffer::Readf(const char *format, ...) const
{
  va_list args;
  va_start(args, format);
  const char *c;
  for (c = format; *c != '\0'; c++) {
    DWORD dummy;
    DWORD *data;
    data = va_arg(args, DWORD*);
    if (!data) {
      data = &dummy;
    }
    bool result = false;
    switch (*c) {
    case 'B': // BYTE
      result = Read(data, sizeof(BYTE));
      break ;
    case 'H': // WORD
       result = Read(data, sizeof(WORD));
      break ;
    case 'I': // DWORD
       result = Read(data, sizeof(DWORD));
      break ;
    default :
      result = false;
      break ;
    }
    if (!result) {
      va_end(args);
      return false;
    }
  }
  va_end(args);
  return true;
}

bool Buffer::Write(UINT offset, void *buf, UINT buf_size)
{
  if (!data_) {
    return false;
  }
  if (offset + buf_size > data_size_) {
    if (!growable_) {
      return false;
    }
    if (!Grow(offset + buf_size)) {
      return false;
    }
  }
  memcpy_s(&data_[offset], data_size_ - offset, buf, buf_size);
  return true;
}

bool Buffer::Write(UINT offset, const Buffer& source, UINT size)
{
  if (!source.data_ || !data_) {
    return false;
  }
  if (offset + size >= data_size_) {
    if (!growable_) {
      return false;
    }
    if (!Grow(offset + size)) {
      return false;
    }
  }
  return source.Read(&data_[offset], size);
}

bool Buffer::Write(void *buf, UINT buf_size)
{
  if (!Write(pos_, buf, buf_size)) {
    return false;
  }
  pos_ += buf_size;
  return true;
}

bool Buffer::MemSet(BYTE c, UINT size)
{
  for (UINT i = 0; i < size; i++) {
    if (!WriteByte(c)) {
      return false;
    }
  }
  return true;
}

bool Buffer::WriteByte(BYTE byte)
{
  return WriteT<BYTE>(byte);
}

bool Buffer::WriteWord(WORD word)
{
  return WriteT<WORD>(word);
}

bool Buffer::WriteDword(DWORD dword)
{
  return WriteT<DWORD>(dword);
}

bool Buffer::MoveDataTo(UINT from, UINT to, UINT size)
{
  if (to + size > data_size_ ||
      from + size > data_size_)
  {
    return false;
  }
  if (from > to) {
    return Read(from, data_ + to, size);
  }
  else {
    Buffer buf(false);
    if (!buf.Alloc(size)) {
      return false;
    }
    if (!buf.Write(0, data_ + from, size)) {
      return false;
    }
    if (!buf.Read(data_ + to, size)) {
      return false;
    }
  }
  return true;
}

bool Buffer::Writef(const char *format, ...)
{
  const char *c;
  va_list args;
  va_start(args, format);
  for (c = format; *c != '\0'; c++) {
    BYTE *data = NULL;
    int data_size = 0;
    data = va_arg(args, BYTE*);
    switch (*c) {
    case 'B': // BYTE
      data_size = sizeof(BYTE);
      break ;
    case 'H': // WORD
      data_size = sizeof(WORD);
      break ;
    case 'I': // DWORD
      data_size = sizeof(DWORD);
      break ;
    }
    if (!Write(data, data_size)) {
      va_end(args);
      return false;
    }
  }
  va_end(args);
  return true;
}

bool Buffer::FollowPtr2(UINT base, UINT offset)
{
  UINT addr = 0x00000000;
  if (!Read(base + offset, &addr, 2)) {
    return false;
  }
  pos_ = base + addr;
  return true;
}

bool Buffer::FollowPtr2(UINT base, UINT offset, UINT bank)
{
  UINT addr = bank << 16;
  if (!Read(base + offset, &addr, 2)) {
    return false;
  }
  pos_ = addr;
  return true;
}

bool Buffer::FollowPtr3(UINT base, UINT offset)
{
  UINT addr = 0x00000000;
  if (!Read(base + offset, &addr, 3)) {
    return false;
  }
  if (addr <= 0xC00000) {
    return false;
  }
  addr -= 0xC00000;
  pos_ = addr;
  return true;
}

bool Buffer::ReadAddr(DWORD& addr, UINT offset)
{
  addr = 0x00000000;
  if (!Read(offset, &addr, 3)) {
    return false;
  }
  if (addr <= 0xC00000) {
    return false;
  }
  addr -= 0xC00000;
  return true;
}

bool Buffer::ReadAddr(DWORD& addr)
{
  if (!ReadAddr(addr, pos_)) {
    return false;
  }
  pos_ += 3;
  return true;
}

void Buffer::Seek(UINT offset) const
{
  pos_ = offset;
}

void Buffer::Skip(int offset) const
{
  pos_ += offset;
}

UINT Buffer::GetPosition() const
{
  return pos_;
}

BYTE *Buffer::GetData()
{
  return data_;
}

UINT Buffer::GetDataSize()
{
  return data_size_;
}

void Buffer::WriteDebug(const char *file)
{
  FILE *fp;
  fp = fopen(file, "w");
  fwrite(data_, data_size_, 1, fp);
  fclose(fp);
}

CartFile::CartFile() : Buffer(false)
{
}

CartFile::~CartFile()
{
}

bool CartFile::Open(const std::string& file_name, std::string& error)
{
  error.clear();
  HANDLE file;
  file = CreateFile(file_name.c_str(),
    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL, NULL
  );
  if (file == INVALID_HANDLE_VALUE) {
    GetErrorMessage(error);
    return false;
  }
  DWORD file_size;
  file_size = GetFileSize(file, NULL);

  if (file_size != HIROM_FILE_SIZE) {
    CloseHandle(file);
    error = TEXT("The supplied file doesn't seem to be a valid HiROM cartridge image.");
    return false;
  }
  if (!Alloc(file_size)) {
    CloseHandle(file);
    Close();
    error = TEXT(
      "Not enough system memory to load the entire cartidge." \
      "Close some program and try again."
    );
    return false;
  }
  DWORD read;
  if (!ReadFile(file, data_, data_size_, &read, NULL) ||
      read != file_size) {
    CloseHandle(file);
    Release();
    GetErrorMessage(error);
    return false;
  }
  CloseHandle(file);
  if (!IsValid()) {
    Close();
    error = TEXT(
      "The supplied file doesn't seem to be a valid DKC2 cartridge." \
      "This version may not be supported."
    );
    return false;
  }
  return true;
}

bool CartFile::Save(const std::string& file_name, std::string& error)
{
  error.clear();
  if (!data_) {
    error = "File is not opened.";
    return false;
  }
  HANDLE file;
  file = CreateFile(file_name.c_str(),
    GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL, NULL
  );
  if (file == INVALID_HANDLE_VALUE) {
    GetErrorMessage(error);
    return false;
  }
  DWORD written;
  if (WriteFile(file, data_, data_size_, &written, NULL) == FALSE) {
    GetErrorMessage(error);
    CloseHandle(file);
    return false;
  }
  CloseHandle(file);
  return true;
}

bool CartFile::SaveAs()
{
  return true;
}

bool CartFile::Close()
{
  Release();
  return true;
}

bool CartFile::IsValid() const
{
  char sample[14];
  if (!Read(0x3F0000, sample, 14)) {
    return false;
  }
  if (memcmp(sample, "DIDDY ASSEMBLY", 14)) {
    return false;
  }
  // TODO: more validation
  return true;
}
