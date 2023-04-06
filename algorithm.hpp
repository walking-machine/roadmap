#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "shape_collections.hpp"
#include <random>

class sampler {
      public:
	virtual void seed(uint seed) = 0;
	virtual float generate(std::uniform_real_distribution<float> range) = 0;
};

typedef std::unique_ptr<sampler> sampler_ptr;

template <class T> class sampler_imp : public sampler {
      protected:
	T generator;

      public:
	virtual void seed(uint seed) override { generator.seed(); }

	virtual float
	generate(std::uniform_real_distribution<float> range) override
	{
		return range(generator);
	}
};

/* Allowed number generators */
template class sampler_imp<std::mt19937>;

class algorithm {
      protected:
	system_nd *sys = NULL;
	virtual bool continue_map_internal(graph *cur_set) = 0;
	virtual graph *init_algo_internal(system_nd *new_sys)
	{
		return nullptr;
	}
	std::vector<std::uniform_real_distribution<float>> ranges;
	sampler_ptr generator;
	uint seed;

      public:
	bool continue_map(graph *cur_set);
	graph *init_algo(system_nd *new_sys);

	algorithm(sampler *sampler = new sampler_imp<std::mt19937>)
	{
		generator.reset(sampler);
	}

	virtual float get_connection_radius(system_nd *sys) = 0;
};

#endif