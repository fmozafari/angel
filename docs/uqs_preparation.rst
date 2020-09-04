UQS Preparation Algorithms
============================================
No efficient algorithm is known for preparing arbitrary quantum states.  In the worst case, all existing algorithms require an exponential number of elementary quantum gates and runtime in the number of qubits.  `Uniform quantum states` (UQS) are a subclass of arbitrary quantum states, which are superpositions over a subset of basis states, where all amplitudes are either zero or have the same value.  Although uniformity is a restriction on arbitrary quantum states, uniform quantum states frequently appear as the input states of important quantum algorithms and have many practical applications. 

The central idea of UQSP is that each uniform quantum state can be characterized by a Boolean function, which allows us to draw from the rich fund of Boolean approaches for analyzing and synthesizing circuit implementations.

Theorem
  Each :math:`n`-qubit uniform quantum state :math:`|\phi_{f(x)}\rangle` corresponds one-to-one to an :math:`n`-variable Boolean function :math:`f(x)`, such that
 
    :math:`|\phi_{f(x)}\rangle = \frac{1}{|\mathrm{Min}(f)|^{-2}} \sum_{\hat x \in \mathrm{Min}(f)} |\hat{x}\rangle`
  
  holds, where :math:`\mathrm{Min}(f)` denotes the minterms of :math:`f`.

This theorem states that it is possible to map uniform quantum states into Boolean functions, i.e., the column vector representation :math:`|\phi_{f(x)}\rangle` of a uniform quantum state can be expressed as the superposition of those basis states :math:`|\hat x \rangle` for which :math:`f(\hat x) = 1` normalized by the square-root of the number of minterms of :math:`f`. Using Boolean functions, we proposed two algorithms based on ``functional decomposition`` and ``functional dependency``.

Functional decomposition
------------------------
Representing uniform quantum states as Boolean functions allows us to employ the Shannon decomposition to solve the state preparation problem recursively :cite:`Mozafari20`.  Our algorithm iterates over the variables of the Boolean function, which correspond to qubits, and prepares them one by one, by computing the probability of being zero for the variable depending on previously prepared variables.  This computational step requires to count the number of ones for each recursive co-factor of the Boolean function.  The probability is then the number of ones of the current function divided by the number of ones of the negative co-factor.  We have presented an implementation of this algorithm in :cite:`Mozafari20` using `Binary Decision Diagrams` (BDDs) :cite:`bryant1986graph` as a representation of Boolean functions and dynamic programming.  BDDs are particularly suitable for our purpose because counting and co-factoring can be very efficiently implemented as BDD operations :cite:`bryant1986graph`.

**Header:** ``angel/quantum_state_preparation/qsp_bdd.hpp``

.. doxygenfunction:: angel::qsp_bdd


Functional dependency
---------------------
The construction presented in the previous section is complete and allows us to generate a quantum circuit for every uniform quantum state.  In several cases, however, the recursive decomposition can be avoided in favor of more optimized constructions if a functional dependency among the current and the previously prepared qubits is recognized :cite:`Mozafari21`.  Such functional dependencies have been developed in the context of logic synthesis.  The identified functional dependencies for a qubit :math:`q_i` can be utilized in three ways: (1) to reduce the number of control qubits if :math:`q_i` depends only on a subset of the previously prepared qubits, (2) to reduce the number of elementary quantum gates if the functional dependency can be well expressed with the library of hardware supported quantum gates, and (3) to reduce the number of control lines for preparing other next qubits to be prepared.  We have presented two approaches to identify functional dependencies in :cite:`Mozafari21` and implemented them using truth tables-based algorithms: the first approach, pattern search, identifies dependencies among variables that have a fixed and predefined structure; the second approach, ESOP synthesis, uses a SAT-based synthesis algorithm :cite:`RienerES+20` for `Exclusive-or Sum-Of-Product` (ESOP) forms with a modified cost function.  Finding dependencies in form of ESOP expressions with an XOR with many fanins and ANDs with only few fanins are particularly useful because they are the most general dependency structure that allow us to reduce the number of elementary gates.  Moreover, we make use of variable reordering to ensure that no beneficial dependency is overlooked.

**Header:** ``angel/quantum_state_preparation/qsp_deps.hpp``

.. doxygenclass:: angel::qsp_deps


