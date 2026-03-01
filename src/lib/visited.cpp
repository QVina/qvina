#include "visited.h"

double ele::dist2(const std::vector<double>& now) const {
	double out=0;
	for (int i=0;i<size();i++){
		double d = this->x[i] - now[i];
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
bool ele::check(const std::vector<double>& now_x, double now_f, const std::vector<double>& now_d) const
{
//	int counter=0;//no need currently
	bool newXBigger, newYBigger;
	const long long ONE=1;
	long long bitMask =0;
	newYBigger=(now_f -f) > 0;
//	bool effect=false;

	int nowDSize = now_d.size();
	for (int i = 0; i < nowDSize; i++) {
		bitMask = ONE << i;

		if((d_zero & bitMask) || !(now_d[i])){//if any of them is zero
//			counter++;//no need currently
		}else{
			const bool nowPositive= now_d[i] > 0;
			const bool dPositive= d_positive & bitMask;
			if (nowPositive ^ dPositive) {//if both derivatives have different signs
//				counter++;//no need currently
			}
			else {
				newXBigger=(now_x[i]-x[i]) > 0;
				if (nowPositive? (newXBigger ^ newYBigger): (!(newXBigger ^ newYBigger))) {//if the higher x have lower f (if both ascending), or vice versa
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

bool visited::interesting(const conf& x, double f, const change& g){
	int len=size();
	if (len==0 || !full){
		return true;
	}

	tempx.clear();
	tempd.clear();
	if(n_variable > 0) {
		tempx.reserve(n_variable);
		tempd.reserve(n_variable);
	}
	x.getV(tempx);
	g.getV(tempd);

	dist_scratch.resize(len);
	not_picked_scratch.assign(len, 1);

	// fill dist[] with distances from conf
	for (int i=0;i<len;i++){
		dist_scratch[i]=this->get(i).dist2(tempx);
	}

	bool flag=false;
	double min=1e10;
	int p=0;
	const int maxCheck = 2 * n_variable;
	for (int i = 0; i < maxCheck; i++){
		min=1e10;
		for (int j=0;j<len;j++){
			if (not_picked_scratch[j] && (dist_scratch[j]<min)){
				p=j;
				min=dist_scratch[j];
			}
		}
		not_picked_scratch[p]=0;
		flag=this->get(p).check(tempx, f, tempd);
		if (flag) break;
	}
	return flag;
}
