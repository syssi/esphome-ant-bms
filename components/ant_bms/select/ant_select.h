#pragma once

#include "../ant_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/select/select.h"

namespace esphome::ant_bms {

class AntSelect : public select::Select, public Component {
 public:
  void set_parent(AntBms *parent) { this->parent_ = parent; }
  void set_select_mappings(std::vector<uint16_t> mappings) { this->mappings_ = std::move(mappings); }
  void dump_config() override;

 protected:
  void control(const std::string &value) override;
  std::vector<uint16_t> mappings_;
  AntBms *parent_;
};

}  // namespace esphome::ant_bms
