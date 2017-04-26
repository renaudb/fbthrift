/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _THRIFT_TRANSPORT_TFILETRANSPORT_H_
#define _THRIFT_TRANSPORT_TFILETRANSPORT_H_ 1

#include <thrift/lib/cpp/transport/TTransport.h>
#include <thrift/lib/cpp/Thrift.h>

#include <string>
#include <stdio.h>

#include <boost/scoped_ptr.hpp>
#include <folly/portability/PThread.h>
#include <memory>

namespace apache { namespace thrift {
  class TProcessor;
  namespace protocol {
    class TProtocolFactory;
  }
}}

namespace apache { namespace thrift { namespace transport {

// Data pertaining to a single event
typedef struct eventInfo {
  uint8_t* eventBuff_;
  uint32_t eventSize_;
  uint32_t eventBuffPos_;

  eventInfo():eventBuff_(nullptr), eventSize_(0), eventBuffPos_(0){};
  ~eventInfo() {
    delete[] eventBuff_;
  }
} eventInfo;

// information about current read state
typedef struct readState {
  eventInfo* event_;

  // keep track of event size
  uint8_t   eventSizeBuff_[4];
  uint8_t   eventSizeBuffPos_;
  bool      readingSize_;

  // read buffer variables
  int32_t  bufferPtr_;
  int32_t  bufferLen_;

  // last successful dispatch point
  int32_t lastDispatchPtr_;

  void resetState(uint32_t lastDispatchPtr) {
    readingSize_ = true;
    eventSizeBuffPos_ = 0;
    lastDispatchPtr_ = lastDispatchPtr;
  }

  void resetAllValues() {
    resetState(0);
    bufferPtr_ = 0;
    bufferLen_ = 0;
    if (event_) {
      delete(event_);
    }
    event_ = 0;
  }

  readState() {
    event_ = 0;
   resetAllValues();
  }

  ~readState() {
    if (event_) {
      delete(event_);
    }
  }

} readState;

/**
 * TFileTransportBuffer - buffer class used by TFileTransport for queuing up events
 * to be written to disk.  Should be used in the following way:
 *  1) Buffer created
 *  2) Buffer written to (addEvent)
 *  3) Buffer read from (getNext)
 *  4) Buffer reset (reset)
 *  5) Go back to 2, or destroy buffer
 *
 * The buffer should never be written to after it is read from, unless it is reset first.
 * Note: The above rules are enforced mainly for debugging its sole client TFileTransport
 *       which uses the buffer in this way.
 *
 */
class TFileTransportBuffer {
  public:
    explicit TFileTransportBuffer(uint32_t size);
    ~TFileTransportBuffer();

    bool addEvent(eventInfo *event);
    eventInfo* getNext();
    void reset();
    bool isFull();
    bool isEmpty();

  private:
    TFileTransportBuffer(); // should not be used

    enum mode {
      BUF_WRITE,
      BUF_READ,
    };
    mode bufferMode_;

    uint32_t writePoint_;
    uint32_t readPoint_;
    uint32_t size_;
    eventInfo** buffer_;
};

/**
 * Abstract interface for transports used to read files
 */
class TFileReaderTransport : virtual public TTransport {
 public:
  virtual int32_t getReadTimeout() = 0;
  virtual void setReadTimeout(int32_t readTimeout) = 0;

