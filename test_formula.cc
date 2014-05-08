#include <iostream>
#include <string>
#include <map>
#include "formula.hpp"

using namespace std;
using namespace varint::formula;

int main() {
	Constant<int> *one = new Constant<int>(1);
	Constant<int> *two = new Constant<int>(2);
	Variable<int> *var = new Variable<int>("a");
	Variable<int> *varb = new Variable<int>("b");
	Variable<int> *varc = new Variable<int>("c");
	Sum<int> *sum = new Sum<int>();
	Ratio<int> *ratio = new Ratio<int>(one, two);
	Product<int> *product = new Product<int>();
	Power<int> *power = new Power<int>(var, two);
	product->add(one);
	product->add(two);
	product->add(var);
	product->add(varc);
	product->add(varb);
	Element<int> *tmp;
	std::map<const std::string, Element<int>*> values;
	std::map<const std::string, int> vals;
	vals["a"] = 2;
	values["a"] = two;
	sum->add(one);
	sum->add(var);
	cout<<*one<<"\n";
	cout<<*var<<"\n";
	cout<<*sum<<"\n";
	cout<<*ratio<<"\n";
	cout<<*product<<"\n";
	product->sort();
	cout<<*product<<"\n";
	cout<<*power<<"\n";
	cout<<*(var->evaluate(values))<<"\n";
	tmp = sum->evaluate(values);
	cout<<*tmp<<"\n";
	cout<<sum->nevaluate(vals)<<"\n";
	cout<<ratio->nevaluate(vals)<<"\n";
	cout<<product->nevaluate(vals)<<"\n";
	cout<<power->nevaluate(vals)<<"\n";
	cout<<*(product->evaluate(values))<<"\n";
	cout<<*(power->evaluate(values))<<"\n";
	sum->collectConstants();
	cout<<*sum<<"\n";
	return 0;
}