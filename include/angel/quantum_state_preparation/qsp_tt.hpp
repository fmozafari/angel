/* Author: Fereshte */
#pragma once\
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/algorithms/synthesis/linear_synth.hpp>
#include <array>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <map>
#include <vector>
#include <kitty/operations.hpp>
#include <kitty/kitty.hpp>
#include <math.h>
#include <tweedledum/utils/stopwatch.hpp>
#include <typeinfo>

struct qsp_tt_statistics
{
    double time{0};
    uint32_t total_cnots{0};
    uint32_t total_rys{0};
    std::pair<uint32_t, uint32_t> gates_num = std::make_pair(0, 0);
}; /* qsp_tt_statistics */

namespace angel
{

namespace detail
{

inline void print_gates (std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>> gates)
{
    std::cout<<"++++++++++++ gates in QC +++++++++++++\n";
    for(int32_t i=gates.size()-1; i>=0; i--)
    {
        std::cout<<"line: "<<i<<std::endl;
        for(const auto & g: gates[i])
        {
            std::cout<<"angle: "<<g.first<<" controls: ";
            for(const auto & c:g.second)
            {
                std::cout<<c<<" ";
            }
            std::cout<<"\n";
        }
        std::cout<<"\n ----------------------\n";
    }
}

inline std::vector<uint32_t> initialize_orders(uint32_t n)
{
    std::vector<uint32_t> orders_init;
    for (auto i = 0u; i < n; i++)
        orders_init.emplace_back(i);
    return orders_init;
}

void gates_count_analysis(std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>> gates,
                          std::vector<uint32_t> const &orders,
                          uint32_t num_vars, qsp_tt_statistics &stats)
{
    auto total_rys = 0;
    auto total_cnots = 0;

    bool sig;
    auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

    for (int32_t i = num_vars - 1; i >= 0; i--)
    {
        if (gates.find(orders[i]) == gates.end())
        {
            n_reduc++;
            continue;
        }

        if (gates[orders[i]].size() == 0)
        {
            n_reduc++;
            continue;
        }

        auto rys = 0;
        auto cnots = 0;
        sig = 0;

        for (auto j = 0u; j < gates[orders[i]].size(); j++)
        {
            if (gates[orders[i]][j].second.size() == ((num_vars - i - 1) - n_reduc) &&
                gates[orders[i]][j].second.size() != 0) /* number of controls is max or not? */
            {
                sig = 1;
                break;
            }

            auto cs = gates[orders[i]][j].second.size();
            if (cs == 0)
                rys += 1;
            else if (cs == 1 && (std::abs(gates[orders[i]][j].first - M_PI) < 0.1))
                cnots += 1;
            else
            {
                rys += pow(2, cs);
                cnots += pow(2, cs);
            }
        }
        if (sig || cnots > pow(2, ((num_vars - i - 1) - n_reduc))) /* we have max number of controls */
        {
            if (i == (num_vars - 1 - n_reduc)) /* first line for preparation */
            {
                cnots = 0;
                rys = 1;
            }
            else if (gates[orders[i]].size() == 1 && (std::abs(gates[orders[i]][0].first - M_PI) < 0.1) && gates[orders[i]][0].second.size() == 1) // second line for preparation
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
    }

    stats.total_cnots += total_cnots;
    stats.total_rys += total_rys;
    stats.gates_num = std::make_pair(total_cnots, total_rys);
}

inline void general_qg_generation(
    std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>
        &gates,
    kitty::dynamic_truth_table tt, uint32_t var_index_pure,
    std::vector<uint32_t> controls, std::vector<uint32_t> const &orders)
{
    auto var_index = orders[var_index_pure];
    if (var_index == -1)
        return;
    /*-----co factors-------*/
    kitty::dynamic_truth_table tt0(var_index);
    kitty::dynamic_truth_table tt1(var_index);
    tt0 = kitty::shrink_to(kitty::cofactor0(tt, var_index), tt.num_vars() - 1);
    tt1 = kitty::shrink_to(kitty::cofactor1(tt, var_index), tt.num_vars() - 1);
    /*--computing probability gate---*/
    auto c0_ones = kitty::count_ones(tt0);
    auto c1_ones = kitty::count_ones(tt1);
    auto tt_ones = kitty::count_ones(tt);
    if (c0_ones != tt_ones)
    { /* == --> identity and ignore */
        double angle = 2 * acos(sqrt(static_cast<double>(c0_ones) / tt_ones));
        gates[var_index].emplace_back(std::pair{angle, controls});
    }
    /*-----qc of cofactors-------*/
    /*---check state--- */
    auto c0_allone = (c0_ones == pow(2, tt0.num_vars())) ? true : false;
    auto c0_allzero = (c0_ones == 0) ? true : false;
    auto c1_allone = (c1_ones == pow(2, tt1.num_vars())) ? true : false;
    auto c1_allzero = (c1_ones == 0) ? true : false;

    std::vector<uint32_t> controls_new0;
    std::copy(controls.begin(), controls.end(), back_inserter(controls_new0));
    auto ctrl0 =
        var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
    controls_new0.emplace_back(ctrl0);
    if (c0_allone)
    {
        /*---add H gates---*/
        for (auto i = 0u; i < var_index_pure; i++)
            gates[orders[i]].emplace_back(std::pair{M_PI / 2, controls_new0});
        /*--check one cofactor----*/
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1));
        auto ctrl1 =
            var_index * 2 + 0; /*positive control: /2 ---> index %2 ---> sign */
        controls_new1.emplace_back(ctrl1);
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index_pure; i++)
                gates[orders[i]].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            general_qg_generation(gates, tt1, var_index_pure - 1, controls_new1,
                                  orders);
        }
    }
    else if (c0_allzero)
    {
        /*--check one cofactor----*/
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1));
        auto ctrl1 =
            var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
        controls_new1.emplace_back(ctrl1);
        if (c1_allone)
        {
            /*---add H gates---*/
            for (auto i = 0u; i < var_index_pure; i++)
                gates[orders[i]].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            return;
        }
        else
        { /* some 1 some 0 */
            general_qg_generation(gates, tt1, var_index_pure - 1, controls_new1,
                                  orders);
        }
    }
    else
    { /* some 0 some 1 for c0  */
        std::vector<uint32_t> controls_new1;
        std::copy(controls.begin(), controls.end(), back_inserter(controls_new1));
        auto ctrl1 =
            var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
        controls_new1.emplace_back(ctrl1);
        if (c1_allone)
        {
            general_qg_generation(gates, tt0, var_index_pure - 1, controls_new0,
                                  orders);
            /*---add H gates---*/
            for (auto i = 0u; i < var_index_pure; i++)
                gates[orders[i]].emplace_back(std::pair{M_PI / 2, controls_new1});
        }
        else if (c1_allzero)
        {
            general_qg_generation(gates, tt0, var_index_pure - 1, controls_new0,
                                  orders);
        }
        else
        { /* some 1 some 0 */
            general_qg_generation(gates, tt0, var_index_pure - 1, controls_new0,
                                  orders);
            general_qg_generation(gates, tt1, var_index_pure - 1, controls_new1,
                                  orders);
        }
    }
}
// to do: need to complete and verify
template <typename Network>
void qc_generation(Network & net,
    std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>> gates)
{
    for (auto i = 0u; i < gates.size(); i++)
    {
        for (const auto & g : gates[i])
        {
            auto angle = g.first;
            auto controls = g.second;
            std::vector<tweedledum::io_id> qcontrols;
            /* convert controls to qubit controls */
            for (auto i = 0u; i < controls.size(); i++)
            {
                auto c = (controls[i] % 2) ? tweedledum::io_id(controls[i] / 2, 1, 1) : tweedledum::io_id(controls[i] / 2, 1, 0);
                qcontrols.emplace_back(c);
            }

            if (controls.size() == 0)
            {
                net.add_gate(tweedledum::gate_base(tweedledum::gate_lib::ry, angle), tweedledum::io_id(i, 1));
            }
            else if ((controls.size() == 1) && (std::abs(angle - M_PI) < 0.001))
            {
                net.add_gate(tweedledum::gate_lib::cx, qcontrols[0], tweedledum::io_id(i, 1));
            }
            else
            { /* we have multi control probability gate */
                net.add_gate(tweedledum::gate_base(tweedledum::gate_lib::mcry, angle), qcontrols, std::vector<tweedledum::io_id>{tweedledum::io_id(i, 1)});
            } // end multi control

        } // end loop for gates on a line
    }     // end loop for lines
}