  virtual uint32_t getNumChunks() = 0;
  virtual uint32_t getCurChunk() = 0;
  virtual void seekToChunk(int32_t chunk) = 0;
  virtual void seekToEnd() = 0;
};

/**
 * Abstract interface for transports used to write files
 */
class TFileWriterTransport : virtual public TTransport {
 public:
  virtual uint32_t getChunkSize() = 0;
  virtual void setChunkSize(uint32_t chunkSize) = 0;
};

/**
 * File implementation of a transport. Reads and writes are done to a
 * file on disk.
 *
 */
class TFileTransport : public TFileReaderTransport,
                       public TFileWriterTransport {
 public:
  explicit TFileTransport(std::string path, bool readOnly=false);
  ~TFileTransport() override;

  // TODO: what is the correct behavior for this?
  // the log file is generally always open
  bool isOpen() override { return true; }

  void write(const uint8_t* buf, uint32_t len);
  void flush() override;

  uint32_t readAll(uint8_t* buf, uint32_t len);
  bool peek() override;
  uint32_t read(uint8_t* buf, uint32_t len);

  // log-file specific functions
  void seekToChunk(int32_t chunk) override;
  void seekToEnd() override;
  uint32_t getNumChunks() override;
  uint32_t getCurChunk() override;

  // for changing the output file
  void resetOutputFile(int fd, std::string filename, int64_t offset);

  // Setter/Getter functions for user-controllable options
  void setReadBuffSize(uint32_t readBuffSize) {
    if (readBuffSize) {
      readBuffSize_ = readBuffSize;
    }
  }
  uint32_t getReadBuffSize() {
    return readBuffSize_;
  }

  static const int32_t TAIL_READ_TIMEOUT = -1;
  static const int32_t NO_TAIL_READ_TIMEOUT = 0;
  void setReadTimeout(int32_t readTimeout) override {
    readTimeout_ = readTimeout;
  }
  int32_t getReadTimeout() override { return readTimeout_; }

  void setChunkSize(uint32_t chunkSize) override {
    if (chunkSize) {
      chunkSize_ = chunkSize;
    }
  }
  uint32_t getChunkSize() override { return chunkSize_; }

  void setEventBufferSize(uint32_t bufferSize) {
    if (bufferAndThreadInitialized_) {
      GlobalOutput("Cannot change the buffer size after writer thread started");
      return;
    }
    eventBufferSize_ = bufferSize;
  }

  uint32_t getEventBufferSize() {
    return eventBufferSize_;
  }

  void setFlushMaxUs(uint32_t flushMaxUs) {
    if (flushMaxUs) {
      flushMaxUs_ = flushMaxUs;
    }
  }
  uint32_t getFlushMaxUs() {
    return flushMaxUs_;
  }

  void setFlushMaxBytes(uint32_t flushMaxBytes) {
    if (flushMaxBytes) {
      flushMaxBytes_ = flushMaxBytes;
    }
  }
  uint32_t getFlushMaxBytes() {
    return flushMaxBytes_;
  }

  void setMaxEventSize(uint32_t maxEventSize) {
    maxEventSize_ = maxEventSize;
  }
  uint32_t getMaxEventSize() {
    return maxEventSize_;
  }

  void setMaxCorruptedEvents(uint32_t maxCorruptedEvents) {
    maxCorruptedEvents_ = maxCorruptedEvents;
  }
  uint32_t getMaxCorruptedEvents() {
    return maxCorruptedEvents_;
  }

  void setEofSleepTimeUs(uint32_t eofSleepTime) {
    if (eofSleepTime) {
      eofSleepTime_ = eofSleepTime;
    }
  }
  uint32_t getEofSleepTimeUs() {
    return eofSleepTime_;
  }

  /*
   * Override TTransport *_virt() functions to invoke our implementations.
   * We cannot use TVirtualTransport to provide these, since we need to inherit
   * virtually from TTransport.
   */
  uint32_t read_virt(uint8_t* buf, uint32_t len) override {
    return this->read(buf, len);
  }
  uint32_t readAll_virt(uint8_t* buf, uint32_t len) override {
    return this->readAll(buf, len);
  }
  void write_virt(const uint8_t* buf, uint32_t len) override {
    this->write(buf, len);
  }

 private:
  // helper functions for writing to a file
  void enqueueEvent(const uint8_t* buf, uint32_t eventLen);
  bool swapEventBuffers(struct timespec* deadline);
  bool initBufferAndWriteThread();

  // control for writer thread
  static void* startWriterThread(void* ptr) {
    (((TFileTransport*)ptr)->writerThread());
    return 0;
  }
  void writerThread();

  // helper functions for reading from a file
  eventInfo* readEvent();

  // event corruption-related functions
  bool isEventCorrupted();
  void performRecovery();

  // Utility functions
  void openLogFile();
  void getNextFlushTime(struct timespec* ts_next_flush);

  // Class variables
  readState readState_;
  uint8_t* readBuff_;
  eventInfo* currentEvent_;

  uint32_t readBuffSize_;
  static const uint32_t DEFAULT_READ_BUFF_SIZE = 1 * 1024 * 1024;

  int32_t readTimeout_;
  static const int32_t DEFAULT_READ_TIMEOUT_MS = 200;

  // size of chunks that file will be split up into
  uint32_t chunkSize_;
  static const uint32_t DEFAULT_CHUNK_SIZE = 16 * 1024 * 1024;

  // size of event buffers
  uint32_t eventBufferSize_;
  static const uint32_t DEFAULT_EVENT_BUFFER_SIZE = 10000;

  // max number of microseconds that can pass without flushing
  uint32_t flushMaxUs_;
  static const uint32_t DEFAULT_FLUSH_MAX_US = 3000000;

  // max number of bytes that can be written without flushing
  uint32_t flushMaxBytes_;
  static const uint32_t DEFAULT_FLUSH_MAX_BYTES = 1000 * 1024;

  // max event size
  uint32_t maxEventSize_;
  static const uint32_t DEFAULT_MAX_EVENT_SIZE = 0;

  // max number of corrupted events per chunk
  uint32_t maxCorruptedEvents_;
  static const uint32_t DEFAULT_MAX_CORRUPTED_EVENTS = 0;

  // sleep duration when EOF is hit
  uint32_t eofSleepTime_;
  static const uint32_t DEFAULT_EOF_SLEEP_TIME_US = 500 * 1000;

  // sleep duration when a corrupted event is encountered
  static const uint32_t DEFAULT_CORRUPTED_SLEEP_TIME_US = 1 * 1000 * 1000;

  // sleep duration in seconds when an IO error is encountered in the writer thread
  uint32_t writerThreadIOErrorSleepTime_;
  static const uint32_t DEFAULT_WRITER_THREAD_SLEEP_TIME_US = 60 * 1000 * 1000;

  // writer thread id
  pthread_t writerThreadId_;

  // buffers to hold data before it is flushed. Each element of the buffer stores a msg that
  // needs to be written to the file.  The buffers are swapped by the writer thread.
  TFileTransportBuffer *dequeueBuffer_;
  TFileTransportBuffer *enqueueBuffer_;

  // conditions used to block when the buffer is full or empty
  pthread_cond_t notFull_, notEmpty_;
  volatile bool closing_;

  // To keep track of whether the buffer has been flushed
  pthread_cond_t flushed_;
  bool forceFlush_;

  // Mutex that is grabbed when enqueuing and swapping the read/write buffers
  pthread_mutex_t mutex_;

  // File information
  std::string filename_;
  int fd_;

  // Whether the writer thread and buffers have been initialized
  bool bufferAndThreadInitialized_;

  // Offset within the file
  off_t offset_;

  // event corruption information
  uint32_t lastBadChunk_;
  uint32_t numCorruptedEventsInChunk_;

  bool readOnly_;
};

// Exception thrown when EOF is hit
class TEOFException : public TTransportException {
 public:
  TEOFException():
    TTransportException(TTransportException::END_OF_FILE) {};
};


// wrapper class to process events from a file containing thrift events
class TFileProcessor {
 public:
  /**
   * Constructor that defaults output transport to null transport
   *
   * @param processor processes log-file events
   * @param protocolFactory protocol factory
   * @param inputTransport file transport
   */
  TFileProcessor(std::shared_ptr<apache::thrift::TProcessor> processor,
                 std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory,
                 std::shared_ptr<TFileReaderTransport> inputTransport);

