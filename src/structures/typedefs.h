#ifndef VROOM_TYPEDEFS_H
#define VROOM_TYPEDEFS_H

/*

This file is part of VROOM.

Copyright (c) 2015-2024, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <array>
#include <cassert>
#include <chrono>
#include <limits>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef _MSC_VER
// include support for "and"/"or"
#include <iso646.h>
#endif

namespace vroom {

// To easily differentiate variable types.
using Id = uint64_t;
using Index = uint16_t;
using UserCost = uint32_t;
using Cost = int64_t;
using UserDuration = uint32_t;
using UserDurationList = std::vector<UserDuration>;
using UserDurationMap = std::map<std::string, UserDuration>;
using Duration = int64_t;
using DurationList = std::vector<Duration>;
using DurationMap = std::map<std::string, Duration>;
using UserDistance = uint32_t;
using Distance = int64_t;
using Coordinate = double;
using Capacity = int64_t;
using Skill = uint32_t;
using Priority = uint32_t;
using MaxTasks = size_t;
using MaxTasksMap = std::map<std::string, MaxTasks>;

// Type helpers.
struct Coordinates {
  Coordinate lon;
  Coordinate lat;
};

using OptionalCoordinates = std::optional<Coordinates>;
using Skills = std::unordered_set<Skill>;
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Timeout = std::optional<std::chrono::milliseconds>;
using Deadline = std::optional<TimePoint>;

// Setting max value would cause trouble with further additions.
constexpr UserCost INFINITE_USER_COST =
  3 * (std::numeric_limits<UserCost>::max() / 4);

const std::string DEFAULT_PROFILE = "car";
const std::string DEFAULT_OSRM_SNAPPING_RADIUS = "35000";
constexpr double DEFAULT_LIBOSRM_SNAPPING_RADIUS = 35000;

// Our internal time measure is the hundredth of a second.
constexpr Duration DURATION_FACTOR = 100;

// Used to scale distances internally in a consistent way when used
// inside cost evaluations.
constexpr Distance DISTANCE_FACTOR = 360;

// Costs can be derived from travel times with a cost per hour for
// vehicles. So we scale all costs in order to not use floating point
// values while avoiding rounding issues internally.
constexpr Cost COST_FACTOR = 3600;
// This means a cost of one per second so that we default to
// outputting exact same values for duration and cost if per_hour
// values are not set.
constexpr UserCost DEFAULT_COST_PER_HOUR = 3600;
constexpr UserCost DEFAULT_COST_PER_KM = 0;

constexpr Priority MAX_PRIORITY = 100;
constexpr double MAX_SPEED_FACTOR = 5.0;
constexpr unsigned MAX_EXPLORATION_LEVEL = 5;

constexpr unsigned DEFAULT_EXPLORATION_LEVEL = 5;
constexpr unsigned DEFAULT_THREADS_NUMBER = 4;

constexpr auto DEFAULT_MAX_TASKS = std::numeric_limits<size_t>::max();
constexpr auto DEFAULT_MAX_TRAVEL_TIME = std::numeric_limits<Duration>::max();
constexpr auto DEFAULT_MAX_DISTANCE = std::numeric_limits<Distance>::max();

// Available routing engines.
enum class ROUTER { OSRM, LIBOSRM, ORS, VALHALLA };

// Used to describe a routing server.
struct Server {
  std::string host;
  std::string port;
  std::string path{""};

  Server() : host("0.0.0.0"), port("5000") {
  }

  Server(std::string host, std::string port)
    : host(std::move(host)), port(std::move(port)) {
  }
};

// 'Single' job is a regular one-stop job without precedence
// constraints.
enum class JOB_TYPE { SINGLE, PICKUP, DELIVERY };

// Available location status.
enum class STEP_TYPE { START, JOB, BREAK, END };

// Heuristic options.
enum class HEURISTIC { BASIC, DYNAMIC, INIT_ROUTES };
enum class INIT { NONE, HIGHER_AMOUNT, NEAREST, FURTHEST, EARLIEST_DEADLINE };
enum class SORT { AVAILABILITY, COST };

struct HeuristicParameters {
  HEURISTIC heuristic;
  INIT init;
  float regret_coeff;
  SORT sort;

  constexpr HeuristicParameters(HEURISTIC heuristic,
                                INIT init,
                                float regret_coeff,
                                SORT sort = SORT::AVAILABILITY)
    : heuristic(heuristic), init(init), regret_coeff(regret_coeff), sort(sort) {
  }

  // Only makes sense for user-defined initial routes.
  constexpr HeuristicParameters(HEURISTIC heuristic)
    : heuristic(heuristic),
      init(INIT::NONE),
      regret_coeff(0),
      sort(SORT::AVAILABILITY) {
    assert(heuristic == HEURISTIC::INIT_ROUTES);
  }
};

// Possible violations.
enum class VIOLATION {
  LEAD_TIME,
  DELAY,
  LOAD,
  MAX_TASKS,
  SKILLS,
  PRECEDENCE,
  MISSING_BREAK,
  MAX_TRAVEL_TIME,
  MAX_LOAD,
  MAX_DISTANCE
};

enum OperatorName {
  UnassignedExchange,
  CrossExchange,
  MixedExchange,
  TwoOpt,
  ReverseTwoOpt,
  Relocate,
  OrOpt,
  IntraExchange,
  IntraCrossExchange,
  IntraMixedExchange,
  IntraRelocate,
  IntraOrOpt,
  IntraTwoOpt,
  PDShift,
  RouteExchange,
  SwapStar,
  RouteSplit,
  PriorityReplace,
  TSPFix,
  MAX
};

// Defined based on
// https://sonarcloud.io/organizations/vroom-project/rules?open=cpp%3AS6045&rule_key=cpp%3AS6045
struct StringHash {
  using is_transparent = void; // enables heterogenous lookup

  std::size_t operator()(std::string_view sv) const {
    std::hash<std::string_view> hasher;
    return hasher(sv);
  }
};

namespace utils {
constexpr inline Duration scale_from_user_duration(UserDuration d) {
  return DURATION_FACTOR * static_cast<Duration>(d);
}

inline DurationMap scale_from_user_duration_map(const UserDurationMap& d) {
  DurationMap scaled;

  for (const auto& [key, duration] : d) {
    scaled[key] = DURATION_FACTOR * duration;
  }

  return scaled;
}

constexpr inline UserDuration scale_to_user_duration(Duration d) {
  assert(d <=
         scale_from_user_duration(std::numeric_limits<UserDuration>::max()));
  return static_cast<UserDuration>(d / DURATION_FACTOR);
}

constexpr inline Cost scale_from_user_cost(UserCost c) {
  return DURATION_FACTOR * COST_FACTOR * static_cast<Cost>(c);
}

constexpr inline UserCost scale_to_user_cost(Cost c) {
  assert(c <= scale_from_user_cost(std::numeric_limits<UserCost>::max()));
  return static_cast<UserCost>(c / (DURATION_FACTOR * COST_FACTOR));
}
} // namespace utils

#ifdef LOG_LS_OPERATORS
namespace ls {
struct OperatorStats {
  unsigned tried_moves;
  unsigned applied_moves;

  OperatorStats() : tried_moves(0), applied_moves(0) {
  }

  OperatorStats(const unsigned tried_moves, const unsigned applied_moves)
    : tried_moves(tried_moves), applied_moves(applied_moves) {
  }
};
} // namespace ls
#endif

} // namespace vroom

#endif
