/*
 *  Cypress -- C++ Spiking Neural Network Simulation Framework
 *  Copyright (C) 2016  Andreas Stöckel
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

#include <csignal>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

#include <unistd.h>

#include <cypress/backend/nest/nest.hpp>
#include <cypress/core/neurons.hpp>
#include <cypress/util/filesystem.hpp>
#include <cypress/util/process.hpp>

namespace cypress {
namespace {
/**
 * Static class used to lookup information about the NEST simulator.
 */
class NESTUtil {
private:
	std::mutex util_mutex;

	bool m_has_info = false;
	bool m_installed = false;
	std::string m_version;

	void get_nest_info()
	{
		// This is a global resource
		std::lock_guard<std::mutex> lock(util_mutex);

		// Abort if the information has already been read
		if (m_has_info) {
			return;
		}

		// Fetch the version from NEST
		auto res = Process::exec(
		    "sh", {"-c", "nest -v | grep -o 'NEST version [0-9.]*'"});
		m_has_info = true;
		m_installed = std::get<0>(res) == 0;

		// Make sure the version string starts with "NEST version " and there is
		// at least one character left containing the version
		const std::string s = std::get<1>(res);
		if (s.size() < 14 || s.substr(0, 13) != "NEST version ") {
			m_installed = false;
			return;
		}
		m_version = s.substr(13, s.size() - 13);
	}

public:
	bool installed()
	{
		get_nest_info();
		return m_installed;
	}

	std::string version()
	{
		get_nest_info();
		return m_version;
	}
};

/**
 * Static instance of NESTUtil which caches information about the available NEST
 * version.
 */
static NESTUtil NEST_UTIL;
}  // namespace

NEST::NEST(const Json &setup)
{
	if (setup.count("timestep") > 0) {
		m_params.timestep = setup["timestep"];
	}
	if (setup.count("record_interval") > 0) {
		m_params.record_interval = setup["record_interval"];
	}
	
	if (setup.count("threads") > 0) {
		m_params.threads = setup["threads"];
	}
}

void NEST::do_run(NetworkBase &source, Real duration) const
{
	if (!installed()) {
		throw NESTSimulatorNotFound(
		    "The NEST simulator is not installed on your system or has an "
		    "incompatible version!");
	}

	// Start the NEST child process
	Process proc("nest", {"--verbosity=DEBUG", "-"});

	// Serialise the network into it and start the simulation
	std::signal(SIGPIPE, SIG_IGN);
	std::thread thread_input(sli::write_network, std::ref(proc.child_stdin()),
	                         std::ref(source), duration, std::ref(m_params));

	// Read the network response and messages
	std::thread thread_output(sli::read_response, std::ref(proc.child_stdout()),
	                          std::ref(source));

	thread_input.join();
	proc.close_child_stdin();
	thread_output.join();

	// Wait for the subprocess to exit
	if (proc.wait() != 0) {
		throw ExecutionError(
		    std::string("Error while executing the NEST simulation"));
	}
}

std::unordered_set<const NeuronType *> NEST::supported_neuron_types() const
{
	return std::unordered_set<const NeuronType *>{&SpikeSourceArray::inst(),
	                                              &IfCondExp::inst(),
	                                              &EifCondExpIsfaIsta::inst(),
                                                  &IfCurrExp::inst()};
}

bool NEST::installed() { return NEST_UTIL.installed(); }

std::string NEST::version() { return NEST_UTIL.version(); }
}  // namespace cypress
