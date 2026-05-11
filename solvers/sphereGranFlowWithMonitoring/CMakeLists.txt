
set(source_files
sphereGranFlowWithMonitoring.cpp
)
set(link_lib Kokkos::kokkos phasicFlow Particles Geometry Property Interaction Interaction Utilities)

pFlow_make_executable_install(sphereGranFlowWithMonitoring source_files link_lib)

