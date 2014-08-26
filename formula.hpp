#ifndef VARINT_FORMULA_HPP
#define VARINT_FORMULA_HPP

#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <memory>
#include <tuple>

namespace varint {
namespace formula {

#define EL_PTR Element<Field> *
#define EL_UPTR std::unique_ptr< Element<Field> >
#define EL_SPTR std::shared_ptr< Element<Field> >
#define VA_UPTR std::unique_ptr< Variable<Field> >

#define DEFAULT_POWER_PRECISION -6 // 10^(-6)

// Forward definitions
template<class Field> class Element;
template<class Field> class Constant;
template<class Field> class Variable;
template<class Field, template <class> class CombinerInner > class Collection;
template<class Field> class Sum;
template<class Field> class Product;
template<class Field> class Power;
template<class Field> class Ratio;

template<class Field> struct Intersection {
	EL_UPTR common;
	EL_UPTR remainder1;
	EL_UPTR remainder2;
};

template<class Field> EL_UPTR solve_for(const EL_UPTR& expression) { return nullptr; }

template<class Field> class Combiner {
public:
	virtual Field combine(Field a, Field b) const { return 0; }
	virtual EL_UPTR combine(const EL_UPTR &el1, const EL_UPTR &el2) const { return nullptr; }
	virtual Field initial() const { return 0; }
	virtual std::string symbol() const { return ""; }
};

template<class Field> class Addition : public Combiner<Field> {
public:
	virtual Field combine(Field a, Field b) const { return a+b; }
	virtual EL_UPTR combine(const EL_UPTR &el1, const EL_UPTR &el2) const {
		std::unique_ptr< Product<Field> > ret = std::unique_ptr< Product<Field> >(new Product<Field>());
		Field res = initial();
		ret->append(EL_UPTR(new Constant<Field>(res)));
		ret->collectConstants();
		ret->simplifyObject();
		return std::move(ret);
	}
	virtual Field initial() const { return 0; }
	virtual std::string symbol() const { return "+"; }
};

template<class Field> class Multiplication : public Combiner<Field> {
public:
	virtual Field combine(Field a, Field b) const { return a*b; }
	virtual EL_UPTR combine(const EL_UPTR &el1, const EL_UPTR &el2) const {
		std::unique_ptr< Power<Field> > ret = std::unique_ptr< Power<Field> >(new Power<Field>());
		Power<Field> *tmpPow1 = dynamic_cast<Power<Field>*>(el1.get());
		Power<Field> *tmpPow2 = dynamic_cast<Power<Field>*>(el2.get());
		std::unique_ptr< Sum<Field> > sum = std::unique_ptr< Sum<Field> >(new Sum<Field>());
		EL_UPTR term1, term2, expr;
		if (tmpPow1 != nullptr) {
			term1 = std::move(tmpPow1->getPower()->clone());
			expr = std::move(tmpPow1->getExpression()->clone());
		} else {
			term1 = EL_UPTR(new Constant<Field>(initial()));
			expr = std::move(el1->clone());
		}
		if (tmpPow2 != nullptr) {
			term2 = std::move(tmpPow2->getPower()->clone());
		} else {
			term2 = EL_UPTR(new Constant<Field>(initial()));
		}
		ret->setExpression(std::move(expr));
		sum->append(std::move(term1));
		sum->append(std::move(term2));
		sum->collectConstants();
		ret->setPower(std::move(sum));
		ret->simplifyObject();
		return std::move(ret);
	}
	virtual Field initial() const { return 1; }	
	virtual std::string symbol() const { return "*"; }
};

template<class Field> class Element {
protected:
	unsigned int id;
	EL_PTR parent;
public:
	Element(EL_PTR _parent = nullptr) : id(0), parent(_parent) {}
	Element(Element<Field> const &other) {}
	void setParent(EL_PTR _parent) { parent = _parent; }
	virtual void canonify() {}
	virtual EL_UPTR clone(bool empty=false) const = 0;
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const {
		return { nullptr, std::move(this->clone()), std::move(with->clone()) };
	} 
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const = 0;
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return 0; }
	virtual std::ostream& print(std::ostream& os) const { os<<""; return os; }
	virtual void simplifyObject() {}
	virtual const std::string stringify() const {
		std::ostringstream stream;
		this->print(stream);
		return stream.str();
	}
	virtual const std::string similarity(const Combiner<Field> &combiner) const { return this->stringify(); }
	virtual const std::string similarity(const Multiplication<Field> &combiner) const { return this->stringify(); }
	virtual const std::string similarity(const Addition<Field> &combiner) const { return this->stringify(); }
	void setId(const unsigned int _id) { id = _id; }
	const unsigned int getId() const { return id; }
	virtual void collect() {}
	virtual void replaceById(const unsigned int id, EL_UPTR elem) {}
	void replace(const EL_UPTR& elem1, EL_UPTR elem2) { this->replaceById(elem1->getId(), std::move(elem2)); }
	virtual void remove(const EL_UPTR& elem) { this->removeById(elem->getId()); }
	virtual void removeById(const unsigned int id) {}
	virtual bool compareWithField(const Field &num) const { return false; }
	virtual bool compareWithString(const std::string &_name) const { return false; }
	Element<Field>& operator = (Element<Field>& other) { return *this; }
	bool operator == (const Field &num) { return this->compareWithField(num); }
	bool operator == (const std::string &name) { return this->compareWithString(name); }
	friend std::ostream& operator<<(std::ostream& os, const Element<Field>& elem) {
		return elem.print(os);
	}
	friend std::ostream& operator<<(std::ostream& os, const EL_UPTR &elem) {
		return elem->print(os);
	}
};

template<class Field, class Derived> class CloneableElement:public Element<Field> {
public:
	CloneableElement(EL_PTR _parent=nullptr) : Element<Field>(_parent) {}
	virtual EL_UPTR clone(bool empty=false) const { return EL_UPTR(new Derived(static_cast<const Derived&>(*this))); }
};

template<class Field> class Constant : public CloneableElement<Field, Constant<Field> > {
private:
	typedef CloneableElement<Field, Constant<Field> > Base;
protected:
	Field value;
public:
	Constant(const Field _value = 0, EL_PTR _parent=nullptr) : value(_value), Base(_parent) {}
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const { return std::move(this->clone()); }
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return value; }
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const { 
		if (Constant<Field> *tmp = dynamic_cast< Constant<Field>* >(with.get())) {
			return { EL_UPTR(new Constant<Field>(combiner.initial())), std::move(this->clone()), std::move(with->clone()) };
		} else {
			return { nullptr, std::move(this->clone()), std::move(with->clone()) }; 
		}
	} 
	virtual const Field& getValue() const { return value; }
	virtual std::ostream& print(std::ostream& os) const {
		os<<value;
		return os;
	}
	virtual bool compareWithField(const Field &num) const { return num==value; }
	Constant<Field>& operator = (Constant<Field>& other) { value=other.getValue(); return *this; }
};

