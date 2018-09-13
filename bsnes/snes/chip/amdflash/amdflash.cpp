#include <snes.hpp>

#define AMDFLASH_CPP
namespace SNES {

AmdFlash amdflash;

#include "serialization.cpp"

void AmdFlash::init() {
  printf("**stub** AmdFlash init\n");
}

void AmdFlash::enable() {
  printf("AmdFlash enable\n");
}

void AmdFlash::power() {
  reset();
}

void AmdFlash::reset() {
  printf("AmdFlash reset\n");
  flash_state = StateNormal;
  step = 0;
}


uint8 AmdFlash::read(unsigned addr)
{
  addr &= 0x1FFFF;

  switch (flash_state)
  {
	  default:
	    flash_state = StateNormal;
	  case StateNormal:
	  	return memory::cartflash.read(addr);

	  case StateId:
	  	if (addr == 0x0000) {
			return 0x01; // manufacturer
		}
		if (addr == 0x0001) {
			return 0x20;
		}
		return 0xff; // not sure

	  case StateEraseAll:
		flash_state = StateNormal;
	    return 0x80; // instantaneous!

	  case StateEraseSector:
		flash_state = StateNormal;
		return 0x80; // instantaneous!

      case StateWriteByte:
        // Again, no attempt to provide realistic timing.
		flash_state = StateNormal;
        return (dta & 0x80);
  }

}

void AmdFlash::write(unsigned addr, uint8 data)
{
  int i;
  addr &= 0x1FFFF;

  if (flash_state == StateWriteByte) {
    dta = data;
    memory::cartflash.write(addr, data);
	flash_state = StateNormal;
	return;
  }

  if (step == 1) {
	if ((addr == 0x2AAA) && (data == 0x55)) {
      step++;
	  return;
	}
    else
      step = 0;
  }
  if ((step == 0) && (addr == 0x5555) && (data == 0xAA)) {
    step++;
	return;
  };

  if (step < 2)
	  return;

  step = 0;

  switch (data)
  {
    default:
      printf("unknown flash command 0x%02x\n", data);
      flash_state = StateNormal;
      break;

    case 0x90: // enter ID mode
      if (addr != 0x5555) {
        break;
      }
      flash_state = StateId;
	  break;

    case 0xF0: // end / return to normal mode
      flash_state = StateNormal;
      break;

    case 0x80: // prepare erase (unlock?)
      if (addr != 0x5555) { step = 0; break; }
      flash_state = StatePrepareErase;
	  break;

     case 0x10: // erase all
       if (addr != 0x5555) { step = 0; break; }
	   for (i=0; i<0x20000; i++)
         memory::cartflash.write(addr+i, 0xFF);
       flash_state = StateEraseAll;
       break;

     case 0x30: // erase 16kb sector
	   for (i=0; i<0x4000; i++)
		 memory::cartflash.write((addr&0x1C000)+i, 0xFF);
       flash_state = StateEraseSector;
	   break;

     case 0xA0: // write byte
       if (addr != 0x5555) { step = 0; break; }
	   flash_state = StateWriteByte;
	   break;
  }
}

AmdFlash::AmdFlash() {
}

}
