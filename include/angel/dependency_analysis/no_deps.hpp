#include <map>
#include <fmt/format.h>
#include <iostream>

namespace angel
{

struct no_deps_analysis_params
{
};

struct no_deps_analysis_stats
{
  stopwatch<>::duration_type total_time{0};

  void report() const
  {
  }

  void reset()
  {
    *this = {};
  }
};

struct no_deps_analysis_result_type
{
  /* maps an index to an ESOP cover */
  std::map<uint32_t, std::vector<std::vector<uint32_t>>> dependencies;

  void print() const {}
};

class no_deps_analysis
{
public:
  using parameter_type = no_deps_analysis_params;
  using statistics_type = no_deps_analysis_stats;
  using result_type = no_deps_analysis_result_type;

public:
  using function_type = kitty::dynamic_truth_table;

public:
  explicit no_deps_analysis( no_deps_analysis_params const& ps, no_deps_analysis_stats& st )
    : ps( ps )
    , st( st )
  {
  }

  no_deps_analysis_result_type run( function_type const& function ) const
  {
    stopwatch t( st.total_time );
    return no_deps_analysis_result_type{};
  }

private:
  no_deps_analysis_params const& ps;
  no_deps_analysis_stats& st;
};

} /// namespace angel