template<class Field> class Variable : public CloneableElement<Field, Variable<Field> > {
private:
	typedef CloneableElement<Field, Variable<Field> > Base;
protected:
	std::string name;
public:
	Variable(const std::string _name, EL_PTR _parent=nullptr) : name(_name), Base(_parent) {}
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return values[name]; }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const {
		if (values.count(name)>0)
			return std::move((values.at(name))->clone());
		else
			return std::move(this->clone());
	}
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const { 
		if (Variable<Field> *tmp = dynamic_cast< Variable<Field>* >(with.get())) {
			if (tmp->getName()==getName()) {
				return { std::move(this->clone()), nullptr, nullptr };
			}
		}
		return { nullptr, std::move(this->clone()), std::move(with->clone()) }; 
	} 
	virtual bool compareWithString(const std::string &_name) const { return _name==name; }
	virtual const std::string& getName() const { return name; }
	virtual std::ostream& print(std::ostream& os) const {
		os<<name;
		return os;
	}
	Variable<Field>& operator = (Variable<Field>& other) { name=other.getName(); return *this; }
};

template<class Field, const unsigned int nargs> class Function;

template<class Field, template <class> class CombinerInner > class Collection : public Element<Field> {
private:
	typedef Element<Field> Base;
protected:
	bool sorted;
	const CombinerInner<Field> combiner = CombinerInner<Field>();
	std::vector< EL_UPTR > terms;
	unsigned int max_id;
	struct Less {
		Less(const Collection<Field, CombinerInner >& c) : curObject(c) {}
		bool operator () ( const EL_UPTR &i1, const EL_UPTR &i2 ) { return i1->similarity(curObject.combiner) < i2->similarity(curObject.combiner); } 
		const Collection<Field, CombinerInner >& curObject;
	};
public:
	Collection(EL_PTR _parent = nullptr) : sorted(true), max_id(0), Base(_parent) {}
	Collection(const Collection<Field, CombinerInner> &other) {
		const std::vector< EL_UPTR > & otherTerms = other.getTerms();
		if (!otherTerms.empty()) {
			terms.reserve(otherTerms.size());
			for (auto term = otherTerms.begin(); term != otherTerms.end(); term++) {
				append(std::move((*term)->clone()));
			}
		}
	}
	Collection(const std::vector< EL_UPTR >& _terms, EL_PTR _parent = nullptr) : sorted(true), max_id(0), Base(_parent) { 
		if (!_terms.empty()) {
			terms.reserve(_terms.size());
			for (auto term = _terms.begin(); term != _terms.end(); term++) {
				append(std::move((*term)->clone()));
			}
		}
	}
	virtual void canonify() { for (auto term = terms.begin(); term != terms.end(); term++) (*term)->canonify(); }
	virtual void compress() {}
	virtual void together() {}
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const {
		Field result = combiner.initial();
		for (auto term = terms.begin(); term != terms.end(); term++)
			result = combiner.combine(result,(*term)->nevaluate(values, power_precision));
		return result;
	}
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const {
		EL_UPTR ret = std::move(this->clone(true));
		for (auto term = terms.begin(); term != terms.end(); term++) {
			((Collection<Field, CombinerInner>*)ret.get())->append(std::move((*term)->evaluate(values)));
		}
		return std::move(ret);
	}
	virtual void simplifyObject() {
		sort();
		collectConstants();
		std::vector< EL_UPTR > *tmpVec = new std::vector< EL_UPTR >();
		auto term=terms.begin();
		while (term!=terms.end()) {
			(*term)->simplifyObject();
			if (Collection<Field, CombinerInner> *tmpTerm = dynamic_cast< Collection<Field, CombinerInner> *>((*term).get())) {
				for(auto _term=tmpTerm->getTerms().begin();_term!=tmpTerm->getTerms().end();_term++) {
					tmpVec->push_back(std::move((*_term)->clone()));
				}
				terms.erase(term);
			} else term++;
		}
		if (tmpVec->size() > 0) {
			appendTerms(*tmpVec);
			sort();
			collectConstants();
		}
		delete tmpVec;

		if (terms.size() == 1) {
			if (this->parent != nullptr)
				this->parent->replaceById(this->getId(), std::move((*(terms.begin()))->clone()));
		} else if (terms.size() == 0) {
			if (this->parent != nullptr)
				this->parent->removeById(this->getId());
		}
	}
	virtual void collectConstants() {
		Field res = combiner.initial();
		auto term = terms.begin();
		while (term != terms.end()) {
			if (Constant<Field> *cur = dynamic_cast<Constant<Field> *>((*term).get())) {
				res = combiner.combine(res, cur->getValue());
				terms.erase(term);
			} else term++;
		}
		if (res != combiner.initial()) terms.insert(terms.begin(), EL_UPTR(new Constant<Field>(res)));
	}
	virtual void sort() { std::sort(terms.begin(), terms.end(), Less(*this)); sorted = true; }
	void append(EL_UPTR term) { if (term) { term->setId(max_id++); term->setParent(this); terms.push_back( std::move(term) ); sorted = false; } }
	void clear() { terms.clear(); }
	unsigned int size() { return terms.size(); }
	virtual EL_UPTR clone(bool empty=false) const = 0;
	virtual void collect() {
		for (auto _term = terms.begin(); _term != terms.end(); _term++) (*_term)->collect();
		simplifyObject();
	}
	virtual void replaceById(const unsigned int _id, EL_UPTR elem) {
		for(auto term=terms.begin(); term != terms.end(); term++) {
			if((*term)->getId()==_id) {
				elem->setId(_id);
				elem->setParent(this);
				*term = std::move(elem);
			}
		}
		sorted = false;
	}
	virtual void removeById(const unsigned int _id) {
		auto term = terms.begin();
		while (term != terms.end()) {
			if((*term)->getId()==_id) {
				terms.erase(term);
			} else term++;
		}
	}
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const = 0;
	virtual const EL_UPTR& term(unsigned int i) const { return terms.at(i); }
	virtual const std::vector< EL_UPTR >& getTerms() const { return terms; }
	virtual void replaceTerms(const std::vector< EL_UPTR > &other) { terms.clear(); appendTerms(other); }
	virtual void appendTerms(const std::vector< EL_UPTR > &other) { for(auto term=other.begin(); term!=other.end(); term++) this->append(std::move((*term)->clone())); }
	virtual std::ostream& print(std::ostream& os) const {
		bool brackets = false;
		if (this->parent && dynamic_cast< Collection<Field, CombinerInner> *>(this->parent)) {
			brackets = true;
		}
		if (brackets) os<<"(";
		for (auto term = terms.begin(); term != terms.end(); term++) {
			if (term != terms.begin()) {
				os<<combiner.symbol();
			}
			os<<**term;
		}
		if (brackets) os<<")";
		return os;
	}
	Collection<Field, CombinerInner>& operator = (const Collection<Field,CombinerInner>& other) { 
		Element<Field>::operator=(other); 
		const std::vector< EL_UPTR >& otherTerms = other.getTerms();
		if (!otherTerms.empty()) {
			terms.reserve(otherTerms.size());
			for (auto term=otherTerms.begin();term!=otherTerms.end();term++) {
				terms.append(std::move((*term)->clone()));
			}
		}
		return *this; 
	}
};

