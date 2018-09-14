#include <snes.hpp>

#define MODEM_CPP
namespace SNES {

Modem::Modem()
{
	lbuf_pos = 0;
	abuf_head = abuf_tail = 0;
	mode = Command;
	echo_on = false;
}

bool Modem::hasData(void)
{
	if (answer_delay) {
		answer_delay--;
		return false;
	}
	return abuf_head != abuf_tail;
}

uint8 Modem::readData(void)
{
	uint8 b = 0xff;

	if (hasData()) {
      b = abuf[abuf_tail];
	  abuf_tail++;
	  if (abuf_tail >= MODEM_LINEBUF_SIZE) {
		  abuf_tail = 0;
	  }
	}
	return b;
}

void Modem::writeData(uint8 data)
{
  switch (mode) {
    case Command:
      if (lbuf_pos >= MODEM_LINEBUF_SIZE) {
        printf("modem rx buffer full\n");
        return;
      }

	  if (echo_on) {
		  addOutByte(data);
	  }

      lbuf[lbuf_pos] = data;
	  if (data == 0x0d)
		  return;

      if (data == 0x0a) {
		lbuf[lbuf_pos] = 0;
	    processCommandBuffer();
		lbuf_pos = 0;
      } else {
		lbuf_pos++;
	  }
	  break;

    case Connected:
	  printf("%02x", data); fflush(stdout);
	  // todo : send through a socket?
	  break;
  }
}

bool Modem::canWrite()
{
	return lbuf_pos < MODEM_LINEBUF_SIZE;
}

void Modem::processCommandBuffer(void)
{
	printf("AT command: \"%s\"\n", (char*)lbuf);

	if (strcmp((char*)lbuf, "ATE1Q0V1")==0) {
		if (!echo_on) {
          answerCommand("\r\nOK\r\n");
		  echo_on = true;
        } else {
		  answerCommand("OK\r\n");
        }
	}
}

void Modem::addOutByte(uint8 dat)
{
	abuf[abuf_head] = dat;
	abuf_head++;
	if (abuf_head >= MODEM_LINEBUF_SIZE)
		abuf_head = 0;

	answer_delay = 1;
}

void Modem::answerCommand(const char *str)
{
	printf("Reply: ", str);
	while (*str) {
		if (*str >= 32) {
			printf("%c", *str);
		} else {
			printf("<%02x>", *str);
		}

		addOutByte(*str);
		str++;
	}
	printf("\n");
}

}
