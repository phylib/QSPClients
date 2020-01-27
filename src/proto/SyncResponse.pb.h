// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: SyncResponse.proto

#ifndef PROTOBUF_SyncResponse_2eproto__INCLUDED
#define PROTOBUF_SyncResponse_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace quadtreesync {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_SyncResponse_2eproto();
void protobuf_AssignDesc_SyncResponse_2eproto();
void protobuf_ShutdownFile_SyncResponse_2eproto();

class ChunkData;
class SyncResponse;

// ===================================================================

class SyncResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:quadtreesync.SyncResponse) */ {
 public:
  SyncResponse();
  virtual ~SyncResponse();

  SyncResponse(const SyncResponse& from);

  inline SyncResponse& operator=(const SyncResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SyncResponse& default_instance();

  void Swap(SyncResponse* other);

  // implements Message ----------------------------------------------

  inline SyncResponse* New() const { return New(NULL); }

  SyncResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const SyncResponse& from);
  void MergeFrom(const SyncResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(SyncResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required bool hashKnown = 1;
  bool has_hashknown() const;
  void clear_hashknown();
  static const int kHashKnownFieldNumber = 1;
  bool hashknown() const;
  void set_hashknown(bool value);

  // required bool chunkData = 2;
  bool has_chunkdata() const;
  void clear_chunkdata();
  static const int kChunkDataFieldNumber = 2;
  bool chunkdata() const;
  void set_chunkdata(bool value);

  // required uint64 curHash = 3;
  bool has_curhash() const;
  void clear_curhash();
  static const int kCurHashFieldNumber = 3;
  ::google::protobuf::uint64 curhash() const;
  void set_curhash(::google::protobuf::uint64 value);

  // repeated .quadtreesync.ChunkData chunks = 4;
  int chunks_size() const;
  void clear_chunks();
  static const int kChunksFieldNumber = 4;
  const ::quadtreesync::ChunkData& chunks(int index) const;
  ::quadtreesync::ChunkData* mutable_chunks(int index);
  ::quadtreesync::ChunkData* add_chunks();
  ::google::protobuf::RepeatedPtrField< ::quadtreesync::ChunkData >*
      mutable_chunks();
  const ::google::protobuf::RepeatedPtrField< ::quadtreesync::ChunkData >&
      chunks() const;

  // optional uint32 treeLevel = 5;
  bool has_treelevel() const;
  void clear_treelevel();
  static const int kTreeLevelFieldNumber = 5;
  ::google::protobuf::uint32 treelevel() const;
  void set_treelevel(::google::protobuf::uint32 value);

  // repeated uint64 hashValues = 6;
  int hashvalues_size() const;
  void clear_hashvalues();
  static const int kHashValuesFieldNumber = 6;
  ::google::protobuf::uint64 hashvalues(int index) const;
  void set_hashvalues(int index, ::google::protobuf::uint64 value);
  void add_hashvalues(::google::protobuf::uint64 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >&
      hashvalues() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >*
      mutable_hashvalues();

  // @@protoc_insertion_point(class_scope:quadtreesync.SyncResponse)
 private:
  inline void set_has_hashknown();
  inline void clear_has_hashknown();
  inline void set_has_chunkdata();
  inline void clear_has_chunkdata();
  inline void set_has_curhash();
  inline void clear_has_curhash();
  inline void set_has_treelevel();
  inline void clear_has_treelevel();

  // helper for ByteSize()
  int RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  bool hashknown_;
  bool chunkdata_;
  ::google::protobuf::uint32 treelevel_;
  ::google::protobuf::uint64 curhash_;
  ::google::protobuf::RepeatedPtrField< ::quadtreesync::ChunkData > chunks_;
  ::google::protobuf::RepeatedField< ::google::protobuf::uint64 > hashvalues_;
  friend void  protobuf_AddDesc_SyncResponse_2eproto();
  friend void protobuf_AssignDesc_SyncResponse_2eproto();
  friend void protobuf_ShutdownFile_SyncResponse_2eproto();

  void InitAsDefaultInstance();
  static SyncResponse* default_instance_;
};
// -------------------------------------------------------------------

class ChunkData : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:quadtreesync.ChunkData) */ {
 public:
  ChunkData();
  virtual ~ChunkData();

  ChunkData(const ChunkData& from);

  inline ChunkData& operator=(const ChunkData& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ChunkData& default_instance();

  void Swap(ChunkData* other);

  // implements Message ----------------------------------------------

  inline ChunkData* New() const { return New(NULL); }

  ChunkData* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ChunkData& from);
  void MergeFrom(const ChunkData& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ChunkData* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required uint64 x = 1;
  bool has_x() const;
  void clear_x();
  static const int kXFieldNumber = 1;
  ::google::protobuf::uint64 x() const;
  void set_x(::google::protobuf::uint64 value);

  // required uint64 y = 2;
  bool has_y() const;
  void clear_y();
  static const int kYFieldNumber = 2;
  ::google::protobuf::uint64 y() const;
  void set_y(::google::protobuf::uint64 value);

  // required uint64 data = 3;
  bool has_data() const;
  void clear_data();
  static const int kDataFieldNumber = 3;
  ::google::protobuf::uint64 data() const;
  void set_data(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:quadtreesync.ChunkData)
 private:
  inline void set_has_x();
  inline void clear_has_x();
  inline void set_has_y();
  inline void clear_has_y();
  inline void set_has_data();
  inline void clear_has_data();

  // helper for ByteSize()
  int RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint64 x_;
  ::google::protobuf::uint64 y_;
  ::google::protobuf::uint64 data_;
  friend void  protobuf_AddDesc_SyncResponse_2eproto();
  friend void protobuf_AssignDesc_SyncResponse_2eproto();
  friend void protobuf_ShutdownFile_SyncResponse_2eproto();

  void InitAsDefaultInstance();
  static ChunkData* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// SyncResponse

// required bool hashKnown = 1;
inline bool SyncResponse::has_hashknown() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SyncResponse::set_has_hashknown() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SyncResponse::clear_has_hashknown() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SyncResponse::clear_hashknown() {
  hashknown_ = false;
  clear_has_hashknown();
}
inline bool SyncResponse::hashknown() const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.hashKnown)
  return hashknown_;
}
inline void SyncResponse::set_hashknown(bool value) {
  set_has_hashknown();
  hashknown_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.SyncResponse.hashKnown)
}

// required bool chunkData = 2;
inline bool SyncResponse::has_chunkdata() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void SyncResponse::set_has_chunkdata() {
  _has_bits_[0] |= 0x00000002u;
}
inline void SyncResponse::clear_has_chunkdata() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void SyncResponse::clear_chunkdata() {
  chunkdata_ = false;
  clear_has_chunkdata();
}
inline bool SyncResponse::chunkdata() const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.chunkData)
  return chunkdata_;
}
inline void SyncResponse::set_chunkdata(bool value) {
  set_has_chunkdata();
  chunkdata_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.SyncResponse.chunkData)
}

