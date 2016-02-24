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

#include <iostream>
#include <fstream>

#include <cypress/cypress.hpp>

using namespace cypress;

int main(int argc, const char *argv[])
{
	if (argc != 2 && !NMPI::check_args(argc, argv)) {
		std::cout << "Usage: " << argv[0] << " <SIMULATOR>" << std::endl;
		return 1;
	}

	Network()
	    .population<SpikeSourceArray>("source", 1, {100.0, 200.0, 300.0},
	                                  SpikeSourceArraySignals().record_spikes())
	    .population<IfCondExp>("neuron", 4, IfCondExpParameters().v_rest(-60.0),
	                           IfCondExpSignals().record_spikes())
	    .connect("source", "neuron", Connector::all_to_all(0.16))
	    .run(PyNN(argv[1]));

	return 0;
}
