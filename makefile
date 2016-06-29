all: photon core

photon:
	rm -rf build/particle/*
	mkdir -p build/particle
	cp src/*.* build/particle/
	cp examples/simple/* build/particle/
	cd build/ && particle compile photon particle

core:
	rm -rf build/particle/*
	mkdir -p build/particle
	cp src/*.* build/particle/
	cp examples/simple/* build/particle/
	cd build && particle compile core particle

clean:
	rm -rf build/*