template<class Field, template <class> class CombinerInner, class Derived> class CloneableCollection: public Collection<Field, CombinerInner > {
private:
	typedef Collection< Field, CombinerInner > Base;
public:
	CloneableCollection(EL_PTR _parent = nullptr) : Base(_parent) {}
	CloneableCollection(const CloneableCollection<Field, CombinerInner, Derived> &other) : Base(static_cast<const Derived&>(other)) {}
	CloneableCollection(const std::vector< EL_UPTR >& _terms, EL_PTR _parent = nullptr) : Base(_terms, _parent) {}
	virtual EL_UPTR clone(bool empty=false) const { return empty?EL_UPTR(new Derived()):EL_UPTR(new Derived(static_cast<const Derived&>(*this))); }
};

template<class Field> class Sum :  public CloneableCollection<Field, Addition, Sum<Field> > {
private:
	typedef CloneableCollection<Field, Addition, Sum<Field> > Base;
public:
	Sum(EL_PTR _parent=nullptr) : Base(_parent) {}
	Sum(const std::vector< EL_UPTR >& _terms, EL_PTR _parent=nullptr) : Base(_terms, _parent) {}
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const {
		EL_UPTR common, remainder1, remainder2;
		if (Sum<Field> *tmp = dynamic_cast<Sum<Field> *>(with.get())) {
			Sum<Field>* _common = new Sum<Field>();
			Sum<Field>* _remainder1 = new Sum<Field>();
			Sum<Field>* _remainder2 = new Sum<Field>();
			auto a=this->terms.begin();
			auto b=tmp->getTerms().begin();
			if (dynamic_cast<const Addition<Field> *>(&combiner)) {
				while((a!=this->terms.end())&&(b!=tmp->getTerms().end())) {
					if ((*a)->similarity(combiner)<(*b)->similarity(combiner)) {
						a++;
					} else if ((*a)->similarity(combiner)==(*b)->similarity(combiner)) {
						_common->append(std::move((*a)->clone()));
						b++;
						a++;
					} else {
						b++;
					}
				}
				if (_common->size()>1) {
					common = EL_UPTR(_common);
				} else if (_common->size()==1) {
					common = std::move(_common->term(0)->clone());
					delete _common;
				} else {
					common = nullptr;
					delete _common;
				}
				if (_remainder1->size()>1) {
					remainder1 = EL_UPTR(_remainder1);
				} else if (_remainder1->size()==1) {
					remainder1 = std::move(_remainder1->term(0)->clone());
					delete _remainder1;
				} else {
					remainder1 = nullptr;
					delete _remainder1;
				}
				if (_remainder2->size()>1) {
					remainder2 = EL_UPTR(_remainder2);
				} else if (_remainder2->size()==1) {
					remainder2 = std::move(_remainder2->term(0)->clone());
					delete _remainder2;
				} else {
					remainder2 = nullptr;
					delete _remainder2;
				}
			} else if (dynamic_cast<const Multiplication<Field> *>(&combiner)) {
				while((a!=this->terms.end())&&(b!=tmp->getTerms().end())) {
					if ((*a)->similarity(combiner)!=(*b)->similarity(combiner)) {
						break;
					} else {
						a++;
						b++;
					}
				}
				if ((a!=this->terms.end())||(b!=tmp->getTerms().end())) {
					common = nullptr;
					remainder1 = std::move(this->clone());
					remainder2 = std::move(with->clone());
				} else {
					common = std::move(this->clone());
					remainder1 = nullptr;
					remainder2 = nullptr;
				}
			}
		} else {
			Multiplication<Field> *tmpCombiner = new Multiplication<Field>();
			common = std::move(with->clone());
			remainder2 = nullptr;
			for (auto term = this->terms.begin(); term != this->terms.end(); term++) {
				Intersection<Field> t = (*term)->intersect(common, *tmpCombiner);
				if (t.common != nullptr) {
					common = std::move(t.common);
					remainder2 = std::move(t.remainder2);
				} else {
					common = nullptr;
					remainder2 = std::move(with->clone());
					break;
				}
			}
			delete tmpCombiner;
			if (common != nullptr) {
				Sum<Field>* tmpSum = new Sum<Field>();
				for(auto term = this->terms.begin();term != this->terms.end(); term++) {
					Intersection<Field> t = (*term)->intersect(common, combiner);
					if (t.remainder1 != nullptr) tmpSum->append(std::move(t.remainder1));
				}
				tmpSum->collectConstants();
				remainder1 = EL_UPTR(tmpSum);
			} else {
				remainder1 = std::move(this->clone());	
			}
		}
		return { std::move(common), std::move(remainder1), std::move(remainder2) }; 
	} 
	virtual void collect() {
		Base::collect();

		Sum<Field> *finalSum = new Sum<Field>();
		Sum<Field> *tmpSum = new Sum<Field>();

		auto term = this->terms.begin();
		auto prev = term;
		while (term != this->terms.end()) {
			if ((std::next(term)==this->terms.end()) || ((prev!=term) && ((*term)->similarity(this->combiner)!=(*prev)->similarity(this->combiner)) ) ) {
				if (std::next(term)==this->terms.end()) tmpSum->append(std::move((*term)->clone()));
				if (tmpSum->size()>1) { 
					Intersection<Field> _int = tmpSum->intersect(*prev, this->combiner);
					if (_int.common) {
						Product<Field> *tmp = new Product<Field>();
						if (_int.remainder1 != nullptr) tmp->append(std::move(_int.remainder1));
						tmp->append(std::move(_int.common));
						finalSum->append(EL_UPTR(tmp));
					} else {
						finalSum->append(std::move(tmpSum->clone()));
					}
				} else if (tmpSum->size()==1) {
					finalSum->append(std::move((*(tmpSum->getTerms().begin()))->clone()));
				}
				tmpSum->clear();
			}
			tmpSum->append(std::move((*term)->clone()));
			prev = term;
			term++;
		}
		delete tmpSum;
		if (this->parent != nullptr) {
			this->parent->replaceById(this->getId(), EL_UPTR(finalSum));
		} else {
			this->replaceTerms(finalSum->getTerms());
			delete finalSum;
		}
		this->simplifyObject();
	}
};

