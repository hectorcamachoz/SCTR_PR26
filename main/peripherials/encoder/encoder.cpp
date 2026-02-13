#include "encoder.h"

Encoder::Encoder(unsigned char _pinA, unsigned char _pinB)
    : pinA(_pinA), pinB(_pinB) {
  // Config Limits
  pcnt_unit_config_t unit_config = {};
  unit_config.high_limit = 32767;
  unit_config.low_limit = -32768;
  unit_config.intr_priority = 1;
  unit_config.flags.accum_count = 1;
  // Create counter unit
  pcnt_new_unit(&unit_config, &pcnt_unit);

  pcnt_glitch_filter_config_t filter_config = {
      .max_glitch_ns = 1000,
  };
  pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);

  // Set channel A
  pcnt_chan_config_t chan_a_config = {
      .edge_gpio_num = pinA,
      .level_gpio_num = pinB,
  };
  pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a);

  // Set channel B
  pcnt_chan_config_t chan_b_config = {
      .edge_gpio_num = pinB,
      .level_gpio_num = pinA,
  };
  pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b);

  // Set edges
  pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                               PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                               PCNT_CHANNEL_EDGE_ACTION_DECREASE);
  pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

  pcnt_unit_add_watch_point(pcnt_unit, -32768);
  pcnt_unit_add_watch_point(pcnt_unit, 32767);

  pcnt_unit_enable(pcnt_unit);
  pcnt_unit_clear_count(pcnt_unit);
  pcnt_unit_start(pcnt_unit);
}

int Encoder::get_count() {
  int tempCount{0};
  pcnt_unit_get_count(pcnt_unit, &tempCount);
  count = static_cast<int32_t>(tempCount);
  // printf("Count: %ld\n", count);
  return count;
}

Encoder::~Encoder() {
  pcnt_unit_stop(pcnt_unit);
  pcnt_unit_disable(pcnt_unit);
}
