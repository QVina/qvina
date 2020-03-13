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

//#include <boost/date_time/posix_time/posix_time.hpp> // for time in microseconds
#include <algorithm> // fill, etc

#if 0 // use binary cache
	// for some reason, binary archive gives four huge warnings in VC2008
	#include <boost/archive/binary_oarchive.hpp>
	#include <boost/archive/binary_iarchive.hpp>
	typedef boost::archive::binary_iarchive iarchive;
	typedef boost::archive::binary_oarchive oarchive;
#else // use text cache
	#include <boost/archive/text_oarchive.hpp>
	#include <boost/archive/text_iarchive.hpp>
	typedef boost::archive::text_iarchive iarchive;
	typedef boost::archive::text_oarchive oarchive;
#endif 

#include <boost/serialization/split_member.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/static_assert.hpp>
#include "cache.h"
#include "file.h"
#include "szv_grid.h"

cache::cache(const std::string& scoring_function_version_, const grid_dims& gd_, fl slope_, atom_type::t atom_typing_used_) 
: scoring_function_version(scoring_function_version_), gd(gd_), slope(slope_), atu(atom_typing_used_), grids(num_atom_types(atom_typing_used_)) {}

fl cache::eval      (const model& m, fl v) const { // needs m.coords
	fl e = 0;
	sz nat = num_atom_types(atu);

	VINA_FOR(i, m.num_movable_atoms()) {
		const atom& a = m.atoms[i];
		sz t = a.get(atu);
		if(t >= nat) continue;
		const grid& g = grids[t];
		assert(g.initialized());
		e += g.evaluate(m.coords[i], slope, v);
	}
	return e;
}

fl cache::eval_deriv(      model& m, fl v) const { // needs m.coords, sets m.minus_forces
	fl e = 0;
	sz nat = num_atom_types(atu);

	VINA_FOR(i, m.num_movable_atoms()) {
		const atom& a = m.atoms[i];
		sz t = a.get(atu);
		if(t >= nat) { m.minus_forces[i].assign(0); continue; }
		const grid& g = grids[t];
		assert(g.initialized());
		vec deriv;
		e += g.evaluate(m.coords[i], slope, v, deriv);
		m.minus_forces[i] = deriv;
	}
	return e;
}

#if 0 // No longer doing I/O of the cache
void cache::read(const path& p) {
	ifile in(p, std::ios::binary);
	iarchive ar(in);
	ar >> *this;
}

void cache::write(const path& p) const {
	ofile out(p, std::ios::binary);
	oarchive ar(out);
	ar << *this;
}
#endif

template<class Archive>
void cache::save(Archive& ar, const unsigned version) const {
	ar & scoring_function_version;
	ar & gd;
	ar & atu;
	ar & grids;
}

template<class Archive>
void cache::load(Archive& ar, const unsigned version) {
	std::string name_tmp;       ar & name_tmp;       if(name_tmp != scoring_function_version) throw energy_mismatch();
	grid_dims   gd_tmp;         ar &   gd_tmp;       if(!eq(gd_tmp, gd))                    throw grid_dims_mismatch();
	atom_type::t atu_tmp;       ar &  atu_tmp;       if(atu_tmp != atu)                       throw cache_mismatch();

	ar & grids;
}

void cache::populateparalell(const model& m, const precalculate& p, const szv& atom_types_needed, bool display_progress, int noOfCpus) {
using namespace boost::posix_time;
//	ptime time_start1(microsec_clock::local_time());
	szv needed;
	VINA_FOR_IN(i, atom_types_needed) {
		sz t = atom_types_needed[i];
		if(!grids[t].initialized()) {
			needed.push_back(t);
			grids[t].init(gd);
		}
	}
//	ptime time_end1(microsec_clock::local_time());
//	time_duration duration1(time_end1 - time_start1);
//	std::cout<< "first parallel part time: " << (duration1.total_milliseconds()/1000.0)<<std::endl;

	if(needed.empty())
		return;

	grid& g = grids[needed.front()];

	boost::thread_group threadGroup;
	sz total = g.m_data.dim0() * g.m_data.dim1() * g.m_data.dim2();
	int chunkSize = total / noOfCpus;
//	std::cout << "CPUs = "<< noOfCpus << ", chunk size=" << chunkSize << std::endl;
//	std::cout << "starting the 2nd parallel part..."<<std::endl;
//	ptime time_start2(microsec_clock::local_time());

	//do the parallel population part
	for (int threadId = 0; threadId < noOfCpus; ++threadId) {
		sz start=threadId*chunkSize;
		if (threadId==noOfCpus-1 && total%noOfCpus != 0) {
			chunkSize= total % noOfCpus;
		}
		boost::thread* th = new boost::thread(&cache::populateChunk, this,
				threadId, m, needed, p, g, start, start + chunkSize);
		threadGroup.add_thread(th);
	}
	threadGroup.join_all();
//	ptime time_end2(microsec_clock::local_time());
//	time_duration duration2(time_end2 - time_start2);
//	std::cout << "finished the 2nd parallel part. time= "<<(duration2.total_milliseconds()/1000.0)<<std::endl;
}

