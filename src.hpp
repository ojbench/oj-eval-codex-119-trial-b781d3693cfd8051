#ifndef LINEARSCAN_HPP
#define LINEARSCAN_HPP

// don't include other headfiles
#include <string>
#include <vector>
#include <set>

class Location {
public:
    // return a string that represents the location
    virtual std::string show() const = 0;
    virtual int getId() const = 0;
};

class Register : public Location {
private:
    int regId;
public:
    Register(int regId) : regId(regId) {
    }
    virtual std::string show() const {
        return std::string("reg") + std::to_string(regId);
    }
    virtual int getId() const {
        return regId;
    }
};

class StackSlot : public Location {
public:
    StackSlot() {}
    virtual std::string show() const {
        return "stack";
    }
    virtual int getId() const {
        return -1;
    }
};

struct LiveInterval {
    int startpoint;
    int endpoint;
    Location* location = nullptr;
};

class LinearScanRegisterAllocator {
private:
    struct IntervalEndCmp {
        bool operator()(const LiveInterval* lhs, const LiveInterval* rhs) const {
            return lhs->endpoint < rhs->endpoint;
        }
    };

    int regNum;
    std::vector<Register*> freeRegisters;
    std::set<LiveInterval*, IntervalEndCmp> active;

    void expireOldIntervals(LiveInterval& i) {
        while (!active.empty()) {
            auto it = active.begin();
            LiveInterval* interval = *it;
            if (interval->endpoint >= i.startpoint) {
                break;
            }
            freeRegisters.push_back(static_cast<Register*>(interval->location));
            active.erase(it);
        }
    }
    void spillAtInterval(LiveInterval& i) {
        if (active.empty()) {
            i.location = freeRegisters.back();
            freeRegisters.pop_back();
            active.insert(&i);
            return;
        }

        auto it = active.end();
        --it;
        LiveInterval* spilled = *it;
        if (spilled->endpoint > i.endpoint) {
            i.location = spilled->location;
            spilled->location = new StackSlot();
            active.erase(it);
            active.insert(&i);
        } else {
            i.location = new StackSlot();
        }
    }
public:
    LinearScanRegisterAllocator(int regNum) : regNum(regNum) {
        for (int regId = regNum - 1; regId >= 0; --regId) {
            freeRegisters.push_back(new Register(regId));
        }
    }
    void linearScanRegisterAllocate(std::vector<LiveInterval>& intervalList) {
        for (int index = 0; index < static_cast<int>(intervalList.size()); ++index) {
            LiveInterval& interval = intervalList[index];
            expireOldIntervals(interval);
            if (!freeRegisters.empty()) {
                interval.location = freeRegisters.back();
                freeRegisters.pop_back();
                active.insert(&interval);
            } else {
                spillAtInterval(interval);
            }
        }
    }
};

#endif
