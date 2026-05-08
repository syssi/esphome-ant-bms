#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/ant_bms/ant_bms.h"

namespace esphome::ant_bms::testing {

class TestableAntBms : public AntBms {
 public:
  void update() override {}
};

}  // namespace esphome::ant_bms::testing