void cache::populateChunk(int threadId, const model& m, const szv& needed, const precalculate& p, grid& g, sz start, sz end) {
	std::cout<<"function called with threadId "<<threadId << std::endl;
	flv affinities(needed.size());
	sz nat = num_atom_types(atu);
	const fl cutoff_sqr = p.cutoff_sqr();
	grid_dims gd_reduced = szv_grid_dims(gd);
	szv_grid ig(m, gd_reduced, cutoff_sqr);


	sz lineLength=g.m_data.dim0();
	sz linesCount=g.m_data.dim1();
	sz area=lineLength*linesCount;
//	sz levels=g.m_data.dim2();
//	sz total=area*levels;

	sz x, y, z, linearI;

	for (linearI = start; linearI < end; ++linearI) {
		x = (linearI % area) % lineLength;
		y = (linearI % area) / lineLength;
		z = (linearI / area);

		std::fill(affinities.begin(), affinities.end(), 0);
		vec probe_coords;
		probe_coords = g.index_to_argument(x, y, z);
		const szv& possibilities = ig.possibilities(probe_coords);
		VINA_FOR_IN(possibilities_i, possibilities){
			const sz i = possibilities[possibilities_i];
			const atom& a = m.grid_atoms[i];
			const sz t1 = a.get(atu);
			if(t1 >= nat) continue;
			const fl r2 = vec_distance_sqr(a.coords, probe_coords);
			if(r2 <= cutoff_sqr) {
				VINA_FOR_IN(j, needed) {
					const sz t2 = needed[j];
					assert(t2 < nat);
					const sz type_pair_index = triangular_matrix_index_permissive(num_atom_types(atu), t1, t2);
					affinities[j] += p.eval_fast(type_pair_index, r2);
				}
			}
		}
		VINA_FOR_IN(j, needed){
			sz t = needed[j];
			assert(t < nat);
			grids[t].m_data(x, y, z) = affinities[j];
		}

	}
}

void cache::populate(const model& m, const precalculate& p, const szv& atom_types_needed, bool display_progress) {
using namespace boost::posix_time;
//	ptime time_start1(microsec_clock::local_time());
	szv needed;
	VINA_FOR_IN(i, atom_types_needed) {
		sz t = atom_types_needed[i];
		if(!grids[t].initialized()) {
			needed.push_back(t);
			grids[t].init(gd);
		}
	}

//	ptime time_end1(microsec_clock::local_time());
//	time_duration duration1(time_end1 - time_start1);
//	std::cout<< "first parallel part time: " << (duration1.total_milliseconds()/1000.0)<<std::endl;

	if(needed.empty())
		return;
	flv affinities(needed.size());

	sz nat = num_atom_types(atu);

	grid& g = grids[needed.front()];

	const fl cutoff_sqr = p.cutoff_sqr();

	grid_dims gd_reduced = szv_grid_dims(gd);
	szv_grid ig(m, gd_reduced, cutoff_sqr);

//	std::cout << "starting the 2nd parallel part..."<<std::endl;
//	ptime time_start2(microsec_clock::local_time());

	VINA_FOR(x, g.m_data.dim0()) {
		VINA_FOR(y, g.m_data.dim1()) {
			VINA_FOR(z, g.m_data.dim2()) {
				std::fill(affinities.begin(), affinities.end(), 0);
				vec probe_coords; probe_coords = g.index_to_argument(x, y, z);
				const szv& possibilities = ig.possibilities(probe_coords);
				VINA_FOR_IN(possibilities_i, possibilities) {
					const sz i = possibilities[possibilities_i];
					const atom& a = m.grid_atoms[i];
					const sz t1 = a.get(atu);
					if(t1 >= nat) continue;
					const fl r2 = vec_distance_sqr(a.coords, probe_coords);
					if(r2 <= cutoff_sqr) {
						VINA_FOR_IN(j, needed) {
							const sz t2 = needed[j];
							assert(t2 < nat);
							const sz type_pair_index = triangular_matrix_index_permissive(num_atom_types(atu), t1, t2);
							affinities[j] += p.eval_fast(type_pair_index, r2);
						}
					}
				}
				VINA_FOR_IN(j, needed) {
					sz t = needed[j];
					assert(t < nat);
					grids[t].m_data(x, y, z) = affinities[j];
				}
			}
		}
	}
//	ptime time_end2(microsec_clock::local_time());
//	time_duration duration2(time_end2 - time_start2);
//	std::cout << "finished the 2nd parallel part. time= "<<(duration2.total_milliseconds()/1000.0)<<std::endl;
}
