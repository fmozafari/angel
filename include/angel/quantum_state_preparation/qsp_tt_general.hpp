#pragma once
#include <kitty/dynamic_truth_table.hpp>
#include <angel/utils/helper_functions.hpp>
#include <angel/utils/stopwatch.hpp>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <vector>

namespace angel
{
using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;
using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
using order_t = std::vector<uint32_t>;

struct qsp_general_stats
{
    stopwatch<>::duration_type total_time{0};
    uint32_t total_bench{0};
    uint32_t has_no_dependencies{0};
    uint32_t no_dependencies_computed{0};
    uint32_t has_dependencies{0};
    uint32_t funcdep_bench_useful{0};
    uint32_t funcdep_bench_notuseful{0};
    uint32_t total_cnots{0};
    uint32_t total_rys{0};
    uint32_t total_nots{0};
    std::vector<std::pair<uint32_t, uint32_t>> gates_count = {};

    void report(std::ostream &os = std::cout) const
    {
        os << "[i] number of analyzed benchmarks = " << total_bench << std::endl;
        os << fmt::format("[i] total = no deps exist + no deps found + found deps ::: {} = {} + {} + {}\n",
                          (has_no_dependencies + no_dependencies_computed +
                           has_dependencies),
                          has_no_dependencies, no_dependencies_computed,
                          has_dependencies);
        os << fmt::format("[i] total deps = dep useful + dep not useful ::: {} = {} + {}\n",
                          funcdep_bench_useful + funcdep_bench_notuseful, funcdep_bench_useful, funcdep_bench_notuseful);
        os << fmt::format("[i] total synthesis time (considering dependencies) = {:8.2f}s\n",
                          to_seconds( total_time ));

        os << fmt::format("[i] synthesis result: CNOTs / RYs / NOTs = {} / {} / {} \n",
                          total_cnots, total_rys, total_nots);
        os << fmt::format("[i] synthesis result: CNOTs / SQgates = {} / {} \n",
                          total_cnots, total_rys + total_nots);
        /// Compute Avg and SD for CNOTs and SQgates
        auto sum_f = 0; /// CNOTs: first
        auto sum_s = 0; /// SQgates: second
        for (auto n = 0u; n < gates_count.size(); n++)
        {
            sum_f += gates_count[n].first;
            sum_s += gates_count[n].second;
        }

        double avg_f = sum_f / double(gates_count.size());
        double avg_s = sum_s / double(gates_count.size());

        auto var_f = 0.0;
        auto var_s = 0.0;
        for (auto n = 0u; n < gates_count.size(); n++)
        {
            var_f += (double(gates_count[n].first) - avg_f) * (double(gates_count[n].first) - avg_f);
            var_s += (double(gates_count[n].second) - avg_s) * (double(gates_count[n].second) - avg_s);
        }
        var_f /= gates_count.size();
        var_s /= gates_count.size();
        auto sd_f = sqrt(var_f);
        auto sd_s = sqrt(var_s);
        os << fmt::format("[i] Avg Cnots = {}\n", avg_f);
        os << fmt::format("[i] Var Cnots = {}\n", var_f);
        os << fmt::format("[i] SD Cnots = {}\n", sd_f);

        os << fmt::format("[i] Avg SQgates = {}\n", avg_s);
        os << fmt::format("[i] Var SQgates = {}\n", var_s);
        os << fmt::format("[i] SD SQgates = {}\n", sd_s);
    }
};

struct deps_operation_stats
{
    uint32_t eq_op{0};
    uint32_t not_op{0};
    std::vector<uint32_t> xor_op{std::vector<uint32_t>(6,0)};
    std::vector<uint32_t> xnor_op{std::vector<uint32_t>(6,0)};
    std::vector<uint32_t> and_op{std::vector<uint32_t>(6,0)};
    std::vector<uint32_t> nand_op{std::vector<uint32_t>(6,0)};
    std::vector<uint32_t> or_op{std::vector<uint32_t>(6,0)};
    std::vector<uint32_t> nor_op{std::vector<uint32_t>(6,0)};

