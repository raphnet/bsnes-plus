#define MODEM_LINEBUF_SIZE	128
class Modem {
public:
  Modem(void);
  bool hasData(void);
  uint8 readData(void);
  void writeData(uint8 data);
  bool canWrite(void);
  void hangup(void);

private:
  void processCommandBuffer(void);
  void answerCommand(const char *str);
  void addOutByte(uint8 dat);

  enum ModemState { Command, Connected };
  unsigned mode;
  bool echo_on;
  int atx;
  int connection_rate;
  int socketfd;

  unsigned answer_delay;

  // line buffer for at command mode
  uint8 lbuf[MODEM_LINEBUF_SIZE];
  unsigned lbuf_pos;

  // answer buffer to at command mode
  uint8 abuf[MODEM_LINEBUF_SIZE];
  unsigned abuf_head, abuf_tail;
};
