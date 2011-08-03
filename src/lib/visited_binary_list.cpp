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

bool ele::check(std::vector<double> now)
{
	bool out=true;
	
	long now_zero=0;
	long now_positive=0;
/*
	for (int i=0;i<now.size();i++)
	{
		if ( (now[i]>0&&d[i]>0) || (now[i]<0&&d[i]<0) )
		{
			out=false;
			break;
		}
	}
	
	if (out)
	{
	::print(d);
	printf("\n");
	::print(now);				
	printf("\n");
*/	for (int i=0;i<now.size();i++)
	{
		now_zero=now_zero<<1;
		now_positive=now_positive<<1;

		if (now[i]==0) now_zero+=1;
		else if (now[i]>0) now_positive+=1;
	}
	out=((((d_positive^now_positive)|d_zero)|now_zero)==getMask());
//	printf("mask=%ld\td_p=%ld\td_z=%ld\tn_p=%ld\tn_z=%ld\n",getMask(),d_positive,d_zero,now_positive,now_zero);
//	printf("check=%s\n",temp?"true":"false");
//	getchar();
//	}
	
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
				std::vector<double> conf_v;
				x.getV(conf_v);
				std::vector<double> change_v;
				g.getV(change_v);
				double dist[len];
				bool pick[len];
				
				memset(pick,false,sizeof(pick));
				int i=0;
				for (std::list<double>:: it=list.begin();it!=list.end();it++)
 		 		{
					dist[i++]=it->dist2(conf_v);
				} 
//				sort(dist.begin(),dist.end(),comp);
//				
				bool flag=false;
				double min=1e10;
				int p=0;
				for (int i=0;i<2*n_variable;i++)
				{ 
				   	min=1e10;
					for (int j=0;j<len;j++)
					{
						if ((!pick[j])&&(dist[j]<min))
						{
							p=j;
							min=dist[j];
				 		}
				 	}
					pick[p]=true;					
					flag=this->get(p).check(change_v);
					if (flag) break;
 				} 
				
				return flag;
			}
		}
		return true;	
	}

