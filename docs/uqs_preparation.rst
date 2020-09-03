UQS Preparation Algorithms
============================================
No efficient algorithm is known for preparing arbitrary quantum states.  In the worst case, all existing algorithms require an exponential number of elementary quantum gates and runtime in the number of qubits.  Uniform quantum states are a subclass of arbitrary quantum states, which are superpositions over a subset of basis states, where all amplitudes are either zero or have the same value.  Although uniformity is a restriction on arbitrary quantum states, uniform quantum states frequently appear as the input states of important quantum algorithms and have many practical applications. 

The central idea of uniform QSP is that each uniform quantum state can be characterized by a Boolean function, which allows us to draw from the rich fund of Boolean approaches for analyzing and synthesizing circuit implementations.

Theorem
  Each $n$-qubit uniform quantum state :math:`|\phi_{f(x)}\rangle` corresponds one-to-one to an $n$-variable Boolean function $f(x)$, such that
 
    :math:`|\phi_{f(x)}\rangle = \frac{1}{|\mathrm{Min}(f)|^{-2}} \sum_{\hat x \in \mathrm{Min}(f)} |\hat{x}\rangle`
  
  holds, where $\mathrm{Min}(f)$ denotes the minterms of $f$.
\end{theorem}

Theorem~\ref{thm:boolchar} states that it is possible to map uniform quantum states into Boolean functions, i.e., the column vector representation~$|\phi_{f(x)}\rangle$ of a uniform quantum state can be expressed as the superposition of those basis states $|\hat x \rangle$ for which $f(\hat x) = 1$ normalized by the square-root of the number of minterms of~$f$.

Functional decomposition
------------------------


Functional dependency
---------------------


