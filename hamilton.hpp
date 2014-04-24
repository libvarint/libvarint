#ifndef VARINT_HAMILTON_HPP
#define VARINT_HAMILTON_HPP

#include "lie"

namespace varint {

template<class PhaseSpace> class HamiltonIntegrator : public BaseIntegrator<PhaseSpace> {
	HamiltonIntegrator(Fomula<S> _L, double _t0, double _t1, double _t_step, S _initial_pos) 
	: BaseIntegrator<PhaseSpace>(_L, _t0, _t1, _t_step, _initial_pos) {}
}

}

#endif