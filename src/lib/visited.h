#ifndef VISITED_H
#define VISITED_H

#include <stdio.h>
#include <math.h>
//#include <vector>
#include <boost/container/stable_vector.hpp>
#include "conf.h"
#include <algorithm>
#include "common.h"
//#include <mutex>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>


typedef boost::shared_mutex ReadWriteLock;
typedef boost::unique_lock< ReadWriteLock > WriteLock;
typedef boost::shared_lock< ReadWriteLock > ReadLock;

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

	ele(std::vector<double> x_, double f_, std::vector<double> d_): x(std::vector<double>(x_)),f(f_)
	{
//		this->x=std::vector<double>(x_);
//		this->f= f_;

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
	double dist2_3D(std::vector<double>);

	bool check(std::vector<double>, double, std::vector<double>);
};

class linearvisited /*: public visited*/{
public:
private:
	ReadWriteLock myLock;
	boost::container::stable_vector<ele> list;
	linearvisited(){
		list=boost::container::stable_vector<ele>();
	};
	static linearvisited* instance;
	linearvisited(const linearvisited& a);
	linearvisited(linearvisited &a);
	const linearvisited& operator=(const linearvisited& a);

public:
	static linearvisited* getInstance();

	bool interesting(conf x, double f,change g) ;

	inline bool add(conf conf_v, double f, change change_v){
		std::vector<double> tempx =std::vector<double>();
		conf_v.getV(tempx);
		std::vector<double> tempd =std::vector<double>();
		change_v.getV(tempd);
		ele* element = new ele(tempx, f, tempd);

		boost::container::stable_vector<ele>::size_type listCapacity = list.capacity();
//		boost::container::stable_vector<ele>::size_type listSize = list.size();
//		std::cout << listSize << "\t" <<listCapacity << std::endl;
		if (listCapacity <= list.size() ) {//can be < in case a new thread attempts to add before the condition
			WriteLock w_lock(myLock);
			list.reserve(listCapacity << 1);
		}

		{
			WriteLock w_lock(myLock);
			list.push_back(*element);
//			w_lock.unlock();
		}

		return true;
	}
};

class circularvisited /*: public visited*/ {
	boost::container::stable_vector<ele> list;
	int n_variable;
	int p;
	bool full;
	
public:
	inline int get_maxCheck(){
//		return ceil(1.5*n_variable);
		return 4*n_variable;
	}

	inline int get_maxSize(){
		return 5*n_variable;
	}


	circularvisited(){
//		std::cout<<"Visited Instance created"<<std::endl;
		list=boost::container::stable_vector<ele>();
		n_variable=0;
		p=0;
		full=false;
	}

	bool interesting(conf x, double f,change g);
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
