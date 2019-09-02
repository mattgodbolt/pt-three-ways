#include "Metric.h"

#include <algorithm>

Metric::Metric(const char *name) {
  Info info{std::string(name)};
  withMetricData(
      [&](MetricData &data) { data.activeMetrics_.emplace(this, info); });
}

Metric::~Metric() {
  withMetricData([&](MetricData &data) {
    auto name = data.activeMetrics_.at(this).name;
    data.defunctMetrics_[name] += count_;
    data.activeMetrics_.erase(this);
  });
}

std::vector<Metric::Snapshot> Metric::all() {
  std::unordered_map<std::string, size_t> result;
  withMetricData([&](MetricData &data) {
    result = data.defunctMetrics_;
    for (auto &&active : data.activeMetrics_) {
      result[active.second.name] +=
          active.first->count_.load(std::memory_order_relaxed);
    }
  });

  std::vector<Snapshot> toReturn;
  for (auto &&entry : result) {
    toReturn.emplace_back(Snapshot{entry.first, entry.second});
  }
  std::sort(toReturn.begin(), toReturn.end(),
            [](const Snapshot &lhs, const Snapshot &rhs) {
              return lhs.name < rhs.name;
            });
  return toReturn;
}

template <typename F>
void Metric::withMetricData(F &&func) {
  static MetricData theData;
  std::lock_guard lock(theData.mutex);
  func(theData);
}
