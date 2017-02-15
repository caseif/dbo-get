#pragma once

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
    kDeleted,
    kUnknown
};

static const std::vector<Stage> kDisallowedStages = {Stage::kInactive, Stage::kAbandoned, Stage::kDeleted};

Stage stageFromString(std::string str);

std::string stringFromStage(Stage stage);
