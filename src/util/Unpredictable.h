#pragma once

class Unpredictable {
public:
  // Takes unpredictable boolean-producing values, each unpredictable in itself,
  // and returns a single ORed bool if any are true.
  template <typename... Args>
  [[nodiscard]] static constexpr bool any(Args &&...args) noexcept {
    return (static_cast<unsigned>(!!args) | ...);
  }
  [[nodiscard]] static bool any2(bool x1, bool x2) noexcept {
    bool result = x1;
    asm volatile(
        "or %2, %0"
        : "=r"(result)
        : "r"(result), "r"(x2)
        );
    return result;
  }
  // Takes unpredictable boolean-producing values, each unpredictable in itself,
  // and returns a single ANDed bool if any are true.
  template <typename... Args>
  [[nodiscard]] static constexpr bool all(Args &&...args) noexcept {
    return (static_cast<unsigned>(!!args) & ...);
  }
};
