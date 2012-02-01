template <size_t SZ>
class RequestBuf {
  char requestbuf[SZ];
  int requestpos;
  char b;
  
  public:
  RequestBuf() : requestpos(0), b(0) { clearBuf(); }
  void clearBuf() { requestpos = 0; requestbuf[requestpos] = 0; }
  void append(int c) {
    if (c == -1 || requestpos >= SZ - 1) return;
    requestbuf[requestpos++] = c;
    requestbuf[requestpos] = 0; 
  }
  size_t size() const { return SZ; }
  size_t pos() const { return requestpos; }
  const char& operator[](size_t pos) const { if (pos >= 0 && pos < SZ) return requestbuf[pos]; else return b; }
  char& operator[](size_t pos) { if (pos >= 0 && pos < SZ) return requestbuf[pos]; else return b; }
  operator char *() { return requestbuf; }
  operator const char *() const { return requestbuf; }
};

