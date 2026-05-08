#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms/ant_bms.h"
#include "esphome/components/switch/switch.h"

namespace esphome::ant_bms::testing {

class TestableAntBms : public AntBms {
 public:
  void update() override {}
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::ant_bms::testing
