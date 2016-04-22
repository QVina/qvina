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

//class linearvisited /*: public visited*/{
//public:
//private:
//	ReadWriteLock myLock;
//	boost::container::stable_vector<ele> list;
//	linearvisited(){
//		list=boost::container::stable_vector<ele>();
//	};
//	static linearvisited* instance;
//	linearvisited(const linearvisited& a);
//	linearvisited(linearvisited &a);
//	const linearvisited& operator=(const linearvisited& a);
//
//public:
//	static linearvisited* getInstance();
//
//	int interesting(conf x, double f,change g, int excluded) ;
//
//	inline bool add(conf conf_v, double f, change change_v){
//		std::vector<double> tempx =std::vector<double>();
//		conf_v.getV(tempx);
//		std::vector<double> tempd =std::vector<double>();
//		change_v.getV(tempd);
//		ele* element = new ele(tempx, f, tempd);
//
//		boost::container::stable_vector<ele>::size_type listCapacity = list.capacity();
////		boost::container::stable_vector<ele>::size_type listSize = list.size();
////		std::cout << listSize << "\t" <<listCapacity << std::endl;
//		if (listCapacity <= list.size() ) {//can be < in case a new thread attempts to add before the condition
//			WriteLock w_lock(myLock);
//			list.reserve(listCapacity << 1);
//		}
//
//		{
//			WriteLock w_lock(myLock);
//			list.push_back(*element);
////			w_lock.unlock();
//		}
//
//		return true;
//	}
//};

struct Vec3 {
	union {
		struct {
			float x,y,z;
		};
		float D[3];
	};

	Vec3() { }
	Vec3(float _x, float _y, float _z):x(_x), y(_y), z(_z){ }
	Vec3 operator+(const Vec3& r) const {
		return Vec3(x+r.x, y+r.y, z+r.z);
	}
	Vec3 operator-(const Vec3& r) const {
		return Vec3(x-r.x, y-r.y, z-r.z);
	}
	Vec3 operator+(float f) const {
		return Vec3(x+f, y+f, z+f);
	}
	Vec3 operator-(float f) const {
			return Vec3(x-f, y-f, z-f);
	}

	Vec3 operator*(float r) const {
		return Vec3(x*r,y*r,z*r);
	}
	bool operator<(const Vec3& o){
		return x<o.x && y< o.y && z<o.z;
	}
};

class Octree {
	constexpr static float CUTOFF = 5.0f;
	static Vec3 MINIMUM_HALFDIMENSION;// = new const Vec3(0.01,1.01,0.01);
	const static int MAX_FRUITS=8;
	static Octree* instance;
	static Vec3 defaultOrigin;
	static Vec3 defaultHalfDimension;
	ReadWriteLock lock;


	// Physical position/size. This implicitly defines the bounding
	// box of this node
	Vec3 origin;         //! The physical center of this node
	Vec3 halfDimension;  //! Half the width/height/depth of this node

	// The tree has up to eight children and can additionally store
	// a point, though in many applications only, the leaves will store data.
	Octree* children[8]; //! Pointers to child octants
	bool internal=false;
	std::vector<ele> *data=NULL;

	/*
				Children follow a predictable pattern to make accesses simple.
				Here, - means less than 'origin' in that dimension, + means greater than.
				child:	0 1 2 3 4 5 6 7
				z:      - - - - + + + +
				y:      - - + + - - + +
				x:      - + - + - + - +
	 */

public:

	static Octree* getInstance();
	int interesting(conf x, double f,change g, int excluded) ;

	inline bool add(conf conf_v, double f, change change_v){
		std::vector<double> tempx =std::vector<double>();
		conf_v.getV(tempx);
		std::vector<double> tempd =std::vector<double>();
		change_v.getV(tempd);
		ele* element = new ele(tempx, f, tempd);
		return insert(element);
	}


	Octree(): origin(defaultOrigin), halfDimension(defaultHalfDimension), data(NULL) {}
	Octree(const Vec3& origin, const Vec3& halfDimension): origin(origin), halfDimension(halfDimension), data(NULL) {
		// Initially, there are no children
		for(int i=0; i<8; ++i)
			children[i] = NULL;
	}
//	Octree(const Octree& copy): origin(copy.origin), halfDimension(copy.halfDimension), data(copy.data) {}

	~Octree() {
		// Recursively destroy octants
		for(int i=0; i<8; ++i)
			delete children[i];
		if(data)
			delete data;
	}

	// Determine which octant of the tree would contain 'point'
	int getOctantContainingPoint(std::vector<double>& point){
		int oct = 0;
		if(point[0] >= origin.x) oct |= 1;
		if(point[1] >= origin.y) oct |= 2;
		if(point[2] >= origin.z) oct |= 4;
		return oct;
	}

//	bool isLeafNode() const {
//		return !internal;
//	}
	bool isInternalNode() const {
		return internal;
	}