template<class Field> class Product : public CloneableCollection<Field, Multiplication, Product<Field> > {
private:
	typedef CloneableCollection<Field, Multiplication, Product<Field> > Base;
public:
	Product(EL_PTR _parent = nullptr) : Base(_parent) {}
	Product(const std::vector< EL_UPTR > &_terms, EL_PTR _parent) : Base(_terms, _parent) {}
	virtual const std::string similarity(const Addition<Field> &combiner) const {
		std::string ret;
		bool first = true;
		for (auto term = this->terms.begin(); term != this->terms.end(); term++) {
			Constant<Field> *tmp_const = dynamic_cast< Constant<Field> *>((*term).get());
			if (tmp_const) continue;
			if (first) ret += (*term)->stringify();
			else ret += (this->combiner).symbol() + (*term)->stringify();
		}
		return ret;
	}
	virtual Intersection<Field> intersect(const EL_UPTR& with, const Combiner<Field> &combiner ) const { 
		EL_UPTR common, remainder1, remainder2;
		if (Product<Field> *tmp = dynamic_cast<Product<Field> *>(with.get())) {
			Product<Field>* _common = new Product<Field>();
			Product<Field>* _remainder1 = new Product<Field>();
			Product<Field>* _remainder2 = new Product<Field>();
			auto a=this->terms.begin();
			auto b=tmp->getTerms().begin();
			if (dynamic_cast<const Addition<Field> *>(&combiner)) {
				while((a!=this->terms.end())&&(b!=tmp->getTerms().end())) {
					if ((*a)->similarity(combiner) < (*b)->similarity(combiner)) {
						a++;
					} else if ((*a)->similarity(combiner)==(*b)->similarity(combiner)) {
						_common->append(std::move((*a)->clone()));
						b++;
						a++;
					} else {
						b++;
					}
				}
				common = EL_UPTR(_common);
				remainder1 = EL_UPTR(_remainder1);
				remainder2 = EL_UPTR(_remainder2);
			} else if (dynamic_cast<const Multiplication<Field> *>(&combiner)) {
				while((a!=this->terms.end())&&(b!=tmp->getTerms().end())) {
					if ((*a)->similarity(combiner)!=(*b)->similarity(combiner)) {
						break;
					} else {
						a++;
						b++;
					}
				}
				if ((a!=this->terms.end())||(b!=tmp->getTerms().end())) {
					common = nullptr;
					remainder1 = std::move(this->clone());
					remainder2 = std::move(with->clone());
				} else {
					common = std::move(this->clone());
					remainder1 = nullptr;
					remainder2 = nullptr;
				}
			}
		} else {
			Addition<Field> *tmpCombiner = new Addition<Field>();
			common = std::move(with->clone());
			remainder2 = nullptr;
			for (auto term = this->terms.begin(); term != this->terms.end(); term++) {
				if (dynamic_cast< Constant<Field> *>((*term).get())) continue;
				Intersection<Field> t = (*term)->intersect(common, *tmpCombiner);
				if (t.common != nullptr) {
					common = std::move(t.common);
					remainder2 = std::move(t.remainder2);
				} else {
					common = nullptr;
					remainder2 = std::move(with->clone());
					break;
				}
			}
			delete tmpCombiner;
			if (common != nullptr) {
				Product<Field>* tmpProd = new Product<Field>();
				for(auto term = this->terms.begin(); term != this->terms.end(); term++) {
					if (dynamic_cast< Constant<Field> *>((*term).get())) continue;
					Intersection<Field> t = (*term)->intersect(common, combiner);
					if (t.remainder1 != nullptr) tmpProd->append(std::move(t.remainder1));
				}
				remainder1 = EL_UPTR(tmpProd);
			} else {
				remainder1 = std::move(this->clone());	
			}
		}
		return { std::move(common), std::move(remainder1), std::move(remainder2) }; 
	} 
	virtual const std::string similarity(const Multiplication<Field> &combiner) const {
		std::string ret;
		bool first = true;
		for (auto term = this->terms.begin(); term != this->terms.end(); term++) {
			Constant<Field> *tmp_const = dynamic_cast< Constant<Field> *>((*term).get());
			if (tmp_const) continue;
			if (first) ret += (*term)->stringify();
			else ret += (this->combiner).symbol() + (*term)->stringify();
		}
		return ret;
	}
	virtual void collect() {
		Base::collect();

		Product<Field> *finalProd = new Product<Field>();
		Product<Field> *tmpProd = new Product<Field>();

		auto term = this->terms.begin();
		auto prev = term;
		while (term != this->terms.end()) {
			if ((std::next(term)==this->terms.end()) || ((prev!=term) && ((*term)->similarity(this->combiner)!=(*prev)->similarity(this->combiner)) ) ) {
				if (std::next(term)==this->terms.end()) tmpProd->append(std::move((*term)->clone()));
				Intersection<Field> _int = { nullptr, nullptr, nullptr};
				bool firstContant = false;
				if (dynamic_cast< Constant<Field> *>((*(tmpProd->getTerms().begin())).get())) {
					// If first element is a constant
					firstContant = true;
					if (tmpProd->getTerms().size() > 3) {
						_int = tmpProd->intersect(*(++tmpProd->getTerms().begin()), this->combiner);
					}
				} else {
					_int = tmpProd->intersect(*(tmpProd->getTerms().begin()), this->combiner);
				}
				if (_int.common) {
					Product<Field> *_tmpProd = new Product<Field>();
					Power<Field> *commonPow = new Power<Field>();
					if (Power<Field> * tmpPow = dynamic_cast< Power<Field> *>(_int.common.get())) {
						Sum<Field> *tmpSum = new Sum<Field>();
						tmpSum->append(EL_UPTR(new Constant<Field>(firstContant?tmpProd->getTerms().size()-1:tmpProd->getTerms().size())));
						tmpSum->append(std::move(tmpPow->getPower()->clone()));
						commonPow->setPower(EL_UPTR(tmpSum));
						commonPow->setExpression(std::move(tmpPow->getExpression()->clone()));
					} else {
						commonPow->setPower(EL_UPTR(new Constant<Field>(firstContant?tmpProd->getTerms().size()-1:tmpProd->getTerms().size())));
						commonPow->setExpression(std::move(_int.common));
					}
					if (firstContant) _tmpProd->append(std::move((*(tmpProd->getTerms().begin()))->clone()));
					if (((Product<Field> *)(_int.remainder1.get()))->getTerms().size() > 0) _tmpProd->append(std::move(_int.remainder1));
					_tmpProd->append(EL_UPTR(commonPow));
					finalProd->append(EL_UPTR(_tmpProd));
				} else {
					finalProd->append(std::move(tmpProd->clone()));
				}
				tmpProd->clear();
			}
			tmpProd->append(std::move((*term)->clone()));
			prev = term;
			term++;
		}
		delete tmpProd;
		if (this->parent != nullptr) {
			this->parent->replaceById(this->getId(), EL_UPTR(finalProd));
		} else {
			this->replaceTerms(finalProd->getTerms());
			delete finalProd;
		}
		this->simplifyObject();
	}
};

