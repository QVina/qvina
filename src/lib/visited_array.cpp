#include "visited.h"

	double ele::dist2(std::vector<double> now)
	{
		double out=0;
		for (int i=0;i<size();i++)
		{ 
			out+=(this->x[i]-now[i])*(this->x[i]-now[i]);
		}
		return out;
	}
static bool comp(std::pair<int, double> p, std::pair<int, double> q)
{
	return (p.second<q.second);
}
/*
bool ele::check(std::vector<double> now)
{
	int same=0;
	for (int i=0;i<now.size();i++)
	{
//		if (now[i]*d[i]>0)
		if ((now[i]>0&&d[i]>0) || (now[i]<0&&d[i]<0))
		{
			same++;
		}
	}
	if (same<=(now.size()/2)) return true;
	else false;
}
*/

bool ele::check(std::vector<double> now)
{
	bool out=true;
	for (int i=0;i<now.size();i++)
	{
		if ( (now[i]>0&&d[i]>0) || (now[i]<0&&d[i]<0) )
		{
			out=false;
			break;
		}
	}
	return out;
}

bool visited::interesting(conf x, change g)
	{
		int len=size();
		if (len==0)
		{
			return true;
		}
		else
		{
//			if (len<2*n_variable)
			if (len<10*n_variable)
		 	{ 
//				printf("len==%d<%d\n",len,10*n_variable);
				return true;
			}
			else
		 	{
				std::vector < std::pair <int, double> > dist;
				std::pair<int, double> temp;
				std::vector<double> conf_v;
				x.getV(conf_v);
				std::vector<double> change_v;
				g.getV(change_v);

//				printf("check:\t");
//				for (int i=0;i<conf_v.size();i++)
//					printf("%.1lf ",change_v[i]);
//				printf("\n");

				for (int i=0;i<len;i++)
 		 		{
					temp=std::pair<int, double>(i,this->get(i).dist2(conf_v));
					dist.push_back( temp);
				} 
				sort(dist.begin(),dist.end(),comp);
		//		printf("printing sorted dist\n");
		//		for (int i=0;i<dist.size();i++)
		//			printf("%d->%lf\n",dist[i].first,dist[i].second);
		//		printf("\n");
				bool flag=false;
//				for (int i=0;i<2*n_variable;i++)
				for (int i=0;i<2*n_variable;i++)
				{ 
					flag=this->get(dist[i].first).check(change_v);
					if (flag)
					{
//						std::vector<double> now(get(dist[i].first).d);
//						printf("hit:\t");
//						for (int j=0;j<n_variable;j++)
//							printf("%.1lf ",now[j]);
				//		printf("yeah\n");
						break;
					}
					else
					{
//						printf("no\n");
					}
 				}
				/*
				if (!flag)
				{
					printf("fail\n");
				}
				*/
				
				return flag;
			}
		}
		return true;	
	}