    void report(std::ostream &os = std::cout) const
    {
        os << "[i] number of eq operation: " << eq_op << std::endl;
        os << "[i] number of not operation: " << not_op << std::endl;
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of xor({}) operation: {}\n", i, xor_op[i] );
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of xnor({}) operation: {}\n", i, xnor_op[i] );
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of and({}) operation: {}\n", i, and_op[i] );
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of nand({}) operation: {}\n", i, nand_op[i] );
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of or({}) operation: {}\n", i, or_op[i] );
        for(auto i=2u; i<6; i++)
            os << fmt::format( "[i] number of nor({}) operation: {}\n", i, nor_op[i] );
        
    }
};

void extract_deps_operation_stats(deps_operation_stats & op_stats, dependencies_t deps)
{
    for(auto i=0u; i<deps.size(); i++)
    {
        if(deps.find(i) == deps.end())
            continue;

        if(deps[i][0].first=="eq")
            op_stats.eq_op++;
        else if(deps[i][0].first=="not")
            op_stats.not_op++;
        else if(deps[i][0].first=="xor")
            op_stats.xor_op[deps[i][0].second.size()]++;
        else if(deps[i][0].first=="xnor")
            op_stats.xnor_op[deps[i][0].second.size()]++;
        else if(deps[i][0].first=="and")
            op_stats.and_op[deps[i][0].second.size()]++;
        else if(deps[i][0].first=="nand")
            op_stats.nand_op[deps[i][0].second.size()]++;
        else if(deps[i][0].first=="or")
            op_stats.or_op[deps[i][0].second.size()]++;
        else if(deps[i][0].first=="nor")
            op_stats.nor_op[deps[i][0].second.size()]++;
    }

}


/* with dependencies */
void MC_qg_generation(gates_t &gates, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls,
                      dependencies_t dependencies, std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines)
{
    /*-----co factors-------*/
    kitty::dynamic_truth_table tt0(var_index);
    kitty::dynamic_truth_table tt1(var_index);

    tt0 = kitty::shrink_to(kitty::cofactor0(tt, var_index), var_index);
    tt1 = kitty::shrink_to(kitty::cofactor1(tt, var_index), var_index);

    /*--computing probability gate---*/
    auto c0_ones = kitty::count_ones(tt0);
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    bool is_const = 0;
    auto it0 = std::find(zero_lines.begin(), zero_lines.end(), var_index);
    auto it1 = std::find(one_lines.begin(), one_lines.end(), var_index);
    if (it1 != one_lines.end() ) // insert not gate
    {
        if( gates.find(var_index)==gates.end() )
        {
            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
        }
        is_const = 1;
    }
    else if (it0 != zero_lines.end()) // inzert zero gate
    {
        is_const = 1;
    }
    else if (c0_ones != tt_ones)
    { /* == --> identity and ignore */
        double angle = 2 * acos(sqrt(static_cast<double>(c0_ones) / tt_ones));
        //angle *= (180/3.14159265); //in degree
        /*----add probability gate----*/
        auto it = dependencies.find(var_index);

        if (it != dependencies.end())
        {
            if (gates[var_index].size() == 0)
            {
                for (auto d = 0; d < dependencies[var_index].size(); d++)
                {
                    if (dependencies[var_index][d].first == "eq")
                    {
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0]}});
                        break;
                    }

                    else if (dependencies[var_index][d].first == "not")
                    {
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "xor")
                    {
                        for (auto d_in = 0u; d_in < dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[d_in]}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "xnor")
                    {
                        for (auto d_in = 0u; d_in < dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[d_in]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "and")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, dependencies[var_index][d].second});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "nand")
                    {
                        gates[var_index].emplace_back(std::pair{M_PI, dependencies[var_index][d].second});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "or")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, dependencies[var_index][d].second});
                        for (auto d_in = 0u; d_in < dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[d_in]}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "nor")
                    {
                        gates[var_index].emplace_back(std::pair{M_PI, dependencies[var_index][d].second});
                        for (auto d_in = 0u; d_in < dependencies[var_index][d].second.size(); d_in++)
                            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[d_in]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "and_xor")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0], dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "and_xnor")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0], dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "or_xor")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0], dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]}});
                        break;
                    }
                    else if (dependencies[var_index][d].first == "or_xnor")
                    {
                        // to do --- insert nots
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0], dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[0]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[1]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index][d].second[2]}});
                        gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
                        break;
                    }
                }
            }
        }
        else
            gates[var_index].emplace_back(std::pair{angle, controls});
    }

    /*-----qc of cofactors-------*/
    /*---check state---*/
    auto c0_allone = (c0_ones == pow(2, tt0.num_vars())) ? true : false;
    auto c0_allzero = (c0_ones == 0) ? true : false;
    auto c1_allone = (c1_ones == pow(2, tt1.num_vars())) ? true : false;
    auto c1_allzero = (c1_ones == 0) ? true : false;

    std::vector<uint32_t> controls_new0;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new0));
    if (dependencies.find(var_index) == dependencies.end() && !is_const)
    {
        auto ctrl0 = var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
        controls_new0.emplace_back(ctrl0);
    }
    std::vector<uint32_t> controls_new1;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new1));
    if (dependencies.find(var_index) == dependencies.end() && !is_const)
    {
        auto ctrl1 = var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
        controls_new1.emplace_back(ctrl1);
    }

    if (c0_allone)
    {
        /*---add H gates---*/
        for (auto i = 0u; i < var_index; i++)
            gates[i].emplace_back(std::pair{M_PI / 2, controls_new0});
        /*--check one cofactor----*/
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines);
        }
    }
    else if (c0_allzero)
    {
        /*--check one cofactor----*/
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines);
        }
    }
    else
    { /* some 0 some 1 for c0 */
        if (c1_allone)
        {
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines);
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines);
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines);
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines);
        }
    }
}

