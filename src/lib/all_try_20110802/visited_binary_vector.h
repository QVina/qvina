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
//	std::vector<double> d;	//the deriatives
//	the deriatives will be encoded into two string of bits, i.e. two integers
//	string one (d_nzero) is to show whether the deriative is zero on one direction
//	string two (d_positive) is to show whether the deriative is positive or negative on one direction if it's not zero
//	
//	note that long type is for 64 bits, hopefully it will be enough for this
	long d_zero;// if zero, bit=1; if not zero, bit=0
	long d_positive;// positive, bit=1; negative, bit=0

	ele(std::vector<double> x_, std::vector<double> d_)
	{ 
		this->x=std::vector<double>(x_);
//		this->d=std::vector<double>(d_);
		d_zero=0;
		d_positive=0;

		for (int i=0;i<d_.size();i++)
		{
			
			d_zero=d_zero<<1;
			d_positive=d_positive<<1;

			if (d_[i]==0) d_zero+=1;
			else if (d_[i]>0) d_positive+=1;
		}
	}

	ele()
	{ 
//		this->x=NULL;
//		this->d=NULL;
	}

	inline long getMask()
	{
		long out=(1<<(this->x.size()))-1;
		return out; 
	}

	inline int size()
	{
		return x.size();
	}

	void print()
	{
		::print(x);
		printf("\n");
		printf("d_zero=%ld\td_positive=%ld\n",d_zero,d_positive);
	}

	double dist2(std::vector<double>);

	bool check(std::vector<double>);
};


struct visited
{
	std::vector<ele> list;
	int n_variable;
	std::vector<double> tempx;
	std::vector<double> tempd;
	


	bool interesting(conf x,change g);

	visited()
	{
		list=std::vector<ele>();
		n_variable=0;
	}

	bool add(conf conf_v, change change_v)
	{
		tempx=std::vector<double>();
		tempd=std::vector<double>();
		conf_v.getV(tempx);
		change_v.getV(tempd);

		if (list.size()==0)
		{
			n_variable=tempx.size();
		}
		else
		{
			if (tempx.size()!=n_variable)
			{
				printf("local search designing variables not the same");
				return false;
			}
		}
		ele e(tempx,tempd);
		list.push_back(e);
		 
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
