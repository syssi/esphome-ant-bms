#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms_old_ble/ant_bms_old_ble.h"

namespace esphome::ant_bms_old_ble::testing {

class TestableAntBmsOldBle : public AntBmsOldBle {
 public:
  void update() override {}
};

}  // namespace esphome::ant_bms_old_ble::testing