	bool insert(ele* point) {
		if(isInternalNode()) {
			// We are at an interior node. Insert recursively into the
			// appropriate child octant
			int octant = getOctantContainingPoint(point->x);
			children[octant]->insert(point);
			return true;
		} else {
			//if no vector initialize one
			if(!data){
				data=new std::vector<ele>();
				data->reserve(MAX_FRUITS);
			}

			std::vector<ele>& data= *this->data;
			// if size less than maximum points, just add the element
			//also if the cell size became too small to divide, override the maximum content condition and just add.
			if(data.size()<MAX_FRUITS){
				WriteLock w_lock(lock);
				data.push_back(*point);
				return true;
			} else {
				//else split and add the old data that was here, along with
				// this new data point

				// Split the current node and create new empty trees for each
				// child octant.
				double lowX = origin.x - halfDimension.x / 2.0;
				double highX= origin.x + halfDimension.x / 2.0;
				double lowY = origin.y - halfDimension.y / 2.0;
				double highY= origin.y + halfDimension.y / 2.0;
				double lowZ = origin.z - halfDimension.z / 2.0;
				double highZ= origin.z + halfDimension.z / 2.0;
//				std::cout<< (halfDimension.x /2.0) << std::endl;
				for(int i=0; i<8; ++i) {
					// Compute new bounding box for this child
					Vec3 newOrigin;
					newOrigin.x = i&1 ? highX : lowX;
					newOrigin.y = i&2 ? highY : lowY;
					newOrigin.z = i&4 ? highZ : lowZ;
					children[i] = new Octree(newOrigin, (halfDimension * 0.5));
				}

				WriteLock w_lock(lock);
				// Re-insert the old point, and insert this new point
				// (We wouldn't need to insert from the root, because we already
				// know it's guaranteed to be in this section of the tree)
				for (int i= 0; i < data.size() ; ++i) {
					ele oldPoint = data[i];
					children[getOctantContainingPoint(oldPoint.x)]->insert(&oldPoint);
				}
				children[getOctantContainingPoint(point->x)]->insert(point);
				//raise the internal node flag
				this->internal=true;
				return true;
			}
		}
		return false; // in case something went wrong
	}

	// This is a really simple routine for querying the tree for points
	// within a bounding box defined by min/max points (bmin, bmax)
	// All results are pushed into 'results'
	void getPointsWithinCutoff(float cutoff2,std::vector<double> point, const Vec3& bmin, const Vec3& bmax, std::vector<ele>& results, std::vector<double>& distances) {
		// If we're at a leaf node, just see if the current data point is inside
		// the query bounding box
		if(isInternalNode()) {
			// We're at an interior node of the tree. We will check to see if
			// the query bounding box lies outside the octants of this node.
			for(int i=0; i<8; ++i) {
				// Compute the min/max corners of this child octant
				Vec3 cmax = children[i]->origin + children[i]->halfDimension;
				Vec3 cmin = children[i]->origin - children[i]->halfDimension;

				// If the query rectangle is outside the child's bounding box, then continue
				if(cmax.x<bmin.x || cmax.y<bmin.y || cmax.z<bmin.z) continue;
				if(cmin.x>bmax.x || cmin.y>bmax.y || cmin.z>bmax.z) continue;

				// At this point, we've determined that this child is intersecting the query bounding box
				children[i]->getPointsWithinCutoff(cutoff2, point, bmin,bmax,results, distances);
			}
		}else {
			if(data) {
				ReadLock r_lock(lock);
				std::vector<ele>& data = *this->data;
				for (int i= 0; i < data.size(); ++i) {
					const std::vector<double>& p = data[i].x;
					if(p[0]>bmax.x || p[1]>bmax.y || p[2]>bmax.z) continue;
					if(p[0]<bmin.x || p[1]<bmin.y || p[2]<bmin.z) continue;
					double dist2 = data[i].dist2_3D(point);
					if(dist2<=cutoff2){
						results.push_back(data[i]);
						distances.push_back(dist2);
					}
				}
			}
		}
	}

	static void setDefaultOrigin(Vec3 defaultOrigin) {
		Octree::defaultOrigin = defaultOrigin;
	}

	static void setDefaultHalfDimension(Vec3 defaultHalfDimension) {
		Octree::defaultHalfDimension = defaultHalfDimension;
	}
};



class circularvisited /*: public visited*/ {
	boost::container::stable_vector<ele> list;
	int n_variable;
	int p;
	bool full;
	
public:
	inline int get_maxCheck(){
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

	int interesting(conf x, double f,change g, int excluded) ;

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
