class AmdFlash  : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);
  AmdFlash();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

private:
  enum FlashState { StateNormal, StateId, StatePrepareErase, StateEraseAll, StateEraseSector, StateWriteByte };
  unsigned flash_state;
  unsigned step;
  uint8 dta;
/*  static const unsigned months[12];
  enum RtcMode { RtcReady, RtcCommand, RtcRead, RtcWrite };
  unsigned rtc_mode;
  signed rtc_index;

  void update_time();
  unsigned weekday(unsigned year, unsigned month, unsigned day);*/

};

extern AmdFlash amdflash;