/* without dependencies */
void MC_qg_generation(gates_t &gates, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls,
                      std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines)
{
    /*-----co factors-------*/
    kitty::dynamic_truth_table tt0(var_index);
    kitty::dynamic_truth_table tt1(var_index);

    tt0 = kitty::shrink_to(kitty::cofactor0(tt, var_index), var_index);
    tt1 = kitty::shrink_to(kitty::cofactor1(tt, var_index), var_index);

    /*--computing probability gate---*/
    auto c0_ones = kitty::count_ones(tt0);
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    bool is_const = 0;
    auto it0 = std::find(zero_lines.begin(), zero_lines.end(), var_index);
    auto it1 = std::find(one_lines.begin(), one_lines.end(), var_index);
    if ( it1 != one_lines.end() ) // insert not gate
    {
        if( gates.find(var_index)==gates.end() )
            gates[var_index].emplace_back(std::pair{M_PI, std::vector<uint32_t>{}});
        is_const = 1;
    }
    else if (it0 != zero_lines.end()) // inzert zero gate
    {
        is_const = 1;
    }
    else if (c0_ones != tt_ones)
    { /* == --> identity and ignore */
        double angle = 2 * acos(sqrt(static_cast<double>(c0_ones) / tt_ones));
        //angle *= (180/3.14159265); //in degree
        /*----add probability gate----*/

        gates[var_index].emplace_back(std::pair{angle, controls});
    }

    /*-----qc of cofactors-------*/
    /*---check state---*/
    auto c0_allone = (c0_ones == pow(2, tt0.num_vars())) ? true : false;
    auto c0_allzero = (c0_ones == 0) ? true : false;
    auto c1_allone = (c1_ones == pow(2, tt1.num_vars())) ? true : false;
    auto c1_allzero = (c1_ones == 0) ? true : false;

    std::vector<uint32_t> controls_new0;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new0));
    if (!is_const)
    {
        auto ctrl0 = var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
        controls_new0.emplace_back(ctrl0);
    }
    std::vector<uint32_t> controls_new1;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new1));
    if (!is_const)
    {
        auto ctrl1 = var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
        controls_new1.emplace_back(ctrl1);
    }

    if (c0_allone)
    {
        /*---add H gates---*/
        for (auto i = 0u; i < var_index; i++)
            gates[i].emplace_back(std::pair{M_PI / 2, controls_new0});
        /*--check one cofactor----*/
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines);
        }
    }
    else if (c0_allzero)
    {
        /*--check one cofactor----*/
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines);
        }
    }
    else
    { /* some 0 some 1 for c0 */
        if (c1_allone)
        {
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines);
            /*---add H gates---*/
            for (auto i = 0u; i < var_index; i++)
                gates[i].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines);
        }
        else
        { /* some 1 some 0 */
            MC_qg_generation(gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines);
            MC_qg_generation(gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines);
        }
    }
}

