#pragma once

#include <atomic>
#include <cstdint>

namespace savage::sync {

class StateHub {
 public:
  static StateHub& instance();

  void request_shutdown() noexcept { shutdown_.store(true, std::memory_order_release); }
  [[nodiscard]] bool shutdown_requested() const noexcept {
    return shutdown_.load(std::memory_order_acquire);
  }

  void set_menu_visible(bool v) noexcept { menu_visible_.store(v, std::memory_order_relaxed); }
  [[nodiscard]] bool menu_visible() const noexcept { return menu_visible_.load(std::memory_order_relaxed); }

  void set_frame_counter(std::uint64_t n) noexcept { frames_.store(n, std::memory_order_relaxed); }
  [[nodiscard]] std::uint64_t frame_counter() const noexcept { return frames_.load(std::memory_order_relaxed); }

 private:
  std::atomic<bool> shutdown_{false};
  std::atomic<bool> menu_visible_{true};
  std::atomic<std::uint64_t> frames_{0};
};

}  // namespace savage::sync