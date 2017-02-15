#include "stage.h"

#include <assert.h>
#include <map>
#include <string>
#include <vector>

#include "util.h"

static std::vector<std::string> stageStrings = {"planning", "alpha", "beta", "release", "mature", "inactive",
        "abandoned", "deleted", "unknown"};

static std::map<std::string, Stage> stageMap = std::map<std::string, Stage>();
static std::map<Stage, std::string> stageMap2 = std::map<Stage, std::string>(); // idgaf anymore

static void initMaps() {
    if (stageMap.empty()) {
        for (int i = Stage::kAlpha; i != Stage::kUnknown; i++) {
            stageMap[stageStrings[i - Stage::kAlpha]] = (Stage) i;
            stageMap2[(Stage) i] = stageStrings[i - Stage::kAlpha];
        }
    }
}

Stage stageFromString(std::string str) {
    initMaps();
    if (stageMap.count(str) > 0) {
        return stageMap[str];
    } else {
        return Stage::kUnknown;
    }
}

std::string stringFromStage(Stage stage) {
    initMaps();
    return stageMap2[stage];
}
