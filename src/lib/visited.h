#ifndef VISITED_H
#define VISITED_H

#include <stdio.h>
#include <math.h>
#include <vector>
#include "conf.h"
#include <algorithm>
#include "common.h"

struct ele
{
	std::vector<double> x;	//the designing variables
	double			    f;	//the function value
//	std::vector<double> d;	//the deriatives
//	the derivatives will be encoded into two string of bits, i.e. two integers
//	string one (d_nzero) is to show whether the deriative is zero on one direction
//	string two (d_positive) is to show whether the deriative is positive or negative on one direction if it's not zero
//	
//	note that long type is for 32 bits, hopefully it will be enough for this
	long long d_zero;// if zero, bit=1; if not zero, bit=0
	long long d_positive;// positive, bit=1; negative, bit=0

	ele(std::vector<double> x_, double f_, std::vector<double> d_)
	{
		this->x=std::vector<double>(x_);
		this->f= f_;
//		this->d=std::vector<double>(d_);
		d_zero=0x0000000000000000;
		d_positive=0x0000000000000000;
		const long long ONE=0x0000000000000001;
		long long bitMask  =0x0000000000000000;

		//N.B.: now, d_zero and d_positive count from right to left
		for (int i=0;i<d_.size();i++){
			bitMask=ONE<<i;
//			d_zero=d_zero<<1;
//			d_positive=d_positive<<1;

			if (d_[i]==0)
				d_zero |= bitMask;
			else if (d_[i]>0)
				d_positive |= bitMask;
		}
	}

//	ele(){
////		this->x=NULL;
////		this->d=NULL;
//	}

	inline long long getMask()
	{
		long long out=(1<<(this->x.size()))-1;
		return out; 
	} 

	inline int size()
	{
		return x.size();
	}

	void print()
	{
		::print(x);printf(" %f\n",f);
		printf("d_zero=%lld\td_positive=%lld\n",d_zero,d_positive);
	}

	double dist2(std::vector<double>);

	bool check(std::vector<double>, double, std::vector<double>);
};


struct visited {
	std::vector<ele> list;
	int n_variable;
	int p;
	bool full;
	
	inline int get_maxCheck(){
//		return ceil(1.5*n_variable);
		return 4*n_variable;
	}

	inline int get_maxSize(){
		return 10*n_variable;
	}

	bool interesting(conf x, double f,change g);

	visited(){
//		std::cout<<"Visited Instance created"<<std::endl;
		list=std::vector<ele>();
		n_variable=0;
		p=0;
		full=false;
	}

	bool add(conf conf_v, double f, change change_v)
	{
		std::vector<double> tempx =std::vector<double>();
		conf_v.getV(tempx);
		std::vector<double> tempd =std::vector<double>();
		change_v.getV(tempd);
		double tempf =f;

		if (list.size()==0){
			n_variable=tempx.size();
		} else {
			if (tempx.size()!=n_variable){
				printf("local search designing variables not the same");
				return false;
			}
		}
		
		ele e(tempx,tempf, tempd);
		
		if (!full){
			list.push_back(e);
			if (list.size()>=get_maxSize()){
				full=true;
				p=0;
			}
		} else {
			list[p]=e;
			p=(p+1)%(get_maxSize());
		}
		 
		return true;
	} 
	
	inline ele get(int i)
	// no boundary check
	{ 
		return list[i];
	} 

	inline int size()
	{
		return list.size();
	} 

	void print()
	 {
		for (int i=0;i<size();i++)
	 	{
			this->get(i).print();
			printf("\n");
		}
	}
};

#endif
