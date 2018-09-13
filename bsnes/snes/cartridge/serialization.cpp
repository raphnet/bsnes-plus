#ifdef CARTRIDGE_CPP

void Cartridge::serialize(serializer &s) {
  if(memory::cartram.size() != 0) {
    s.array(memory::cartram.data(), memory::cartram.size());
  }

  if(memory::cartrtc.size() != 0) {
    s.array(memory::cartrtc.data(), memory::cartrtc.size());
  }

  if(memory::bsxpram.size() != 0) {
    s.array(memory::bsxpram.data(), memory::bsxpram.size());
  }

  if(memory::stAram.size() != 0) {
    s.array(memory::stAram.data(), memory::stAram.size());
  }

  if(memory::stBram.size() != 0) {
    s.array(memory::stBram.data(), memory::stBram.size());
  }

  if(memory::gbram.size() != 0) {
    s.array(memory::gbram.data(), memory::gbram.size());
  }

  if(memory::gbrtc.size() != 0) {
    s.array(memory::gbrtc.data(), memory::gbrtc.size());
  }

  if(memory::cartflash.size() != 0) {
    s.array(memory::cartflash.data(), memory::cartflash.size());
  }
}

#endif
