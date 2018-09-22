#include <snes.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MODEM_CPP
namespace SNES {

Modem::Modem()
{
	lbuf_pos = 0;
	abuf_head = abuf_tail = 0;
	mode = Command;
	echo_on = false;
	socketfd = -1;
}

void Modem::hangup(void)
{
	mode = Command;
	if (socketfd != -1) {
		close(socketfd);
		socketfd = -1;
	}
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
		  if (data != '\n')
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
	  printf("%02x ", data); fflush(stdout);
	  if (socketfd >= 0) {
		  if (send(socketfd, &data, 1, 0)<0) {
			  perror("send");
		  }
	  }
	  break;
  }
}

bool Modem::canWrite()
{
	return lbuf_pos < MODEM_LINEBUF_SIZE;
}

void Modem::processCommandBuffer(void)
{
	const char *OK = "\r\nOK\r\n";
	const char *ERROR = "\r\nERROR\r\n";

	if (strncmp((char*)lbuf, "AT", 2)) {
		printf("???: \"%s\"\n", (char*)lbuf);
		return;
	}

	printf("AT command: \"%s\"\n", (char*)lbuf);

	if (strcmp((char*)lbuf, "ATE1Q0V1")==0) {
		echo_on = true;
    	answerCommand(OK);
		return;
	}

	if (strcmp((char*)lbuf, "AT&F&W0&W1")==0) {
    	answerCommand(OK);
		return;
	}

	if (strcmp((char*)lbuf, "ATZ")==0) {
    	answerCommand(OK);
		return;
	}

	// ATP : Set pulse dialing
	if (strncmp((char*)lbuf, "ATP", 3)==0) {
		// note: JRA PAT sends ATP&P1 or ATP&P2 depending on line classification.
		// &P1: 100pps
		// &P2: 200pps
    	answerCommand(OK);
		return;
	}

	// ATT : Set tone dialing
	if (strncmp((char*)lbuf, "ATT", 3)==0) {
    	answerCommand(OK);
		return;
	}

	// ATS: Set S registers
	if (strncmp((char*)lbuf, "ATS", 3)==0) {
		int reg, value;
		if (2 == sscanf((char*)lbuf, "ATS%d=%d", &reg, &value)) {
			printf("Set S register %d to %d\n", reg, value);
    		answerCommand(OK);
		} else {
			answerCommand(ERROR);
		}
		return;
	}

	// ATL: Set speaker volume (0-3)
	if (strncmp((char*)lbuf, "ATL", 3)==0) {
    	answerCommand(OK);
		return;
	}

	// Error
	if (strncmp((char*)lbuf, "ATX", 3)==0) {
		if (1 == sscanf((char*)lbuf, "ATX%d", &atx)) {
    		answerCommand(OK);
		} else {
    		answerCommand(ERROR);
		}
		return;
	}

	// Proprietary command for choosing connection rate?
	if (strncmp((char*)lbuf, "AT%B", 4)==0) {
		if (1==sscanf((char*)lbuf, "AT%%B%d", &connection_rate)) {
			printf("Requesting connection baud rate %d\n", connection_rate);
    		answerCommand(OK);
		} else {
			answerCommand(ERROR);
		}
		return;
	}

	if (strcmp((char*)lbuf, "AT\\N0%C0")==0) {
    	answerCommand(OK);
		return;
	}

	if (strcmp((char*)lbuf, "AT\\N3%C0")==0) {
    	answerCommand(OK);
		return;
	}

	if (strcmp((char*)lbuf, "AT\\N3%C1")==0) {
    	answerCommand(OK);
		return;
	}

	// Dial
	if (strncmp((char*)lbuf, "ATD", 3)==0) {
		char connectStr[32];
		int res;
		struct sockaddr_in destination;

		memset(&destination, 0, sizeof(struct sockaddr_in));
		destination.sin_family = AF_INET;
		destination.sin_port = htons(5555);
		inet_aton("127.0.0.1", &destination.sin_addr);

		socketfd = socket(AF_INET, SOCK_STREAM, 0);
		if (socketfd == -1) {
			perror("socket");
			answerCommand("\r\nBUSY\r\n");
			return;
		}

		res = connect(socketfd, (struct sockaddr *)&destination, sizeof(struct sockaddr));
		if (res == -1) {
			perror("connect");
			answerCommand("\r\nBUSY\r\n");
			return;
		}

		snprintf(connectStr, 32, "\r\nCONNECT %d\r\n", connection_rate);
		answerCommand(connectStr);
		mode = Connected;
		return;
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
