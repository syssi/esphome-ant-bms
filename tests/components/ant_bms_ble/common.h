#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms_ble/ant_bms_ble.h"

namespace esphome::ant_bms_ble::testing {

class TestableAntBmsBle : public AntBmsBle {
 public:
  void update() override {}
};

}  // namespace esphome::ant_bms_ble::testing
