#!/bin/sh
java -Djava.library.path=./libs/`./src/platform`/ -jar ZenGarden.jar pd-patches/simple_osc.pd
