/* author: Fereshte */
#include <catch.hpp>
#include <angel/quantum_state_preparation/qsp_tt_dependencies.hpp>

#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <kitty/constructors.hpp>

using namespace angel;

TEST_CASE("Prepare GHZ(3) state with qsp_tt_dependencies method", "[qsp_tt_dependencies]")
{
	tweedledum::netlist<tweedledum::mcmt_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "10000001");

	std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> deps;
	deps[0].emplace_back(std::make_pair("eq", std::vector<uint32_t>{4}));
	deps[1].emplace_back(std::make_pair("eq", std::vector<uint32_t>{4}));

	qsp_tt_deps_statistics stats;
	qsp_tt_dependencies(network, tt, deps, stats);

	CHECK(stats.total_cnots == 2u);
}

TEST_CASE("Prepare w(3) state with qsp_tt_dependencies method", "[qsp_tt_dependencies]")
{
	tweedledum::netlist<tweedledum::mcmt_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "01101000");

	std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> deps;
	deps[0].emplace_back(std::make_pair("xnor", std::vector<uint32_t>{4,2}));

	qsp_tt_deps_statistics stats;
	qsp_tt_dependencies(network, tt, deps, stats);

	CHECK(stats.total_cnots == 4u);
}

TEST_CASE("Prepare ""1000" "0010" "0001" "0010"" state and utilize reordering", "[qsp_tt_dependencies]")
{
    kitty::dynamic_truth_table tt(4);
    std::string tt_str = "1100" "0010" "0010" "0001";
    kitty::create_from_binary_string(tt, tt_str);

    /* first order */
    std::vector<uint32_t> orders_1;
    for(int32_t i=4-1; i>=0; i--)
        orders_1.emplace_back(i);
    functional_dependency_stats stats_temp;
    auto deps = functional_dependency_analysis( tt, stats_temp , orders_1);
    /* 1 = and(2,3)*/
    CHECK(deps[1].size()==1);
    CHECK(deps[1][0].first=="and");
    CHECK(deps[1][0].second.size()==2);
    tweedledum::netlist<tweedledum::mcmt_gate> ntk;
    qsp_tt_deps_statistics qsp_stats_temp;
    qsp_tt_dependencies(ntk, tt, deps, qsp_stats_temp);
    CHECK(qsp_stats_temp.total_cnots==14);

    /* second order */
    std::vector<uint32_t> orders_2;
    orders_2.emplace_back(3); orders_2.emplace_back(2); orders_2.emplace_back(0); orders_2.emplace_back(1);
    functional_dependency_stats stats_temp2;
    angel::reordering_on_tt_inplace(tt,orders_2);
    auto deps2 = functional_dependency_analysis( tt, stats_temp2 , orders_1);
    tweedledum::netlist<tweedledum::mcmt_gate> ntk2;
    qsp_tt_deps_statistics qsp_stats_temp2;
    qsp_tt_dependencies(ntk2, tt, deps2, qsp_stats_temp2);
    CHECK(qsp_stats_temp2.total_cnots==10);

}
