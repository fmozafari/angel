#include <map>
#include <fmt/format.h>
#include <iostream>
#include <angel/quantum_state_preparation/qsp_tt_general.hpp>

namespace angel
{

class NoDeps
{
public:
    using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;

public:
    void print_dependencies(dependencies_t const &dependencies, std::ostream &os = std::cout)
    {
        os << "[i] dependencies:" << std::endl;
        os << "dependencies size: " << dependencies.size() << std::endl;
        for (const auto &d : dependencies)
        {
            os << d.first << "  ";
            for (const auto &dd : d.second)
            {
                os << dd.first << ' ';
                for (const auto &c : dd.second)
                {
                    os << c << ' ';
                }
            }
            os << std::endl;
        }
    }

public:
    explicit NoDeps()
    {
    }

    dependencies_t run(kitty::dynamic_truth_table const &tt, qsp_general_stats &stats)
    {
        (void) tt;
        stats.total_bench++;
        dependencies_t deps;
        return deps;
    }

    bool get_considering_deps()
    {
        return considering_deps;
    }

protected:
    bool considering_deps = false;
}; /// class NoDeps end

} /// namespace angel