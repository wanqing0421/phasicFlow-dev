
/*------------------------------- phasicFlow ---------------------------------
      O        C enter of
     O O       E ngineering and
    O   O      M ultiscale modeling of
   OOOOOOO     F luid flow       
------------------------------------------------------------------------------
  Copyright (C): www.cemf.ir
  email: hamid.r.norouzi AT gmail.com
------------------------------------------------------------------------------  
Licence:
  This file is part of phasicFlow code. It is a free software for simulating 
  granular and multiphase flows. You can redistribute it and/or modify it under
  the terms of GNU General Public License v3 or any other later versions. 

  phasicFlow is distributed to help others in their research in the field of 
  granular and multiphase flows, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

-----------------------------------------------------------------------------*/
/** 
 * \file sphereGranFlowWithMonitoring.cpp
 * \brief sphereGranFlow solver with monitoring box functionality
 *
 * This solver simulates granular flow of cohesion-less, spherical particles 
 * with particle insertion mechanism. It includes a monitoring box feature that
 * counts particles in a specified rectangular region before each time step.
 * If no particles are found in the monitoring box, the simulation stops.
 *
 * The monitoring box can be configured through the settings dictionary or
 * command line parameters.
 */

#include "vocabs.hpp"
#include "phasicFlowKokkos.hpp"
#include "systemControl.hpp"
#include "commandLine.hpp"
#include "property.hpp"
#include "geometry.hpp"
#include "sphereParticles.hpp"
#include "interaction.hpp"
#include "Insertions.hpp"
#include "box.hpp"
#include <fstream>

/**
 * DEM solver for simulating granular flow of cohesion-less particles with
 * monitoring box functionality. Stops simulation when no particles are
 * detected in the specified monitoring box.
 *
 * In the root case directory just simply enter the following command to 
 * run the simulation. For command line options use flag -h. 
 */
