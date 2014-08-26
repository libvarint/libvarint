#define VARINT_DEBUG 1
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "formula.hpp"

using namespace std;
using namespace varint::formula;

int main() {
	unique_ptr< Element<int> > zero(new Constant<int>(0));
	unique_ptr< Element<int> > one(new Constant<int>(1));
	unique_ptr< Element<int> > one1(new Constant<int>(1));
	unique_ptr< Element<int> > two(new Constant<int>(2));
	unique_ptr< Element<int> > two1(new Constant<int>(2));
	unique_ptr< Element<int> > three(new Constant<int>(3));
	unique_ptr< Element<int> > four(new Constant<int>(4));
	unique_ptr< Element<int> > var(new Variable<int>("a"));
	unique_ptr< Element<int> > var1(new Variable<int>("a"));
	unique_ptr< Element<int> > varb(new Variable<int>("b"));
	unique_ptr< Element<int> > varb1(new Variable<int>("b"));
	unique_ptr< Element<int> > varc(new Variable<int>("c"));
	unique_ptr< Element<int> > varc1(new Variable<int>("c"));
	unique_ptr< Element<int> > varc2(new Variable<int>("c"));
	unique_ptr< Element<int> > varc3(new Variable<int>("c"));
	unique_ptr< Element<int> > varc4(new Variable<int>("c"));
 	std::map<const std::string, unique_ptr< Element<int> > > values;
 	std::map<const std::string, int> vals;
 	vals["a"] = 2;
 	values["a"] = move(unique_ptr< Element<int> >(new Constant<int>(2)));
	std::vector< unique_ptr< Element<int> > > v;
	v.push_back(move(zero));
 	unique_ptr< Sum<int> > sum = unique_ptr< Sum<int> >(new Sum<int>(v));
 	unique_ptr< Ratio<int> > ratio(new Ratio<int>(std::move(one1), std::move(two1)));
 	unique_ptr< Product<int> > product(new Product<int>());
 	unique_ptr< Product<int> > product1(new Product<int>());
 	unique_ptr< Product<int> > product2(new Product<int>());
 	unique_ptr< Power<int> > power(new Power<int>(std::move(var1), unique_ptr< Element<int> >(new Constant<int>(2))));
 	product->append(move(one));
 	product->append(move(two));
 	product->append(move(var));
 	product->append(move(varc));
 	product->append(move(varb));
 	product->append(move(varc1));
 	product->append(move(varc2));
 	product->append(move(varb1));
 	product1->append(move(varc3));
 	product1->append(move(three));
 	product2->append(move(four));
 	product2->append(move(varc4));
 	values["b"] = move(product2);
 	cout<<*product<<"\n";
 	product->sort();
 	cout<<*product<<"\n";
 	product->collectConstants();
 	cout<<*product<<"\n";
 	product->collect();
 	cout<<*product<<"\n";
 	sum->append(move(product));
 	sum->append(unique_ptr< Element<int> >(new Constant<int>(5)));
 	sum->append(move(product1));
 	cout<<*sum<<"\n";
 	sum->sort();
 	cout<<*sum<<"\n";
 	sum->collect();
 	cout<<*sum<<"\n";
 	unique_ptr< Formula<int> > formula(new Formula<int>(std::move(sum)));
 	formula->simplifyObject();
 	cout<<*formula<<"\n";
 	unique_ptr< Element<int> > tmpRes = std::move(formula->evaluate(values));
 	cout<<*tmpRes<<"\n";
 	tmpRes->collect();
 	cout<<*tmpRes<<"\n";
 	tmpRes->collect();
 	cout<<*tmpRes<<"\n";
// 	Element<int> *tmp;
	cout<<*ratio<<"\n";
 	cout<<*power<<"\n";
// 	cout<<sum->nevaluate(vals)<<"\n";
// //	cout<<ratio->nevaluate(vals)<<"\n";
// 	cout<<product->nevaluate(vals)<<"\n";
// //	cout<<power->nevaluate(vals)<<"\n";
// 	product->collectConstants();
// 	cout<<*product<<"\n";
// 	product->collect();
// 	cout<<*product<<"\n";
// 	sum->collectConstants();
// 	cout<<*formula<<"\n";
	return 0;
}