template<class Field> class Ratio : public CloneableElement<Field, Ratio<Field> > {
private:
	typedef CloneableElement<Field, Ratio<Field> > Base;
protected:
	EL_UPTR numerator;
	EL_UPTR denominator;
	enum { NUMERATOR_ID, DENOMINATOR_ID };
public:
	Ratio(EL_PTR _parent=nullptr) : Base(_parent) {}
	Ratio(const Ratio<Field> &other) : numerator(std::move((other.getNumerator())->clone())), denominator(std::move((other.getDenominator())->clone())) { if (numerator) { numerator->setParent(this); numerator->setId(NUMERATOR_ID); } if (denominator) { denominator->setParent(this); denominator->setId(DENOMINATOR_ID); } }
	Ratio(EL_UPTR _numerator, EL_UPTR _denominator, EL_PTR _parent=nullptr) : numerator(std::move(_numerator)), denominator(std::move(_denominator)), Base(_parent) { if (numerator) { numerator->setParent(this); numerator->setId(NUMERATOR_ID); } if (denominator) { denominator->setParent(this); denominator->setId(DENOMINATOR_ID); } }
	virtual Element<Field>* copy() { return new Ratio<Field>(*this); }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return numerator->nevaluate(values)/denominator->nevaluate(values); }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const {
		Ratio<Field> *ret = new Ratio<Field>();
		ret->setNumerator(numerator->evaluate(values));
		ret->setDenominator(denominator->evaluate(values));
		return EL_UPTR(ret);
	}
	virtual void setNumerator(EL_UPTR elem) {
		numerator = std::move(elem);
		numerator->setId(NUMERATOR_ID);
		numerator->setParent(this);
	}
	virtual void setDenominator(EL_UPTR elem) {
		denominator = std::move(elem);
		denominator->setId(DENOMINATOR_ID);
		denominator->setParent(this);
	}
	virtual const EL_UPTR& getNumerator() const {
		return numerator;
	}
	virtual const EL_UPTR& getDenominator() const {
		return denominator;
	}
	virtual void simplifyObject() {
		numerator->simplifyObject();
		denominator->simplifyObject();
		if (this->parent != nullptr) {
			if ((*denominator)==1) {
				this->parent->replaceById(this->getId(), std::move(this->numerator->clone()));
			} else if ((*numerator)==0) {
				this->parent->replaceById(this->getId(), EL_UPTR(new Constant<Field>(0)));
			}
		}
	}
	virtual void replaceById(const unsigned int _id, EL_UPTR elem) { 
		if (_id==NUMERATOR_ID) {
			numerator = std::move(elem);
		} else if (_id==DENOMINATOR_ID) {
			denominator = std::move(elem);
		}
	}
	virtual void remove(const unsigned int _id) { 
		if (_id==NUMERATOR_ID) {
			numerator = EL_UPTR(new Constant<Field>(0));
		} else if (_id==DENOMINATOR_ID) {
			denominator = EL_UPTR(new Constant<Field>(1));
		}
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<"\\frac{"<<*numerator<<"}{"<<*denominator<<"}";
		return os;
	}
};

