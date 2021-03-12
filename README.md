[![Actions Status](https://github.com/fmozafari/angel/workflows/Linux%20CI/badge.svg)](https://github.com/fmozafari/angel/actions)
[![Actions Status](https://github.com/fmozafari/angel/workflows/MacOS%20CI/badge.svg)](https://github.com/fmozafari/angel/actions)
[![Actions Status](https://github.com/fmozafari/angel/workflows/Windows%20CI/badge.svg)](https://github.com/fmozafari/angel/actions)
[![Coverage Status](https://coveralls.io/repos/github/fmozafari/angel/badge.svg?branch=master)](https://coveralls.io/github/fmozafari/angel?branch=master)
[![Documentation Status](https://readthedocs.org/projects/libangel/badge/?version=latest)](https://libangel.readthedocs.io/en/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# angel
<img src="https://github.com/fmozafari/angel/blob/master/angel.svg" width="64" height="64" align="left" style="margin-right: 12pt" />
angel is a C++-17 Quantom State Preparation (QSP) library. It provides several algorithm for QSP with the purpose of CNOTs reduction and increasing speed.

Read the full documentation [here](https://libangel.readthedocs.io/en/latest/index.html).

## Example

The following code prepares GHZ(3) state by utilizing dependencies between qubits. To prepare the state variable reordering and dependency analysis methods are specified. Finally, the number of CNOTs and single-qubit gates reports. 

```c++
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <kitty/constructors.hpp>
#include <angel/dependency_analysis/esop_based_dependency_analysis.hpp>
#include <angel/quantum_state_preparation/qsp_deps.hpp>
#include <angel/reordering/no_reordering.hpp>

tweedledum::netlist<tweedledum::mcmt_gate> network;
kitty::dynamic_truth_table tt( 3 );
kitty::create_from_binary_string( tt, "10000001" );

/* reordering strategy */
angel::no_reordering no_reorder;

/* dependency analysis method */
typename angel::esop_deps_analysis::parameter_type esop_ps;
typename angel::esop_deps_analysis::statistics_type esop_st;
angel::esop_deps_analysis esop( esop_ps, esop_st );

angel::state_preparation_parameters qsp_ps;
angel::state_preparation_statistics qsp_st;
angel::qsp_deps<decltype( network ), decltype( esop ), decltype( no_reorder )> qsp( network, esop, no_reorder, qsp_ps, qsp_st );
qsp( tt );
qsp_st.report();
```

# Installation requirements
A modern compiler is required to build angel. We are continously testing with Clang 6.0.1, GCC 7.3.0, and GCC 8.2.0. More information can be found in the [documentation](https://libangel.readthedocs.io/en/latest/installation.html).
