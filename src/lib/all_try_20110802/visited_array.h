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
	std::vector<double> d;	//the deriatives

	ele(std::vector<double> x_, std::vector<double> d_)
	{ 
		this->x=std::vector<double>(x_);
		this->d=std::vector<double>(d_);
	}
	ele()
	{ 
//		this->x=NULL;
//		this->d=NULL;
	}

	inline int size()
	{
		return x.size();
	}

	void print()
	{
		::print(x);
		printf("\n");
		::print(d);
		printf("\n");
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
