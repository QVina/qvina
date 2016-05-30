#include "visited.h"

double ele::dist2(std::vector<double> now){
	double out=0;
	for (int i=0; i < size(); i++){
		double d = x[i] - now[i];
		out += d * d;
	}
	return out;
}
double ele::dist2_3D(std::vector<double> now){
	double d, out=0;
	for (int i=0;i<3;i++){
		d = x[i] - now[i];
		out += d * d;
	}
	return out;
}
//static bool comp(std::pair<int, double> p, std::pair<int, double> q)
//{
//	return (p.second<q.second);
//}

/** differences:
 * 1- I check also the value of f
 * 2- point is considered interesting if only half of number of variables contains stationary points
*/
bool ele::check(std::vector<double> now_x, double now_f, std::vector<double> now_d) const {
	bool out=false;
//	int counter=0;//no need currently
	bool newXBigger, newYBigger;
	const long long ONE=0x0000000000000001;
	long long bitMask;
	newYBigger=(now_f - f) > 0;
//	bool effect=false;

	int nowDSize = now_d.size();
	for (int i = 0; i < nowDSize; i++) {
		bitMask = ONE << i;

		if((d_zero & bitMask) || !(now_d[i])){//if any of them is zero
//			counter++;//no need currently
			continue;
		}else{
			const bool nowPositive= now_d[i] > 0;
			const bool dPositive= d_positive & bitMask;
			if (nowPositive ^ dPositive) {//if both derivatives have different signs
//				counter++;//no need currently
				continue;
			}
			else {
				newXBigger=(now_x[i]-x[i]) > 0;
//				if (nowPositive? (newXBigger ^ newYBigger): (!(newXBigger ^ newYBigger))) {//if the higher x have lower f (if both ascending), or vice versa
				if (! (nowPositive ^ newXBigger ^ newYBigger)) {//if the higher x have lower f (if both ascending), or vice versa
//					counter++;//no need currently
//					effect=true;
				}
				//TODO this else is valid only in case the least accepted number is
				//ALLLLLL the variables (to be removed if we want to relax the check later on)
				else{
					return false;
				}
			}
		}
//		if (counter >= (now_d.size())) {
//			return true;
//		}
	}
	return true; //just return true, no need for check (to be removed if we want to relax the check later on)

//	out=((((d_positive^now_positive)|d_zero)|now_zero)==getMask());
////	printf("mask=%ld\td_p=%ld\td_z=%ld\tn_p=%ld\tn_z=%ld\n",getMask(),d_positive,d_zero,now_positive,now_zero);
////	printf("check=%s\n",temp?"true":"false");
////	getchar();
////	}
////	return out;

//	return false;
}

//template<class T> using min_heap = std::priority_queue<T, std::vector<T>, std::greater<T>>;

//just initialize the defaults outside the class to have any reference to them later
Vec3 Octree::defaultHalfDimension;
Vec3 Octree::defaultOrigin;
Vec3 Octree::MINIMUM_HALFDIMENSION = Vec3(0.1, 0.1, 0.1);
//Singleton design pattern
Octree* Octree::instance=NULL;
Octree* Octree::getInstance(){
	if (!instance) {
		static Octree self;
		Octree::instance = & self;
		std::cout << "lazy initialization done"<< std::endl;
	}
	return instance;
}

/**
 * returns -1 if interesting (found at least one point with accepted condition),
 * or a number >=0 indicating number of done checks otherwise (if nothing is found).
 */
int Octree::interesting(conf x, double f, change g, int excluded) {
	//n.b. excluded is not used. it is here only for homology with the other function

	std::vector<double> conf_v;
	x.getV(conf_v);
	std::vector<double> change_v;
	g.getV(change_v);
	std::vector<ele> nearbyPoints;
	std::vector<double> distances;
	Vec3 bmin(conf_v[0]-CUTOFF, conf_v[1]-CUTOFF, conf_v[2]-CUTOFF);
	Vec3 bmax(conf_v[0]+CUTOFF, conf_v[1]+CUTOFF, conf_v[2]+CUTOFF);
	getPointsWithinCutoff(CUTOFF*CUTOFF,conf_v, bmin, bmax, nearbyPoints, distances);

	int len=nearbyPoints.size();
	bool notYetChecked[len];
	memset(notYetChecked,true,sizeof(notYetChecked));

	const int grandMaxCheck= 1 * conf_v.size(); //1N in this case
	const int maxCheck= (nearbyPoints.size()<= grandMaxCheck)? nearbyPoints.size():grandMaxCheck;

	double min=1e10;
	int i=0; //counts checked done so far
	int p; //pointer to current nearest point
	for ( ; i < maxCheck; i++){
		min=1e10;
		for (int j=0;j<len;j++){
			if (notYetChecked[j] && (distances[j] <= min)){
				p=j;
				min=distances[p];
			}
		}
		notYetChecked[p]=false;

		if (nearbyPoints[p].check(conf_v, f, change_v)){
			return -1; //i.e. return success
		}
	}
	return i;
}

int circularvisited::interesting(conf x, double f, change g, int excluded){

//	printf("%d   %d\n", get_maxCheck(), get_maxSize());

	int len=size();
	if (len==0){
		return -1; //i.e. interesting
	}
	else{
//		if (len<2*n_variable)
		if (!full){
//			printf("len==%d<%d\n",len,10*n_variable);
			return -1; //i.e. interesting
		}

		std::vector<double> conf_v;
		x.getV(conf_v);
		std::vector<double> change_v;
		g.getV(change_v);
		double dist[len];
		bool notPicked[len];

		memset(notPicked,true,sizeof(notPicked));
		//fill dist[] with distances from conf
		for (int i=0;i<len;i++){
			dist[i]=this->get(i).dist2(conf_v);
		}

//		bool flag=false;
		double min=1e10;
		int p=0;
		const int maxCheck = get_maxCheck()-excluded;
		int i = 0;
		for ( ; i < maxCheck; i++){
			min=1e10;
			for (int j=0;j<len;j++){
				if (notPicked[j] && (dist[j]<min)){
					p=j;
					min=dist[j];
				}
			}
			notPicked[p]=false;
			if (this->get(p).check(conf_v, f, change_v))
				return -1; //i.e. interesting
		}
		return i;//i.e. not interesting after checking i points
	}
}
