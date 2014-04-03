libvarint
=========
LibVarInt is a library of variational integrators written on C++. The goal behind the library are:
* Implement easy to use variation integration library
* Implement easily extensible library
* Implement several common and illustrative variational integrators
* Make it fast and efficient
* Implement group operations to work correctly on complex phase spaces

# Variational integrators

Variational integrators are numerical integrators for dynamical systems which can be described by variational principles. There two main variational integrators:
* Lagrangial system integrator based on Hamilton's and Hamilton-Pontryagin principles
* Hamiltonian system integrator based on Hamilton phase space principle

The reason why you might use variational integrator is because of their properties:
* Variatoinal integrators preserve momentum, energy
* Variational integrators preserve geometric structure of the system (for example, they are symplectic in case of sysmplectic systems)

## Hamilton principle integrator

Hamilton principle integrator is designed to integrate Lagrangian systems. Simple example:
	
	#include "libvarint"

	typedef LIE<SO3> S;

	int main() {
		S phase0 = new S();
		S phaset;
		Formula<S> L;
		double t0 = 0.0, t1 = 1.0, t_step = 0.01;
		HamiltonIntegrator<S>  int0;
		HamiltonIntegrator<S>::iterator iter;

		int0 = new HamiltonIntegrator<S>(L, t0, t1, t_step, phase0);
		for (iter = int0.begin(); iter != int0.end(); iter++) {
			phaset = *iter;
			cout<<phaset;
		}
	}