template<class Field, const unsigned int nargs=1> class Function : public CloneableElement<Field, Function<Field, nargs> > {
private:
	typedef CloneableElement<Field, Function<Field, nargs> > Base;
protected:
	std::string name;
	std::array< EL_UPTR, nargs > expressions;
public:
	Function(const std::string _name, EL_PTR _parent=nullptr) : name(_name), Base(_parent) {}
	Function(const Function<Function, nargs>& other) {
		const std::array< EL_UPTR, nargs >& otherArray = other.getExpressions();
		name = other.getName();
		for (auto part=otherArray.begin(); part!=otherArray.end(); part++) {
			setExpression(std::move((*part)->clone()));
		}
	}
	Function(const std::array< EL_UPTR, nargs >& _parts, EL_PTR _parent=nullptr) : Base(_parent) {
		for (auto part=_parts.begin(); part!=_parts.end(); part++) {
			setExpression(std::move((*part)->clone()));
		}
	}
	virtual Field nevaluate(std::map<const std::string, Field> values) { return 0; }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const { 
		EL_UPTR ret = std::move(this->clone());
		return std::move(ret);
	}
	virtual void setExpression(const unsigned int index, EL_UPTR _expression) { expressions[index] = std::move(_expression); if (expressions[index]) { expressions[index]->setParent(this); expressions[index]->setId(index); } }
	virtual void simplifyObject() {
		for (unsigned int i=0; i < nargs; i++) expressions[i]->simplifyObject();
	}
	virtual void replaceById(const unsigned int _id, Element<Field> *elem) {
		expressions[_id] = std::move(elem);
		expressions[_id]->setParent(this);
		expressions[_id]->setId(_id);
	}
	virtual void removeById(const unsigned int _id) {
		expressions[_id] = std::move(EL_UPTR(new Constant<Field>(0)));
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<*name<<"(";
		for(unsigned int i=0; i<nargs; i++) {
			os<<*expressions[i];
			if (i>0) os<<",";
		}
		os<<")";
	}
	virtual const std::string getName() const {
		return name;
	}
	virtual const std::array< EL_UPTR, nargs >& getExpressions() const {
		return expressions;
	}
	virtual const EL_UPTR& getExpression(const unsigned int i) const { 
		if (i < nargs) {
			return expressions[i];
		} else {
			return nullptr;
		}
	}
};

