.. angel documentation master file, created by
   sphinx-quickstart on Thu JUN 9 12:31:14 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to angel's documentation!
======================================

.. toctree::
   :maxdepth: 2
   
   introduction
   installation
   changelog
   acknowledgments

.. toctree::
   :maxdepth: 2
   :caption: Standard library
   QSP
   using_functional_decomposition
   using_functional_dependency

.. toctree::
   :maxdepth: 2
   :caption: dependency_analysis

   dependency_analysis/common
   dependency_analysis/no_deps
   dependency_analysis/pattern_based_dependency_analysis
   dependency_analysis/esop_based_dependency_analysis


.. toctree::
   :maxdepth: 2
   :caption: reordering

   reordering/no_reordering
   reordering/exhaustive_reordering
   reordering/random_reordering
   reordering/greedy_reordering

.. toctree::
   :maxdepth: 2
   :caption: Generators

   generators/arithmetic
   generators/control
   generators/modular_arithmetic
   generators/majority

.. toctree::
   :maxdepth: 2
   :caption: quantum_state_preparation

   quantum_state_preparation/qsp_tt_general
   quantum_state_preparation/qsp_add

.. toctree::
   :maxdepth: 2
   :caption: Utilities

   utils/function_extractor
   utils/helper_functions
   utils/stopwatch

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
