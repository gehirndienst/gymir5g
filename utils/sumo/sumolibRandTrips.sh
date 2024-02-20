# change -p parameter for repititions, the higher it is, the more trips will be generated
# change --fringe-factor parameter for adding more "out-of" trips, i.e. from the border junctions. 10 is default value and is good for Kileer Förde scenario

python randomTrips.py -n osm.net.xml -a osm_stops.add.xml --fringe-factor 10 -p 0.5 -o osm.ship.trips.xml -e 10000 --vehicle-class ship --vclass ship --prefix ship --validate --fringe-start-attributes "departSpeed=\"max\""