template<class Field> class Function<Field, 1> : public CloneableElement<Field, Function<Field> > {
private:
	typedef CloneableElement<Field, Function<Field> > Base;
protected:
	std::string name;
	EL_UPTR expression;
public:
	Function(const std::string _name, EL_UPTR _expression=nullptr, EL_PTR _parent=nullptr) : name(_name), expression(std::move(_expression)), Base(_parent) { if (expression) { expression->setParent(this); expression->setId(0); } }
	Function(const Function<Field>& other) : name(other.getName()), expression(std::move((other.getExpression())->clone())) { if (expression) { expression->setParent(this); expression->setId(0); } }
	virtual Field nevaluate(std::map<const std::string, Field> values) { return expression->nevaluate(values); }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const { 
		EL_UPTR ret = std::move(this->clone());
		return std::move(ret);
	}
	virtual void replaceById(const unsigned int _id, EL_UPTR elem) {
		expression = std::move(elem);
		expression->setParent(this);
		expression->setId(0);
	}
	virtual void removeById(const unsigned int _id) {
		expression = std::move(EL_UPTR(new Constant<Field>(0)));
	}
	virtual void simplifyObject() {
		expression->simplifyObject();
	}
	virtual const std::string getName() const {
		return name;
	}
	virtual void setExpression(EL_UPTR _expression) { expression = std::move(_expression); if (expression) { expression->setParent(this); expression->setId(0); } }
	virtual const EL_UPTR& getExpression() const { 
		return expression;
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<name<<"("<<*expression<<")";
		return os;
	}
};

template<class Field, unsigned int nargs, class Derived> class CloneableFunction:public Function<Field, nargs> {
private:
	typedef Function< Field, nargs > Base;
public:
	CloneableFunction(const std::string _name, EL_PTR _parent=nullptr) : Base(_name, _parent) {}
	CloneableFunction(const CloneableFunction<Field, nargs, Derived> &other) : Base(static_cast<const Derived&>(other)) {}
	CloneableFunction(const std::array< EL_UPTR, nargs >& _parts, EL_PTR _parent=nullptr) : Base(_parts, _parent) {}
	virtual EL_UPTR clone(bool empty=false) const { return EL_UPTR(new Derived(static_cast<const Derived&>(*this))); }
};

template<class Field, class Derived> class CloneableFunction<Field, 1, Derived>:public Function<Field, 1> {
private:
	typedef Function< Field, 1 > Base;
public:
	CloneableFunction(const std::string _name, EL_UPTR _expression=nullptr, EL_PTR _parent=nullptr) : Base(_name, std::move(_expression), _parent) {}
	CloneableFunction(const CloneableFunction<Field, 1, Derived> &other) : Base(static_cast<const Derived&>(other)) {}
	virtual EL_UPTR clone(bool empty=false) const { return EL_UPTR(new Derived(static_cast<const Derived&>(*this))); }
};

#define POWER_ID 1
#define EXPRESSION_ID 0