  TFileProcessor(std::shared_ptr<apache::thrift::TProcessor> processor,
                 std::shared_ptr<apache::thrift::protocol::TProtocolFactory> inputProtocolFactory,
                 std::shared_ptr<apache::thrift::protocol::TProtocolFactory> outputProtocolFactory,
                 std::shared_ptr<TFileReaderTransport> inputTransport);

  /**
   * Constructor
   *
   * @param processor processes log-file events
   * @param protocolFactory protocol factory
   * @param inputTransport input file transport
   * @param output output transport
   */
  TFileProcessor(std::shared_ptr<apache::thrift::TProcessor> processor,
                 std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory,
                 std::shared_ptr<TFileReaderTransport> inputTransport,
                 std::shared_ptr<TTransport> outputTransport);

  /**
   * processes events from the file
   *
   * @param numEvents number of events to process (0 for unlimited)
   * @param tail tails the file if true
   */
  void process(uint32_t numEvents, bool tail);

  /**
   * process events until the end of the chunk
   *
   */
  void processChunk();

 private:
  std::shared_ptr<apache::thrift::TProcessor> processor_;
  std::shared_ptr<apache::thrift::protocol::TProtocolFactory> inputProtocolFactory_;
  std::shared_ptr<apache::thrift::protocol::TProtocolFactory> outputProtocolFactory_;
  std::shared_ptr<TFileReaderTransport> inputTransport_;
  std::shared_ptr<TTransport> outputTransport_;
};


}}} // apache::thrift::transport

#endif // _THRIFT_TRANSPORT_TFILETRANSPORT_H_