// required uint64 curHash = 3;
inline bool SyncResponse::has_curhash() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void SyncResponse::set_has_curhash() {
  _has_bits_[0] |= 0x00000004u;
}
inline void SyncResponse::clear_has_curhash() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void SyncResponse::clear_curhash() {
  curhash_ = GOOGLE_ULONGLONG(0);
  clear_has_curhash();
}
inline ::google::protobuf::uint64 SyncResponse::curhash() const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.curHash)
  return curhash_;
}
inline void SyncResponse::set_curhash(::google::protobuf::uint64 value) {
  set_has_curhash();
  curhash_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.SyncResponse.curHash)
}

// repeated .quadtreesync.ChunkData chunks = 4;
inline int SyncResponse::chunks_size() const {
  return chunks_.size();
}
inline void SyncResponse::clear_chunks() {
  chunks_.Clear();
}
inline const ::quadtreesync::ChunkData& SyncResponse::chunks(int index) const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.chunks)
  return chunks_.Get(index);
}
inline ::quadtreesync::ChunkData* SyncResponse::mutable_chunks(int index) {
  // @@protoc_insertion_point(field_mutable:quadtreesync.SyncResponse.chunks)
  return chunks_.Mutable(index);
}
inline ::quadtreesync::ChunkData* SyncResponse::add_chunks() {
  // @@protoc_insertion_point(field_add:quadtreesync.SyncResponse.chunks)
  return chunks_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::quadtreesync::ChunkData >*
SyncResponse::mutable_chunks() {
  // @@protoc_insertion_point(field_mutable_list:quadtreesync.SyncResponse.chunks)
  return &chunks_;
}
inline const ::google::protobuf::RepeatedPtrField< ::quadtreesync::ChunkData >&
SyncResponse::chunks() const {
  // @@protoc_insertion_point(field_list:quadtreesync.SyncResponse.chunks)
  return chunks_;
}

// optional uint32 treeLevel = 5;
inline bool SyncResponse::has_treelevel() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void SyncResponse::set_has_treelevel() {
  _has_bits_[0] |= 0x00000010u;
}
inline void SyncResponse::clear_has_treelevel() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void SyncResponse::clear_treelevel() {
  treelevel_ = 0u;
  clear_has_treelevel();
}
inline ::google::protobuf::uint32 SyncResponse::treelevel() const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.treeLevel)
  return treelevel_;
}
inline void SyncResponse::set_treelevel(::google::protobuf::uint32 value) {
  set_has_treelevel();
  treelevel_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.SyncResponse.treeLevel)
}

