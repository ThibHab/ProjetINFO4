#pragma once

namespace dhe {
namespace envelope {

template <typename Signals> class InputMode {
public:
  InputMode(Signals &signals) : signals_{signals} {}

  void enter() { signals_.show_active(false); }
  void execute() { signals_.output(signals_.input()); }
  void exit() { signals_.show_active(false); }

private:
  Signals &signals_;
};
} // namespace envelope
} // namespace dhe