int main( int argc, char* argv[])
{

pFlow::commandLine cmds(
		"sphereGranFlowWithMonitoring",
		"DEM solver for non-cohesive spherical particles with monitoring box "
		"that stops simulation when no particles are detected in the box");

bool isCoupling = false;

if(!cmds.parse(argc, argv)) return 0;

// this should be palced in each main 
pFlow::processors::initProcessors(argc, argv);
pFlow::initialize_pFlowProcessors();
#include "initialize_Control.hpp"
	 
	#include "setProperty.hpp"
	#include "setSurfaceGeometry.hpp"

	#include "createDEMComponents.hpp"
	
	// Define the monitoring box region
	// Option 1: Read from settings dictionary (if defined)
	// Option 2: Use default values
	pFlow::box monitoringBox;
	bool useMonitoringBox = false;
	
	// Try to read monitoring box from settings dictionary
	const auto& settingsDict = Control.settingsDict();
	if (settingsDict.containsDictionay("monitoringBox")) {
		try {
			const auto& boxDict = settingsDict.subDict("monitoringBox");
			monitoringBox = pFlow::box(boxDict);
			useMonitoringBox = true;
			REPORT(0)<<"Reading monitoring box from settingsDict"<<END_REPORT;
		} catch (...) {
			REPORT(0)<<"Warning: Failed to read monitoringBox from settingsDict, using default"<<END_REPORT;
		}
	}
	
	// If not read from dictionary, use default box or check command line
	if (!useMonitoringBox) {
		// Default monitoring box (modify these coordinates as needed)
		monitoringBox = pFlow::box(
			pFlow::realx3(-0.1, -0.1, -0.1),  // min point
			pFlow::realx3( 0.1,  0.1,  0.1)   // max point
		);
		useMonitoringBox = true;  // Enable monitoring with default box
		REPORT(0)<<"Using default monitoring box"<<END_REPORT;
	}
	
	if (useMonitoringBox) {
		REPORT(0)<<"Monitoring box defined: min="<<monitoringBox.minPoint()
			<<", max="<<monitoringBox.maxPoint()<<END_REPORT;
	}
	
	REPORT(0)<<"\nStart of time loop . . .\n"<<END_REPORT;

	// Variable to track stop reason
	pFlow::word stopReason = "Normal completion";
	pFlow::real stopTime = 0.0;
	pFlow::uint32 stopIteration = 0;

	do 
	{
		// Count particles in the monitoring box before iteration
		if (useMonitoringBox) {
			// Access pointPosition through the base class interface
			const auto& positions = static_cast<const pFlow::particles&>(sphParticles).pointPosition();
			pFlow::uint32 totalParticles = 0;
			pFlow::uint32 particlesInBox = 0;
			
			// Get number of active particles
			pFlow::uint32 numActive = sphParticles.numActive();
			totalParticles = numActive;
			
			// Get device view of positions for Kokkos access
			auto positionsView = positions.deviceViewAll();
			
			// Debug: print size info
			if (Control.time().currentIter() == 0) {
				REPORT(0)<<"Debug: numActive="<<numActive 
					<<", size="<<sphParticles.pStruct().size()
					<<", capacity="<<sphParticles.pStruct().capacity()
					<<END_REPORT;
			}
			
			// Count particles inside the box using Kokkos parallel reduction
			// Note: We iterate through active points range
			using policy = Kokkos::RangePolicy<
				Kokkos::DefaultExecutionSpace,
				Kokkos::IndexType<pFlow::uint32>>;
			
			// Get the active range from point structure
			const auto& pStruct = sphParticles.pStruct();
			auto activeRange = pStruct.activeRange();
			
			Kokkos::parallel_reduce("countParticlesInBox",
				policy(activeRange.start(), activeRange.end()),
				LAMBDA_HD(pFlow::uint32 i, pFlow::uint32& count) {
					if (monitoringBox.isInside(positionsView(i))) {
						count++;
					}
				},
				particlesInBox);
			
			// Output particle count information
			// Use timersReportInterval from settingsDict for consistent reporting
			if (Control.time().timersReportTime() || particlesInBox == 0) {
				REPORT(0)<<"Time = "<<Control.time().currentTime()
					<<", Iteration = "<<Control.time().currentIter()
					<<", Total particles = "<<totalParticles
					<<", Particles in monitoring box = "<<particlesInBox
					<<END_REPORT;
			}
			
			// Stop simulation if no particles in the monitoring box
			if (particlesInBox == 0 && totalParticles > 0) {
				stopReason = "No particles in monitoring box";
				stopTime = Control.time().currentTime();
				stopIteration = Control.time().currentIter();
				REPORT(0)<<"\nNo particles in monitoring box. Stopping simulation."<<END_REPORT;
				break;
			}
			
			// Also stop if there are no particles at all
			if (totalParticles == 0) {
				stopReason = "No particles in simulation";
				stopTime = Control.time().currentTime();
				stopIteration = Control.time().currentIter();
				REPORT(0)<<"\nNo particles in simulation. Stopping."<<END_REPORT;
				break;
			}
		}
		
		//Ping;
		if(! sphInsertion.insertParticles( 
			Control.time().currentIter(),
			Control.time().currentTime(),
			Control.time().dt()	) )
		{
			fatalError<<
			"particle insertion failed in sphereDFlow solver.\n";
			return 1;
		}
		
		// set force to zero
		surfGeometry.beforeIteration();

		// set force to zero, predict, particle deletion and etc. 
		sphParticles.beforeIteration();
		//Ping;
		sphInteraction.beforeIteration();
		
		sphInteraction.iterate();
			
		surfGeometry.iterate();

		//Ping;
		sphParticles.iterate();
		
		//Ping;
		sphInteraction.afterIteration();

		//Ping;
		surfGeometry.afterIteration();

		//Ping;
		sphParticles.afterIteration();
		//Ping;	

	}while(Control++);

	// Record final time if stopped by monitoring condition
	if (stopReason != "Normal completion") {
		stopTime = Control.time().currentTime();
		stopIteration = Control.time().currentIter();
	}
    else
    {
        stopReason = "Normal completion";
        stopTime = Control.time().currentTime();
		stopIteration = Control.time().currentIter();
    }

	REPORT(0)<<"\nEnd of time loop.\n"<<END_REPORT;

	// Write stop information to file
	{
		std::ofstream outFile("stopInfo.txt", std::ios::out);
		if (outFile.is_open()) {
			outFile << "# Simulation Stop Information" << std::endl;
			outFile << "# Generated by sphereGranFlowWithMonitoring" << std::endl;
			outFile << "StopReason: " << stopReason << std::endl;
			outFile << "StopTime: " << stopTime << std::endl;
			outFile << "StopIteration: " << stopIteration << std::endl;
			outFile.close();
			REPORT(0)<<"Stop information written to stopInfo.txt"<<END_REPORT;
		} else {
			REPORT(0)<<"Warning: Could not write stopInfo.txt"<<END_REPORT;
		}
	}

// this should be palced in each main 
#include "finalize.hpp"
pFlow::processors::finalizeProcessors();


}	

