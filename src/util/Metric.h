#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Metric {
  struct Info {
    std::string name;
  };

  struct MetricData {
    std::mutex mutex;
    std::unordered_map<const Metric *, Info> activeMetrics_;
    std::unordered_map<std::string, size_t> defunctMetrics_;
  };

  template <typename F>
  static void withMetricData(F &&func);

  std::atomic<size_t> count_;

public:
  explicit Metric(const char *name);
  ~Metric();

  Metric(const Metric &) = delete;
  Metric &operator=(const Metric &) = delete;
  Metric(Metric &&) = delete;
  Metric &operator=(Metric &&) = delete;

  inline void increment() {
    count_.store(count_.load(std::memory_order_relaxed) + 1,
                 std::memory_order_relaxed);
  }

  struct Snapshot {
    std::string name;
    size_t count;
  };

  static std::vector<Snapshot> all();
};

static_assert(sizeof(Metric) == sizeof(size_t), "Unexpectedly large metric");