template <typename Network>
void qsp_ownfunction(Network &net, const kitty::dynamic_truth_table tt,
                     qsp_tt_statistics &stats,
                     std::vector<uint32_t> const &orders)
{
    std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>
        gates;

    auto var_idx = tt.num_vars() - 1;

    std::vector<uint32_t> cs;
    general_qg_generation(gates, tt, var_idx, cs, orders);
    qc_generation(net, gates);
    gates_count_analysis(gates, orders, tt.num_vars(), stats);

    // detail::control_line_cancelling(gates,tt_vars);
    // detail::extract_multiplex_gates(net,tt_vars,gates);
}

template <typename Network>
void qsp_allone_first(Network &net, const kitty::dynamic_truth_table tt,
                      qsp_tt_statistics &stats,
                      std::vector<uint32_t> const &orders)
{
    std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>
        gates;

    auto ones = kitty::count_ones(tt);
    auto tt_new = kitty::create<kitty::dynamic_truth_table>(tt.num_vars());
    for (auto i = 0u; i < ones; i++)
        kitty::set_bit(tt_new, i);

    auto var_idx = tt.num_vars() - 1;
    std::vector<uint32_t> cs;
    general_qg_generation(gates, tt_new, var_idx, cs, orders);
    // detail::control_line_cancelling(gates,tt_vars);

    // qc_generation(net,gates);

    // std::vector<io_id> qubits(tt_vars);
    // std::iota(qubits.begin(), qubits.end(), 0u);
    // std::vector<uint32_t> perm;

    // for(auto i=0u;i<tt_str.size();i++)
    //     if(tt_str[i]=='1')
    //         perm.emplace_back(i);
    // for(auto i=ones;i<tt_str.size();i++)
    //     perm.emplace_back(i);

    // for(auto i=0u;i<perm.size();i++)
    // std::cout<<perm[i]<<" ";

    // detail::tbs_unidirectional(net, qubits, perm,ones);
}

} // namespace detail
//**************************************************************
struct qsp_params
{
    enum class strategy : uint32_t
    {
        allone_first,
        ownfunction,
    } strategy = strategy::ownfunction;
};

template <class Network>
void qsp_tt(Network &network, const kitty::dynamic_truth_table tt,
            std::vector<uint32_t> const &orders, qsp_tt_statistics &stats,
            qsp_params params = {})
{
    // assert(tt_str.size() <= pow(2,6));
    const uint32_t num_qubits = tt.num_vars();
    for (auto i = 0u; i < num_qubits; ++i)
    {
        network.add_qubit();
    }

    // stopwatch<>::duration time_traversal{0};
    // {
    //     stopwatch t(time_traversal);
    switch (params.strategy)
    {
    case qsp_params::strategy::allone_first:
        detail::qsp_allone_first(network, tt, stats, orders);
        break;
    case qsp_params::strategy::ownfunction:
        detail::qsp_ownfunction(network, tt, stats, orders);
        break;
    }
    // }

    // std::cout << "time = " << to_seconds(time_traversal) << "s" << std::endl;
}

template <class Network>
void qsp_tt(Network &network, const kitty::dynamic_truth_table tt,
            qsp_tt_statistics &stats, qsp_params params = {})
{
    qsp_tt(network, tt, detail::initialize_orders(tt.num_vars()), stats, params);
}

} // end namespace angel
