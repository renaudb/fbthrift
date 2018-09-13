/*
 * Copyright 2018-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <limits>
#include <stdexcept>
#include <utility>

#include <folly/CPortability.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/lang/Exception.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

namespace apache {
namespace thrift {
namespace rocket {

class Serializer;

class SetupFrame {
 public:
  explicit SetupFrame(folly::IOBuf&& frame);

  explicit SetupFrame(Payload&& payload) : payload_(std::move(payload)) {}

  static constexpr FrameType frameType() {
    return FrameType::SETUP;
  }

  size_t frameHeaderSize() const {
    size_t frameSize = 20 + 2 * kMimeType.size();
    if (hasResumeIdentificationToken()) {
      frameSize +=
          2 /* bytes for token length */ + resumeIdentificationToken_.size();
    }
    return frameSize;
  }

  bool hasResumeIdentificationToken() const noexcept {
    return flags_.resumeToken();
  }

  bool hasLease() const noexcept {
    return flags_.lease();
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  void serialize(Serializer& writer) const;

 private:
  static constexpr folly::StringPiece kMimeType{"text/plain"};

  // Resume ID token and Lease flags are not currently supported/used.
  Flags flags_{Flags::none()};
  std::string resumeIdentificationToken_;
  Payload payload_;
};

class RequestResponseFrame {
 public:
  explicit RequestResponseFrame(folly::IOBuf&& frame);

  RequestResponseFrame(StreamId streamId, Payload&& payload)
      : streamId_(streamId), payload_(std::move(payload)) {}

  static constexpr FrameType frameType() {
    return FrameType::REQUEST_RESPONSE;
  }

  static constexpr size_t frameHeaderSize() {
    return 6;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  bool hasFollows() const noexcept {
    return flags_.follows();
  }
  void setHasFollows(bool hasFollows) noexcept {
    flags_.follows(hasFollows);
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_{Flags::none()};
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) const;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestFnfFrame {
 public:
  explicit RequestFnfFrame(folly::IOBuf&& frame);

  RequestFnfFrame(StreamId streamId, Payload&& payload)
      : streamId_(streamId), payload_(std::move(payload)) {}

  static constexpr size_t frameHeaderSize() {
    return 6;
  }

  static constexpr FrameType frameType() {
    return FrameType::REQUEST_FNF;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  bool hasFollows() const noexcept {
    return flags_.follows();
  }
  void setHasFollows(bool hasFollows) noexcept {
    flags_.follows(hasFollows);
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_{Flags::none()};
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) const;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestStreamFrame {
 public:
  explicit RequestStreamFrame(folly::IOBuf&& frame);

  RequestStreamFrame(
      StreamId streamId,
      Payload&& payload,
      int32_t initialRequestN)
      : streamId_(streamId),
        initialRequestN_(initialRequestN),
        payload_(std::move(payload)) {
    if (initialRequestN_ <= 0) {
      folly::throw_exception<std::logic_error>("initialRequestN MUST be > 0");
    }
  }

  static constexpr FrameType frameType() {
    return FrameType::REQUEST_STREAM;
  }

  static constexpr size_t frameHeaderSize() {
    return 10;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  bool hasFollows() const noexcept {
    return flags_.follows();
  }
  void setHasFollows(bool hasFollows) noexcept {
    flags_.follows(hasFollows);
  }

  int32_t initialRequestN() const noexcept {
    return initialRequestN_;
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  int32_t initialRequestN_;
  Flags flags_{Flags::none()};
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) const;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class RequestNFrame {
 public:
  explicit RequestNFrame(folly::IOBuf&& frame);

  RequestNFrame(StreamId streamId, int32_t n)
      : streamId_(streamId), requestN_(n) {
    if (requestN_ <= 0) {
      folly::throw_exception<std::logic_error>("REQUEST_N value MUST be > 0");
    }
  }

  static constexpr FrameType frameType() {
    return FrameType::REQUEST_N;
  }

  static constexpr size_t frameHeaderSize() {
    return 10;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  int32_t requestN() const noexcept {
    return requestN_;
  }

  void serialize(Serializer& writer) const;

 private:
  StreamId streamId_;
  int32_t requestN_;
};

class CancelFrame {
 public:
  explicit CancelFrame(folly::IOBuf&& frame);

  explicit CancelFrame(StreamId streamId) : streamId_(streamId) {}

  static constexpr FrameType frameType() {
    return FrameType::CANCEL;
  }

  static constexpr size_t frameHeaderSize() {
    return 6;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  void serialize(Serializer& writer) const;

 private:
  StreamId streamId_;
};

class PayloadFrame {
 public:
  explicit PayloadFrame(folly::IOBuf&& frame);

  PayloadFrame(StreamId streamId, Payload&& payload, Flags flags)
      : streamId_(streamId), flags_(flags), payload_(std::move(payload)) {}

  static constexpr FrameType frameType() {
    return FrameType::PAYLOAD;
  }

  static constexpr size_t frameHeaderSize() {
    return 6;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  bool hasFollows() const noexcept {
    return flags_.follows();
  }
  void setHasFollows(bool hasFollows) noexcept {
    flags_.follows(hasFollows);
  }
  bool hasComplete() const noexcept {
    return flags_.complete();
  }
  bool hasNext() const noexcept {
    return flags_.next();
  }

  void serialize(Serializer& writer) &&;

 private:
  StreamId streamId_;
  Flags flags_{Flags::none()};
  Payload payload_;

  void serializeIntoSingleFrame(Serializer& writer) const;
  FOLLY_NOINLINE void serializeInFragmentsSlow(Serializer& writer) &&;
};

class ErrorFrame {
 public:
  explicit ErrorFrame(folly::IOBuf&& frame);

  ErrorFrame(StreamId streamId, ErrorCode errorCode, Payload&& payload)
      : streamId_(streamId),
        errorCode_(errorCode),
        payload_(std::move(payload)) {}

  static constexpr FrameType frameType() {
    return FrameType::ERROR;
  }

  static constexpr size_t frameHeaderSize() {
    return 10;
  }

  StreamId streamId() const noexcept {
    return streamId_;
  }

  ErrorCode errorCode() const noexcept {
    return errorCode_;
  }

  const Payload& payload() const noexcept {
    return payload_;
  }
  Payload& payload() noexcept {
    return payload_;
  }

  void serialize(Serializer& writer) const;

 private:
  StreamId streamId_;
  ErrorCode errorCode_;
  Payload payload_;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
