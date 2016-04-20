/*

   Copyright (c) 2006-2010, The Scripps Research Institute

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Author: Dr. Oleg Trott <ot14@columbia.edu>, 
           The Olson Lab, 
           The Scripps Research Institute

*/

#ifndef VINA_BFGS_H
#define VINA_BFGS_H

#include "matrix.h"
#include "visited.h"

typedef triangular_matrix<fl> flmat;

template<typename Change>
void minus_mat_vec_product(const flmat& m, const Change& in, Change& out) {
	sz n = m.dim();
	VINA_FOR(i, n) {
		fl sum = 0;
		VINA_FOR(j, n)
			sum += m(m.index_permissive(i, j)) * in(j);
		out(i) = -sum;
	}
}

template<typename Change>
inline fl scalar_product(const Change& a, const Change& b, sz n) {
	fl tmp = 0;
	VINA_FOR(i, n)
		tmp += a(i) * b(i);
	return tmp;
}

template<typename Change>
inline bool bfgs_update(flmat& h, const Change& p, const Change& y, const fl alpha) {
	const fl yp  = scalar_product(y, p, h.dim());
	if(alpha * yp < epsilon_fl) return false; // FIXME?
	Change minus_hy(y); minus_mat_vec_product(h, y, minus_hy);
	const fl yhy = - scalar_product(y, minus_hy, h.dim());
	const fl r = 1 / (alpha * yp); // 1 / (s^T * y) , where s = alpha * p // FIXME   ... < epsilon
	const sz n = p.num_floats();
	VINA_FOR(i, n)
		VINA_RANGE(j, i, n) // includes i
			h(i, j) +=   alpha * r * (minus_hy(i) * p(j)
	                                + minus_hy(j) * p(i)) +
			           + alpha * alpha * (r*r * yhy  + r) * p(i) * p(j); // s * s == alpha * alpha * p * p
	return true;
}

template<typename F, typename Conf, typename Change>
fl line_search(F& f, sz n, const Conf& x, const Change& g, const fl f0, const Change& p, Conf& x_new, Change& g_new, fl& f1) { // returns alpha
	const fl c0 = 0.0001;
	const unsigned max_trials = 10;
	const fl multiplier = 0.5;
	fl alpha = 1;

	const fl pg = scalar_product(p, g, n);

	VINA_U_FOR(trial, max_trials) {
		x_new = x; x_new.increment(p, alpha);//x(k+1) = x(k) + delta x (k)
		f1 = f(x_new, g_new);
		if(f1 - f0 < c0 * alpha * pg) // FIXME check - div by norm(p) ? no?
			break;
		alpha *= multiplier;
	}
	return alpha;
}

inline void set_diagonal(flmat& m, fl x) {
	VINA_FOR(i, m.dim())
		m(i, i) = x;
}

template<typename Change>
void subtract_change(Change& b, const Change& a, sz n) { // b -= a
	VINA_FOR(i, n)
		b(i) -= a(i);
}

//the calling function was
//void quasi_newton::operator()(model& m, const precalculate& p, const igrid& ig, output_type& out, change& g, const vec& v) const { // g must have correct size
//	quasi_newton_aux aux(&m, &p, &ig, v);
//	fl res = bfgs(aux, out.c, g, max_steps, average_required_improvement, 10);
//	out.e = res;
//}
//// N.B. y = individual numbers, f = function= y(x), g = delta f = first order derivative
template<typename F, typename Conf, typename Change>
fl bfgs(F& f, Conf& x, Change& g, const unsigned max_steps, const fl average_required_improvement, const sz over, output_container* history
		, circularvisited* tried
		, bool global) { // x is I/O, final value is returned

//	::print(f.v);printf("\n");
//	printf("XOUYANG %lf\n",f.v[0]);
	flv outputFlv;//by Amr
		
	sz n = g.num_floats();//
	flmat h(n, 0);
	set_diagonal(h, 1);

	Change g_new(g);
	Conf x_new(x);

//	linearvisited* buffer_allthreads = linearvisited::getInstance();
	Octree* buffer_allthreads = Octree::getInstance();

	fl f0 = f(x, g); //evaluate the derivative of conf x in change g, and returns the the function value in f0

//	printf("%f\t",f0);
//	printf("Amr\t X : "); outputFlv.clear(); x.getV(outputFlv);::print(outputFlv); printf("\n");


#if WRITE_HISTORY
	if(history)
		history->push_back(new output_type(x, f0, global ? 10 : 7));//Global or local only
#endif
	if(tried){
//		if (!(/*f.p->*/buffer_allthreads->interesting(x, f0, g, 0)||tried->interesting(x, f0, g, ret))) {

		int ret;
		if ((ret = /*f.p->*/buffer_allthreads->interesting(x, f0, g, 0)) >= 0 && tried->interesting(x, f0, g, ret) >= 0){
			return f0;//i.e. failed
		}else{
			/*f.m->*/tried->add(x, f0, g);
#if WRITE_HISTORY
			if(history && global)
				history->push_back(new output_type(x, f0, 2));//QVina accepted
#endif
		}
	}
	fl f_orig = f0;
	Change g_orig(g);
	Conf x_orig(x);
	Change p(g);//descent direction

//	flv f_values; f_values.reserve(max_steps+1);//TODO what is the use of this vector ???
//	f_values.push_back(f0);

	VINA_U_FOR(step, max_steps) {
		minus_mat_vec_product(h, g, p);//find and fill direction p
		fl f1 = 0;
		const fl alpha = line_search(f, n, x, g, f0, p, x_new, g_new, f1);//find amplitudes of change vector and update from p to f1
		Change y(g_new); subtract_change(y, g, n);//y(k) = del f(x(k+1)) - del f(x(k))

//		printf("%f\t",f1);
//		printf("Amr\t y%d\t",(step)); outputFlv.clear(); y.getV(outputFlv); ::print(outputFlv);printf("\n");

//		f_values.push_back(f1);
		f0 = f1;
		x = x_new;
		g = g_new; // ?
		if(!(std::sqrt(scalar_product(g, g, n)) >= 1e-5)) break; // breaks for nans too // FIXME !!?? 

		if(step == 0) {
			const fl yy = scalar_product(y, y, n);
			if(std::abs(yy) > epsilon_fl)
				set_diagonal(h, alpha * scalar_product(y, p, n) / yy);
  	 	}

		bool h_updated = bfgs_update(h, p, y, alpha);//updates h only
		if (tried) {
			tried->add(x, f0, g);
#if WRITE_HISTORY
		if(history && !global)
			history->push_back(new output_type(x, f0, 7));//local search only
#endif
		}
 	}
	if (!global)
		/*f.p->*/buffer_allthreads->add(x, f0, g);//TODO later we shall add in only either this or the last one of the loop

	if(!(f0 <= f_orig)) { // succeeds for nans too
		f0 = f_orig;
		x = x_orig;
		g = g_orig;
	}

//	f.m->tried.add(x,g);
//		printf("%d\n",f.m->tried.size());
 
//	printf("database size=%d\n",f.m->tried.size());
	return f0;
}

#endif