template<class Field> class Power : public CloneableFunction<Field, 1, Power<Field> > {
private:
	typedef CloneableFunction<Field, 1, Power<Field> > Base;
protected:
	EL_UPTR power;
public:
	Power(EL_UPTR _expression=nullptr, EL_UPTR _power=nullptr, EL_PTR _parent=nullptr) : Base("power", std::move(_expression), _parent), power(std::move(_power)) { if (power) { power->setParent(this); power->setId(POWER_ID); } }
	Power(const Power<Field>& other) : Base("power",std::move((other.getExpression())->clone())), power(std::move((other.getPower())->clone())) { if (power) { power->setParent(this); power->setId(POWER_ID); } }
	virtual void setPower(EL_UPTR _power) { power = std::move(_power); if (power) { power->setParent(this); power->setId(POWER_ID); } }
	virtual const EL_UPTR& getPower() const { return power; }
	virtual Field nevaluate(const std::map<const std::string, Field> values) { return pow(this->expression->nevaluate(values), power->nevaluate(values)); }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const {
		return EL_UPTR(new Power<Field>(std::move(this->expression->evaluate(values)), std::move(power->evaluate(values))));
	}
	virtual void replaceById(const unsigned int _id, EL_UPTR elem) {
		if (_id==EXPRESSION_ID) {
			this->expression = std::move(elem);
			if (this->expression) {
				this->expression->setParent(this);
				this->expression->setId(0);
			}
		} else if (_id==POWER_ID) {
			power = std::move(elem);
			if (power) {
				power->setParent(this);
				power->setId(0);
			}
		}
	}
	virtual void removeById(const unsigned int _id) {
		if (_id==EXPRESSION_ID) { 
			this->expression = std::move(EL_UPTR(new Constant<Field>(0)));
		} else {
			power = std::move(EL_UPTR(new Constant<Field>(1)));
		}
	}
	virtual const std::string similarity(const Multiplication<Field> &combiner) const {
		return this->expression->similarity(combiner);
	}
	virtual void simplifyObject() {
		this->expression->simplifyObject();
		power->simplifyObject();
		if (this->parent != nullptr) {
			if ((*power)==1) {
				this->parent->replaceById(this->getId(), std::move(this->expression->clone()));
			} else if ((*power)==0) {
				this->parent->replaceById(this->getId(), EL_UPTR(new Constant<Field>(1)));
			} else {
				if (Product<Field> *tmpProd = dynamic_cast< Product<Field> *>(this->expression.get())) {
					Product<Field> *res = new Product<Field>();
					for(auto term=tmpProd->getTerms().begin();term!=tmpProd->getTerms().end();term++) {
						res->append(EL_UPTR(new Power(std::move((*term)->clone()),std::move(power->clone()))));
					}
					this->parent->replaceById(this->getId(), EL_UPTR(res));
				}
			}
		}
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<"{"<<*(this->expression)<<"}^{"<<*power<<"}";
		return os;
	}
};

#define INDEX_ID 1
template<class Field> class InfiniteSum : public CloneableFunction<Field, 1, InfiniteSum<Field> > {
private:
	typedef CloneableFunction<Field, 1, InfiniteSum<Field> > Base;
protected:
	VA_UPTR index;
	int starting_index;
public:
	InfiniteSum(EL_UPTR _expression, VA_UPTR _index, int _starting_index = 1, EL_PTR _parent=nullptr) : Base("infinite_sum",std::move(_expression),_parent), index(std::move(_index)) { if (index) { index->setId(INDEX_ID); index->setParent(this); } }
	InfiniteSum(const InfiniteSum<Field>& other) : Base("infinite_sum", std::move((other->getExpression())->clone())), index(std::move((other->getIndex())->clone())) { if (index) { index->setId(INDEX_ID); index->setParent(this); } }
	virtual void setIndex(VA_UPTR _index) {
		index = std::move(_index);
		if (index) {
			index->setParent(this);
			index->setId(INDEX_ID);
		}
	}
	virtual const VA_UPTR& getIndex() const { return index; }
	virtual void setStartingIndex(int _i) { starting_index = _i; }
	virtual void simplifyObject() {
		this->expression->simplifyObject();
		index->simplifyObject();
	}
	virtual std::ostream& print(std::ostream &os) const {
		os<<"\\sum\\limit_{"<<*index<<"}^{\\infty}{"<<*(this->expression)<<"}";
		return os;
	}
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

	
template<class Field> class Formula : public CloneableElement<Field, Formula<Field> > {
private:
	EL_UPTR root;
	typedef CloneableElement<Field, Formula<Field> > Base;
public:
	Formula(const Formula<Field> &_formula) : Base(nullptr), root(std::move((_formula.getRoot())->clone())) {}
	Formula(EL_UPTR _root, EL_PTR _parent=nullptr) : root(std::move(_root)), Base(_parent)  { root->setParent(this); }
	virtual EL_UPTR evaluate(const std::map<const std::string, EL_UPTR > &values) const { return std::move(root->evaluate(values)); }
	virtual Field nevaluate(std::map<const std::string, Field> values, int power_precision = DEFAULT_POWER_PRECISION) const { return root->nevaluate(values); }
	virtual void simplifyObject() {	root->simplifyObject();	}
	virtual std::ostream& print(std::ostream& os) const {
		os<<*root;
		return os;
	}
	virtual void replaceById(const unsigned int _id, EL_UPTR elem) {
		root = std::move(elem);
	}
	virtual void collect() {
		root->collect();
	}
	virtual void remove(const unsigned int _id) {
		root = EL_UPTR(new Constant<Field>(0));
	}
	virtual const EL_UPTR& getRoot() const { return root; }
};

#undef EL_PTR
#undef EL_UPTR
#undef EL_SPTR
#undef VA_UPTR

}
}

#endif // VARINT_FORMULA_HPP