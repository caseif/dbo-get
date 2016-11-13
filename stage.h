#pragma once

#include <assert.h>
#include <string>
#include <vector>

enum Stage {
    kPlanning,
    kAlpha,
    kBeta,
    kRelease,
    kMature,
    kInactive,
    kAbandoned,
    kDeleted
};

static std::map<std::string, Stage> stageMap = {{"planning", kPlanning}, {"alpha", kAlpha}, {"beta", kBeta},
        {"release", kRelease}, {"mature", kMature}, {"inactive", kInactive}, {"abandoned", kAbandoned},
        {"deleted", kDeleted}};

Stage stageFromString(std::string str) {
    assert(stageMap.count(str) > 0);
    return stageMap[str];
}
