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
	double			    f;	//the function values
//	std::vector<double> d;	//the deriatives
//	the deriatives will be encoded into two string of bits, i.e. two integers
//	string one (d_nzero) is to show whether the deriative is zero on one direction
//	string two (d_positive) is to show whether the deriative is positive or negative on one direction if it's not zero
//	
//	note that long type is for 32 bits, hopefully it will be enough for this
	long long d_zero;// if zero, bit=1; if not zero, bit=0
	long long d_positive;// positive, bit=1; negative, bit=0

	ele(const std::vector<double>& x_, double f_, const std::vector<double>& d_)
		: x(x_), f(f_), d_zero(0), d_positive(0) {
//		this->d=std::vector<double>(d_);
		const long long ONE=1;
		long long bitMask =0;

		//N.B.: now, d_zero and d_positive count from right to left
		for (int i=0;i<d_.size();i++){
			bitMask=ONE<<i;
//			d_zero=d_zero<<1;
//			d_positive=d_positive<<1;

			if (d_[i]==0) d_zero |= bitMask;
			else if (d_[i]>0) d_positive |= bitMask;
		}
	}

	ele()
	{
//		this->x=NULL;
//		this->d=NULL;
	}

	inline long long getMask()
	{
		long long out=(1<<(this->x.size()))-1;
		return out; 
	} 

	inline int size() const
	{
		return x.size();
	}

	void print() const
	{
		::print(x);printf(" %f\n",f);
		printf("d_zero=%lld\td_positive=%lld\n",d_zero,d_positive);
	}

	double dist2(const std::vector<double>& now) const;

	bool check(const std::vector<double>& now_x, double now_f, const std::vector<double>& now_d) const;
};


struct visited
{
	std::vector<ele> list;
	int n_variable;
	std::vector<double> tempx;
	std::vector<double> tempd;
	std::vector<double> dist_scratch;
	std::vector<unsigned char> not_picked_scratch;
	int p;
	bool full;
	


	bool interesting(const conf& x, double f, const change& g);

	visited()
	{ 
		list=std::vector<ele>();
		n_variable=0;
		p=0;
		full=false;
	}

	bool add(const conf& conf_v, double f, const change& change_v)
	{
		tempx.clear();
		tempd.clear();
		if(n_variable > 0) {
			tempx.reserve(n_variable);
			tempd.reserve(n_variable);
		}
		conf_v.getV(tempx);
		change_v.getV(tempd);

		if (list.size()==0)
		{
			n_variable=tempx.size();
			tempx.reserve(n_variable);
			tempd.reserve(n_variable);
			list.reserve(10 * n_variable);
			dist_scratch.reserve(10 * n_variable);
			not_picked_scratch.reserve(10 * n_variable);
		}
		else
		{
			if (tempx.size()!=n_variable)
			{
				printf("local search designing variables not the same");
				return false;
			}
		}
		
		if (!full)
		{
			list.emplace_back(tempx, f, tempd);
			if (list.size()>=10*n_variable)
			{
				full=true;
				p=0;
			}
		}
		else
		{
			list[p]=ele(tempx, f, tempd);
			p=(p+1)%(10*n_variable);
		}
		 
		return true;
	} 
	
	inline const ele& get(int i) const
	// no boundary check
	{ 
		return list[i];
	} 

	inline int size() const
	{
		return list.size();
	} 

	void print() const
		 {
			for (int i=0;i<size();i++)
		 	{
			this->get(i).print();
			printf("\n");
		}
	}
};

#endif
