/*
 *  Cypress -- C++ Spiking Neural Network Simulation Framework
 *  Copyright (C) 2019  Christoph Ostrau
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @fíle genn.hpp
 *
 * This is the backend implementation for GeNN, allowing simulations on GPUs.
 *
 * @author Christoph Ostrau
 */

#pragma once

#include <cypress/core/backend.hpp>
#include <cypress/util/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace cypress {
// Forward declaration
struct network_storage;

class GeNN : public Backend {
private:
	void do_run(NetworkBase &network, Real duration) const override;

	double m_timestep = 0.1;
	bool m_gpu = false;
	bool m_double = false;
	bool m_timing = false;
	bool m_keep_compile = false;
	bool m_disable_status = false;
	std::shared_ptr<network_storage> m_storage;
	size_t m_recording_buffer_size = 10000;

public:
	/**
	 * Exception thrown if the given GeNN backend is not found.
	 */
	class GeNNSimulatorNotFound : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	/**
	 * Constructor of the GeNN backend class, takes a setup JSON
	 * object.
	 *
	 * @param setup is a JSON object which supports the following settings:
	 *
	 * {
	 *      "timestep" : 0.1,
	 *      "gpu" : false,
	 *      "double" : false,
	 *      "timing" : false,
	 *      "keep_compile": false,
     *      "recording_buffer_size" : 10000
	 * }
	 */
	GeNN(const Json &setup = Json());

	/**
	 * Destructor of the GeNN backend class.
	 */
	~GeNN() override = default;

	/**
	 * Returns the neuron types supported by the NEST backend.
	 */
	std::unordered_set<const NeuronType *> supported_neuron_types()
	    const override;

	/**
	 * Returns the canonical name of the backend.
	 */
	std::string name() const override { return "genn"; }

	// void load_pop(const NeuronGroup& neuron);
};
}  // namespace cypress

extern "C" {
/**
 * @brief Expose constructor for program dynamically loading this lib
 *
 * @return pointer to GeNN object
 */
cypress::Backend *make_genn_backend(const cypress::Json &setup)
{
	return new cypress::GeNN(setup);
}
}
