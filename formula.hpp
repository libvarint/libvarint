#ifndef VARINT_FORMULA_HPP
#define VARINT_FORMULA_HPP

#include <vector>
#include <iostream>
#include <map>
#include <cmath>

namespace varint {
namespace formula {

#define DEFAULT_POWER_PRECISION -6 // 10^(-6)

template<class Field> class Variable;
template<class Field> class Constant;

template<class Field> class Element {
protected:
	int id;
	Element<Field> *parent;
public:
	Element() : id(0) {}
	Element(Element<Field>* _parent) : parent(_parent) {}
	void setParent(Element<Field> *_parent) { parent = _parent; }
	Element<Field> *getParent() { return parent; }
	virtual void canonify() {}
	virtual Element<Field>* copy() const { return new Element<Field>(*this); }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const { return new Element<Field>(parent); }
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return 0; }
	virtual void substitute(std::map<const std::string, Element<Field>*> values) { parent->replace(id, evaluate(values)); }
	Element<Field> *solve_for(Variable<Field> *variable) const {}
	virtual std::ostream& print(std::ostream& os) const {
		os<<"";
		return os;
	}
	void replace(int id, Element<Field> *elem) {}
	friend std::ostream& operator<<(std::ostream& os, const Element<Field>& elem) {
		return elem.print(os);
	}
};

template<class Field> class Constant : public Element<Field> {
protected:
	Field value;
public:
	Constant(const Field _value) : value(_value) {}
	virtual Element<Field>* copy() const { return new Constant<Field>(*this); }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const { return new Constant<Field>(value); }
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return value; }
	virtual std::ostream& print(std::ostream& os) const {
		os<<value;
		return os;
	}
};

template<class Field> class Variable : public Element<Field> {
protected:
	std::string name;
public:
	Variable(const std::string _name) : name(_name) {}
	virtual Element<Field>* copy() const { return new Variable<Field>(*this); }
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return values[name]; }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const {
		if (values.count(name)>0)
			return values[name]->copy();
		else
			return new Variable<Field>(name);
	}
	virtual std::ostream& print(std::ostream& os) const {
		os<<name;
		return os;
	}
};

template<class Field> class Sum : public Element<Field> {
protected:
	std::vector<Element<Field>*> terms;
public:
	virtual Element<Field>* copy() const { return new Sum<Field>(*this); }
	virtual void canonify() { for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++) (*term)->canonify(); }
	virtual void compress() {}
	virtual void together() {}
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const {
		Field sum = 0;
		for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++)
			sum += ((Element<Field>*)(*term))->nevaluate(values, power_precision);
		return sum;
	}
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) {
		Sum<Field> *ret = new Sum<Field>();
		for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++) {
			ret->add(((Element<Field>*)(*term))->evaluate(values));
		}
		return ret;
	}
	virtual void add(Element<Field>* term) { terms.push_back(term); term->setParent(this); }
	virtual std::ostream& print(std::ostream& os) const {
		for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++) {
			if (term != terms.begin()) {
				os<<"+";
			}
			os<<**term;
		}
		return os;
	}
};

template<class Field> class Product : public Element<Field> {
protected:
	std::vector<Element<Field>*> terms;
public:
	virtual Element<Field>* copy() { return new Product<Field>(*this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { 
		Field ret = 1;
		for (typename std::vector<Element<Field>*>::iterator term = terms.begin(); term!=terms.end(); term++) {
			ret *= ((Element<Field>*)(*term))->nevaluate(values);
		}
		return ret;
	}
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) {
		Product<Field> *ret = new Product<Field>();
		for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++) {
			ret->add(((Element<Field>*)(*term))->evaluate(values));
		}
		return ret;
	}
	virtual void add(Element<Field>* term) { terms.push_back(term); term->setParent(this); }
	virtual std::ostream& print(std::ostream& os) const {
		for (typename std::vector<Element<Field>*>::const_iterator term = terms.begin(); term != terms.end(); term++) {
			if (term != terms.begin()) {
				os<<"*";
			}
			os<<**term;
		}
		return os;
	}

	virtual void expand() { }
	virtual void canonify() { }
};

