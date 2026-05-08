#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms_old/ant_bms_old.h"

namespace esphome::ant_bms_old::testing {

class TestableAntBmsOld : public AntBmsOld {
 public:
  void update() override {}
};

}  // namespace esphome::ant_bms_old::testing
