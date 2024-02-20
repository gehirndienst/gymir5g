/*
 * Statistics.h
 *
 * classes that counts and calculates statistics for the cathegorical or numerical values as well as vector queues
 *
 * Created on: Apr 28, 2023
 *      Author: Nikita Smirnov
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <cmath>
#include <numeric>
#include <vector>

template<class T>
class Statistics {
public:
    using TT = decltype(T{} * T{}); // sum of squares

    inline static const int CACHE_LIMIT = 100000;

    Statistics(bool isSimple = false)
        : count_(T{0})
        , last_(T{0})
        , lastCached_(T{0})
        , max_(std::numeric_limits<T>::lowest())
        , min_(std::numeric_limits<T>::max())
        , sum_(T{0})
        , sum2_(TT{0})
        , isSimple_(isSimple) {}

    ~Statistics() = default;

    // add a new element to statistics
    void add(const T& value) {
        *this += value;
        periodicCache_.push_back(value);
        if (periodicCache_.size() > CACHE_LIMIT) {
            clearCache();
            lastCached_ = last_;
        }
    }

    // add a new vector of elements to statistics
    void add(std::vector<T>& vector) {
        for (auto it = vector.begin(); it != vector.end(); ++it) {
            *this += *it;
            periodicCache_.push_back(*it);
            if (periodicCache_.size() > CACHE_LIMIT) {
                clearCacheAndSaveLast();
            }
        }
    }

    // += operator for add methods
    Statistics<T>& operator+=(const T& value) {
        ++count_;
        last_ = value;

        if (!isSimple_) {
            if (last_ < min_)
                min_ = last_;
            else if (last_ > max_)
                max_ = last_;
            sum_ += last_;
            sum2_ += last_ * last_;
        }

        return *this;
    }

    // += operator for add methods for summing two statistics
    Statistics<T>& operator+=(const Statistics& other) {
        count_ += other.count_;
        last_ = other.last_;
        lastCached_ = T{0};
        max_ = std::max(max_, other.max_);
        min_ = std::min(min_, other.min_);
        sum_ += other.sum_;
        sum2_ += other.sum2_;
        clearCache();
        return *this;
    }

    inline void clearCache() {
        periodicCache_.clear();
    }

    inline void clearCacheAndSaveLast() {
        clearCache();
        lastCached_ = last_;
    }

    // getters
    inline size_t getCount() const {
        return count_;
    }

    inline T getLast() const {
        return last_;
    }

    inline T getLastCached() const {
        return lastCached_;
    }

    inline T getMax() const {
        return max_;
    }

    inline T getMin() const {
        return min_;
    }

    inline T getSum() const {
        return sum_;
    }

    inline TT getSum2() const {
        return sum2_;
    }

    inline T getMean() const {
        return sum_ / std::max<size_t>(count_, 1);
    }

    inline T getMeanCached(T defaultValue = T{0.0}) const {
        if (periodicCache_.empty()) {
            return defaultValue;
        } else {
            return std::accumulate(periodicCache_.begin(), periodicCache_.end(), T{0.0}) / periodicCache_.size();
        }
    }

    auto getVar() const {
        auto count = std::max<size_t>(count_, 1);
        return ((sum2_ / count) - (sum_ / count) * (sum_ / count));
    }

    auto getVarCached(T defaultValue = T {0.0}) const {
        if (periodicCache_.empty()) {
            return defaultValue;
        } else {
            T mean = getMeanCached();
            T squaredDiffSum = T{0.0};
            for (const T& value : periodicCache_) {
                squaredDiffSum += (value - mean) * (value - mean);
            }
            return squaredDiffSum / periodicCache_.size();
        }
    }

    inline T getStdev() const {
        return sqrt(getVar());
    }

private:
    size_t count_{};
    T last_;
    T lastCached_;
    T max_;
    T min_;
    T sum_;
    TT sum2_;
    // for periodic measurements
    std::vector<T> periodicCache_;
    // if statistics is simple, then do just incrementing
    bool isSimple_;
};

// a counter class for cathegorical vsalues i.e., ranks, indicators. Has a local cache for periodic sub-measurements
template<class T>
class ElementCounter {
public:
    inline static const int CACHE_LIMIT = 100000;

    ElementCounter()
        : mostCommon_(T{0})
        , mostCommonRatio_(0)
        , last_(T{0})
        , lastCached_(T{0})
        , numElems_(0)
        , numDiffElems_(0)
        , numChangesHappened_(0)
        , numPerDiffElems_(0)
        , numPerChangesHappened_(0) {}

    ~ElementCounter() {
        countMap_.clear();
        clearCache();
    }

    void clearCache() {
        periodicCache_.clear();
        numPerDiffElems_ = 0;
        numPerChangesHappened_ = 0;
    }

    void clearCacheAndSaveLast() {
        clearCache();
        lastCached_ = last_;
    }

    void add(const T& element) {
        numElems_++;

        if (countMap_.find(element) == countMap_.end()) {
            countMap_[element] = 1;
            numDiffElems_++;
            numPerDiffElems_++;
        } else {
            countMap_[element]++;
        }

        if (countMap_[element] > countMap_[mostCommon_]) {
            mostCommon_ = element;
            mostCommonRatio_ = static_cast<double>(countMap_[mostCommon_]) / numElems_;
        }

        if (element != last_) {
            numChangesHappened_++;
            numChangesHappened_++;
        }
        last_ = element;

        periodicCache_.push_back(element);
        if (periodicCache_.size() > CACHE_LIMIT) clearCache();
    }

    inline int getCount(const T& element) const {
        if (countMap_.find(element) == countMap_.end()) {
            return 0;
        } else {
            return countMap_[element];
        }
    }

    inline T getMostCommon() const {
        return mostCommon_;
    }

    inline double getMostCommonRatio() const {
        return mostCommonRatio_;
    }

    inline T getLast() const {
        return last_;
    }

    inline int getNumElements() const {
        return numElems_;
    }

    inline int getNumDiffElements() const {
        return numDiffElems_;
    }

    inline int getNumChangesHappened() const {
        return numChangesHappened_;
    }

    inline std::unordered_map<T, int>& getCountMap() const {
        return countMap_;
    }

    inline std::vector<T>& getPeriodicCache() const {
        return periodicCache_;
    }

    inline int getNumPerElements() const {
        return (int)periodicCache_.size();
    }

    inline int getNumPerDiffElements() const {
        return numPerDiffElems_;
    }

    inline int getNumPerChangesHappened() const {
        return numPerChangesHappened_;
    }

private:
    std::unordered_map<T, int> countMap_;
    T mostCommon_;
    double mostCommonRatio_;
    T last_;
    T lastCached_;
    int numElems_;
    int numDiffElems_;
    int numChangesHappened_;
    // for periodic measurements
    std::vector<T> periodicCache_;
    int numPerDiffElems_;
    int numPerChangesHappened_;

};

#endif
