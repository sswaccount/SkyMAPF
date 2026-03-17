#pragma once

// Generated from CMake configuration.
// Numeric version comes from project(SkyMAPF VERSION x.y.z).
// Prerelease suffix is controlled by SKYMAPF_VERSION_SUFFIX.
#define SKYMAPF_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define SKYMAPF_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define SKYMAPF_VERSION_PATCH @PROJECT_VERSION_PATCH@
#define SKYMAPF_VERSION_SUFFIX "@SKYMAPF_VERSION_SUFFIX@"
#define SKYMAPF_VERSION_STRING "@SKYMAPF_FULL_VERSION@"
#define SKYMAPF_VERSION SKYMAPF_VERSION_STRING

namespace skymapf {

static constexpr int kVersionMajor = SKYMAPF_VERSION_MAJOR;
static constexpr int kVersionMinor = SKYMAPF_VERSION_MINOR;
static constexpr int kVersionPatch = SKYMAPF_VERSION_PATCH;
static constexpr const char kVersionSuffix[] = SKYMAPF_VERSION_SUFFIX;
static constexpr const char kVersionString[] = SKYMAPF_VERSION_STRING;

}  // namespace skymapf