template<class Field> class Ratio : public Element<Field> {
protected:
	Element<Field> *numerator;
	Element<Field> *denominator;
public:
	Ratio() {}
	Ratio(Element<Field> *_numerator, Element<Field> *_denominator) : numerator(_numerator), denominator(_denominator) { _numerator->setParent(this); _denominator->setParent(this); }
	virtual Element<Field>* copy() { return new Ratio<Field>(*this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return numerator->nevaluate(values)/denominator->nevaluate(values); }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const {
		Ratio<Field> *ret = new Ratio<Field>();
		ret->setNumerator(numerator->evaluate(values));
		ret->setDenominator(denominator->evaluate(values));
		return ret;
	}
	virtual void setNumerator(Element<Field>* elem) {
		numerator = elem;
		elem->setParent(this);
	}
	virtual void setDenominator(Element<Field>* elem) {
		denominator = elem;
		elem->setParent(this);
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<"\\frac{"<<*numerator<<"}{"<<*denominator<<"}";
		return os;
	}
};

template<class Field, unsigned int nargs> class Function : public Element<Field> {
protected:
	std::string name;
	Element<Field>* expressions[nargs];
public:
	Function(std::string _name) : name(_name) {}
	virtual Element<Field>* copy() { return new Function<Field, nargs>(*this); }
	virtual void setExpression(unsigned int index, Element<Field>* _expression) { expressions[index] = _expression; _expression->setParent(this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return 0; }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const {
		Function<Field, nargs> *ret = new Function<Field, nargs>(name);
		for(unsigned int i=0; i<nargs; i++) {
			ret->setExpression(i, expressions[i]);
		}
		return ret;
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<*name<<"(";
		for(unsigned int i=0; i<nargs; i++) {
			os<<*expressions[i];
			if (i>0) os<<",";
		}
		os<<")";
	}
};

template<class Field> class Function<Field, 1> : public Element<Field> {
protected:
	std::string name;
	Element<Field>* expression;
public:
	Function(std::string _name, Element<Field> *_expression) : name(_name), expression(_expression) { _expression->setParent(this); }
	virtual Element<Field>* copy() { return new Function<Field, 1>(*this); }
	virtual void setExpression(Element<Field>* _expression) { expression = _expression; _expression->setParent(this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return expression->nevaluate(values); }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const {
		Function<Field, 1> *ret = new Function<Field, 1>(name, expression->copy()->evaluate(values));
		return ret;
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<name<<"("<<*expression<<")";
		return os;
	}
};


template<class Field> class Power : public Function<Field, 1> {
protected:
	Element<Field> *power;
public:
	Power(Element<Field> *_expression, Element<Field> *_power) : Function<Field, 1>("power", _expression), power(_power) { _power->setParent(this); }
	virtual Element<Field>* copy() { return new Power<Field>(*this); }
	virtual void setPower(Element<Field>* _power) { power = _power; _power->setParent(this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return pow(this->expression->nevaluate(values), power->nevaluate(values)); }
	virtual Element<Field>* evaluate(std::map<const std::string, Element<Field>*> values) const {
		Power<Field> *ret = new Power<Field>(this->expression->copy()->evaluate(values), power->copy()->evaluate(values));
		return ret;
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<"{"<<*(this->expression)<<"}^{"<<*power<<"}";
		return os;
	}
};

template<class Field> class InfiniteSum : public Element<Field> {
protected:
	Variable<Field> *index;
	Element<Field> *expression;
	int starting_index;
};

template<class Field> class FiniteSum : public InfiniteSum<Field> {
protected:
	int ending_index;
};

template<class Field> class TaylorSeries : public InfiniteSum<Field> {
protected:
	//std::vector<Range<Field> *> convergence_region;
};

template<class Field> class Integral : public Element<Field> {
protected:
	std::vector<Variable<Field>*> int_variables;
	Element<Field> *expression;
};

template<class Field> class DefiniteIntegral : public Integral<Field> {
protected:
//	std::vector<Range<Field> *> *boundary;
};

template<class Field> class Differential : public Element<Field> {
protected:
	std::vector<Variable<Field>*> diff_variables;
	Element<Field> *expression;
};

	
template<class PhaseSpace, class Field> class Formula : public Element<Field> {
private:
	Element<Field> *formulaElement;

public:
	Formula(const Formula<PhaseSpace, Field> &_formula) {}
	Formula(Element<Field> *_formulaElement) : formulaElement(_formulaElement) { formulaElement->setParent(this); }
	virtual Element<Field>* copy() const { return new Formula<PhaseSpace, Field>(*this); }
	virtual std::ostream& print(std::ostream& os) const {
		os<<*formulaElement;
		return os;
	}

};

}
}

#endif // VARINT_FORMULA_HPP