// repeated uint64 hashValues = 6;
inline int SyncResponse::hashvalues_size() const {
  return hashvalues_.size();
}
inline void SyncResponse::clear_hashvalues() {
  hashvalues_.Clear();
}
inline ::google::protobuf::uint64 SyncResponse::hashvalues(int index) const {
  // @@protoc_insertion_point(field_get:quadtreesync.SyncResponse.hashValues)
  return hashvalues_.Get(index);
}
inline void SyncResponse::set_hashvalues(int index, ::google::protobuf::uint64 value) {
  hashvalues_.Set(index, value);
  // @@protoc_insertion_point(field_set:quadtreesync.SyncResponse.hashValues)
}
inline void SyncResponse::add_hashvalues(::google::protobuf::uint64 value) {
  hashvalues_.Add(value);
  // @@protoc_insertion_point(field_add:quadtreesync.SyncResponse.hashValues)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >&
SyncResponse::hashvalues() const {
  // @@protoc_insertion_point(field_list:quadtreesync.SyncResponse.hashValues)
  return hashvalues_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >*
SyncResponse::mutable_hashvalues() {
  // @@protoc_insertion_point(field_mutable_list:quadtreesync.SyncResponse.hashValues)
  return &hashvalues_;
}

// -------------------------------------------------------------------

// ChunkData

// required uint64 x = 1;
inline bool ChunkData::has_x() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ChunkData::set_has_x() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ChunkData::clear_has_x() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ChunkData::clear_x() {
  x_ = GOOGLE_ULONGLONG(0);
  clear_has_x();
}
inline ::google::protobuf::uint64 ChunkData::x() const {
  // @@protoc_insertion_point(field_get:quadtreesync.ChunkData.x)
  return x_;
}
inline void ChunkData::set_x(::google::protobuf::uint64 value) {
  set_has_x();
  x_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.ChunkData.x)
}

// required uint64 y = 2;
inline bool ChunkData::has_y() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ChunkData::set_has_y() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ChunkData::clear_has_y() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ChunkData::clear_y() {
  y_ = GOOGLE_ULONGLONG(0);
  clear_has_y();
}
inline ::google::protobuf::uint64 ChunkData::y() const {
  // @@protoc_insertion_point(field_get:quadtreesync.ChunkData.y)
  return y_;
}
inline void ChunkData::set_y(::google::protobuf::uint64 value) {
  set_has_y();
  y_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.ChunkData.y)
}

// required uint64 data = 3;
inline bool ChunkData::has_data() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void ChunkData::set_has_data() {
  _has_bits_[0] |= 0x00000004u;
}
inline void ChunkData::clear_has_data() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void ChunkData::clear_data() {
  data_ = GOOGLE_ULONGLONG(0);
  clear_has_data();
}
inline ::google::protobuf::uint64 ChunkData::data() const {
  // @@protoc_insertion_point(field_get:quadtreesync.ChunkData.data)
  return data_;
}
inline void ChunkData::set_data(::google::protobuf::uint64 value) {
  set_has_data();
  data_ = value;
  // @@protoc_insertion_point(field_set:quadtreesync.ChunkData.data)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace quadtreesync

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_SyncResponse_2eproto__INCLUDED