/* with dependencies */
void gates_statistics(gates_t gates, dependencies_t const &dependencies,
                      uint32_t const num_vars, qsp_general_stats &stats)
{
    auto total_rys = 0;
    auto total_cnots = 0;
    auto total_nots = 0;
    bool have_max_controls;
    auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

    for (int32_t i = num_vars - 1; i >= 0; i--)
    {
        auto rys = 0;
        auto cnots = 0;
        auto nots = 0;
        have_max_controls = 0;

        if (gates.find(i) == gates.end())
        {
            n_reduc++;
            continue;
        }
        if (gates[i].size() == 0)
        {
            n_reduc++;
            continue;
        }
        if (gates[i].size() == 1 && gates[i][0].first == M_PI && gates[i][0].second.size() == 0)
        {
            total_nots++;
            n_reduc++;
            continue;
        }

        /* check deps */
        auto it = dependencies.find(i);
        /* there exists deps */
        if (it != dependencies.end())
        {
            for (auto j = 0u; j < gates[i].size(); j++)
            {
                if (gates[i][j].second.size() == ((num_vars - i - 1) - n_reduc) &&
                    gates[i][j].second.size() != 0) /* number of controls is max or not? */
                {
                    have_max_controls = 1;
                    break;
                }

                auto cs = gates[i][j].second.size();
                if (cs == 0 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                    nots++;
                else if (cs == 0)
                    rys++;
                else if (cs == 1 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                    cnots++;
                else
                {
                    rys += pow(2, cs);
                    cnots += pow(2, cs);
                }
            }
            if (have_max_controls || cnots > pow(2, ((num_vars - i - 1) - n_reduc))) /* we have max number of controls */
            {
                if (i == (num_vars - 1 - n_reduc)) /* first line for preparation */
                {
                    cnots = 0;
                    rys = 1;
                }
                else if (gates[i].size() == 1 && (std::abs(gates[i][0].first - M_PI) < 0.1) && gates[i][0].second.size() == 1) // second line for preparation
                {
                    cnots = 1;
                    rys = 0;
                }
                else /* other lines with more than one control */
                {
                    rys = pow(2, ((num_vars - i - 1) - n_reduc));
                    cnots = pow(2, ((num_vars - i - 1) - n_reduc));
                }
            }         
        }
        /* doesn't exist deps */
        else
        {
            for (auto j = 0u; j < gates[i].size(); j++)
            {
                if (gates[i][j].second.size() == ((num_vars - i - 1) - n_reduc) &&
                    gates[i][j].second.size() != 0) /* number of controls is max or not? */
                {
                    have_max_controls = 1;
                    break;
                }

                auto cs = gates[i][j].second.size();
                if (cs == 0 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                    nots++;
                else if (cs == 0)
                    rys++;
                else if (cs == 1 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                    cnots += 1;
                else
                {
                    rys += pow(2, cs);
                    cnots += pow(2, cs);
                }
            }
            if (have_max_controls || cnots > pow(2, ((num_vars - i - 1) - n_reduc))) /* we have max number of controls */
            {
                if (i == (num_vars - 1 - n_reduc)) /* first line for preparation */
                {
                    cnots = 0;
                    rys = 1;
                }
                else if (gates[i].size() == 1 && (std::abs(gates[i][0].first - M_PI) < 0.1) && gates[i][0].second.size() == 1) // second line for preparation
                {
                    cnots = 1;
                    rys = 0;
                }
                else /* other lines with more than one control */
                {
                    rys = pow(2, ((num_vars - i - 1) - n_reduc));
                    cnots = pow(2, ((num_vars - i - 1) - n_reduc));
                }
            }
        }

        total_rys += rys;
        total_cnots += cnots;
        total_nots += nots;
    }

    stats.total_cnots += total_cnots;
    stats.total_rys += total_rys;
    stats.total_nots += total_nots;
    stats.gates_count.emplace_back(std::make_pair(total_cnots, total_rys + total_nots));

    if (dependencies.size() > 0)
    {
        if (total_cnots < (pow(2, num_vars - n_reduc) - 2))
        {
            ++stats.funcdep_bench_useful;
        }
        else
        {
            ++stats.funcdep_bench_notuseful;
        }
    }

    return;
}

/* without dependencies */
void gates_statistics(gates_t gates, uint32_t const num_vars, qsp_general_stats &stats)
{
    auto total_rys = 0;
    auto total_cnots = 0;
    auto total_nots = 0;
    bool have_max_controls;
    auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

    for (int32_t i = num_vars - 1; i >= 0; i--)
    {
        auto rys = 0;
        auto cnots = 0;
        auto nots = 0;
        have_max_controls = 0;

        if (gates.find(i) == gates.end())
        {
            n_reduc++;
            continue;
        }
        if (gates[i].size() == 0)
        {
            n_reduc++;
            continue;
        }
        if (gates[i].size() == 1 && gates[i][0].first == M_PI && gates[i][0].second.size() == 0)
        {
            nots++;
            n_reduc++;
            continue;
        }

        for (auto j = 0u; j < gates[i].size(); j++)
        {
            if (gates[i][j].second.size() == ((num_vars - i - 1) - n_reduc) &&
                gates[i][j].second.size() != 0) /* number of controls is max or not? */
            {
                have_max_controls = 1;
                break;
            }

            auto cs = gates[i][j].second.size();
            if (cs == 0 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                nots++;
            else if (cs == 0)
                rys++;
            else if (cs == 1 && (std::abs(gates[i][j].first - M_PI) < 0.1))
                cnots += 1;
            else
            {
                rys += pow(2, cs);
                cnots += pow(2, cs);
            }
        }
        if (have_max_controls || cnots > pow(2, ((num_vars - i - 1) - n_reduc))) /* we have max number of controls */
        {
            if (i == (num_vars - 1 - n_reduc)) /* first line for preparation */
            {
                cnots = 0;
                rys = 1;
            }
            else if (gates[i].size() == 1 && (std::abs(gates[i][0].first - M_PI) < 0.1) && gates[i][0].second.size() == 1) // second line for preparation
            {
                cnots = 1;
                rys = 0;
            }
            else /* other lines with more than one control */
            {
                rys = pow(2, ((num_vars - i - 1) - n_reduc));
                cnots = pow(2, ((num_vars - i - 1) - n_reduc));
            }
        }

        total_rys += rys;
        total_cnots += cnots;
        total_nots += nots;
    }

    stats.total_cnots += total_cnots;
    stats.total_rys += total_rys;
    stats.total_nots += total_nots;
    stats.gates_count.emplace_back(std::make_pair(total_cnots, total_rys + total_nots));

    return;
}

/**
 * \breif General quantum state preparation algorithm for any function represantstion, 
 * dependency analysis algorithm, and reordering algorithm.
 * 
 * \tparam Network the type of generated quantum circuit
 * \tparam DependencyAnalysisAlgorithm specifies the method of extracting dependencies. By default, we don't have any dependencies.
 * \tparam ReorderingAlgorithm the way of extracting variable orders
 * \param net the extracted quantum circuit for given quantum state
 * \param tt given Boolean function corresponding to the quantum state
 * \param qsp_stats store all desired statistics of quantum state preparation process
*/
template <class Network, class DependencyAnalysisAlgorithm, class ReorderingAlgorithm>
void qsp_tt_general(Network &net, DependencyAnalysisAlgorithm deps_alg, ReorderingAlgorithm orders_alg,
                    kitty::dynamic_truth_table tt, qsp_general_stats& final_qsp_stats, deps_operation_stats& op_stats)
{
    if(kitty::is_const0(tt))
    {
        std::cout<<"The tt is zero, please enter another tt\n";
        return;
    }
    const uint32_t qubits_count = tt.num_vars();
    for (auto i = 0u; i < qubits_count; i++)
        net.add_qubit();

    auto orders = orders_alg.run(qubits_count);
    // std::cout<<"num orders: "<<orders.size()<<std::endl;
    // std::cout<<"qubit size: "<<qubits_count<<std::endl;
    // std::cout<<"order size: "<<orders.size()<<std::endl;
    // for(auto const& o: orders)
    // {
    //     for(auto i=0u; i<o.size(); i++)
    //         std::cout<<o[i]<<"  ";
    //     std::cout<<std::endl;
    // }

    auto max_cnots = pow(2, qubits_count + 1);
    order_t best_order(qubits_count);
    dependencies_t best_deps;
    qsp_general_stats best_stats;
    stopwatch<>::duration_type time_traversal{0};
    {
        stopwatch t(time_traversal);
        for (auto order : orders)
        {
            order_t next_order; 
            
            kitty::dynamic_truth_table tt_temp = tt;
            angel::reordering_on_tt_inplace(tt_temp, order);
            qsp_general_stats qsp_stats_temp;
            dependencies_t deps_temp = deps_alg.run(tt_temp, qsp_stats_temp);
            if(orders_alg.compute_next_order(next_order, qubits_count, deps_temp))
            {
                //std::cout<<"before: "<<order[0]<<"  "<<order[1]<<"  "<<order[2]<<"  "<<order[3]<<std::endl;
                std::copy(next_order.begin(), next_order.end(), order.begin());
                //std::cout<<"next: "<<order[0]<<"  "<<order[1]<<"  "<<order[2]<<"  "<<order[3]<<std::endl;
            }

            kitty::dynamic_truth_table tt_copy = tt;
            angel::reordering_on_tt_inplace(tt_copy, order);
            qsp_general_stats qsp_stats;
            dependencies_t deps = deps_alg.run(tt_copy, qsp_stats);
            
            std::vector<uint32_t> zero_lines;
            std::vector<uint32_t> one_lines;
            extract_independent_vars(zero_lines, one_lines, tt_copy);

            gates_t gates;
            auto var_idx = qubits_count - 1;
            std::vector<uint32_t> cs;

            if (deps_alg.get_considering_deps())
            {
                MC_qg_generation(gates, tt_copy, var_idx, cs, deps, zero_lines, one_lines);
                gates_statistics(gates, deps, qubits_count, qsp_stats);
            }
            else
            {
                MC_qg_generation(gates, tt_copy, var_idx, cs, zero_lines, one_lines);
                gates_statistics(gates, qubits_count, qsp_stats);
            }

            if (max_cnots > qsp_stats.total_cnots)
            {
                std::copy(order.begin(), order.end(), best_order.begin());
                best_stats = qsp_stats;
                max_cnots = qsp_stats.total_cnots;
                if(best_deps.size()>0)
                    best_deps.erase(best_deps.begin(), best_deps.end());
                best_deps = deps;
            }
        }
    }

    extract_deps_operation_stats(op_stats, best_deps);

    final_qsp_stats.total_time += time_traversal;
    final_qsp_stats.total_bench += best_stats.total_bench;
    final_qsp_stats.has_no_dependencies += best_stats.has_no_dependencies;
    final_qsp_stats.no_dependencies_computed += best_stats.no_dependencies_computed;
    final_qsp_stats.has_dependencies += best_stats.has_dependencies;
    final_qsp_stats.funcdep_bench_useful += best_stats.funcdep_bench_useful;
    final_qsp_stats.funcdep_bench_notuseful += best_stats.funcdep_bench_notuseful;
    final_qsp_stats.total_cnots += best_stats.total_cnots;
    final_qsp_stats.total_rys += best_stats.total_rys;
    final_qsp_stats.total_nots += best_stats.total_nots;
    
    if ( best_stats.gates_count.size() > 0u )
      final_qsp_stats.gates_count.emplace_back(best_stats.gates_count.back());

}

} // namespace angel
