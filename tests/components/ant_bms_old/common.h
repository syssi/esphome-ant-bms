#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms_old/ant_bms_old.h"
#include "esphome/components/switch/switch.h"

namespace esphome::ant_bms_old::testing {

class TestableAntBmsOld : public AntBmsOld {
 public:
  void update() override {}
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::ant_bms_old::testing
