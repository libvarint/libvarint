#ifndef VARINT_HPP
#define VARINT_HPP

#include "hamiltonint"

namespace varint {

template<class PhaseSpace> class PhaseSpaceIterator {
private:
	BaseIntegrator<PhaseSpace> & integrator;
	double cur_timestep;
public:
	PhaseSpaceIterator(BaseIntegrator<PhaseSpaceIterator> & _integrator, double _cur_timestep) 
	: integrator(_integrator), cur_timestep(_cur_timestep) {}
	PhaseSpaceIterator & operator++() { integrator.step(); return *this; }

}

template<class PhaseSpace> BaseIntegrator {
private:
	Formula<S> formula;
	double t0;
	double t1;
	double t_step;
	PhaseSpace initial_position;
public:
	typedef PhaseSpaceIterator<PhaseSpace> iterator;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef PhaseSpace value_type;
	typedef PhaseSpace * pointer;
	typedef PhaseSpace & reference;
	iterator begin() { return iterator( *this, t0); }
	iterator end() { return iterator( *this, t1); }
public:
	BaseIntegrator(Fomula<S> _F, double _t0, double _t1, double _t_step, S _initial_pos) 
	: formula(_F), t0(_t0), t1(_t1), t_step(_t_step), initial_position(_initial_pos) {}
	void step() {  }
}
	
}
#endif // VARINT_HPP