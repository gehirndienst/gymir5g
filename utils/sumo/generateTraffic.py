# Created by: Nikita Smirnov, nsm@informatik.uni-kiel.de
# Created Date: 06.10.2022
# version = 1.0

import argparse
import numpy
import random
import sumolib
import xml.dom.minidom

class TrafficGenerator:
    """
    Random traffic generator for ships over the waterways in SUMO networks according to the given routes. 
    
    Outputs a *ship.trips.xml file ready for the injection in sumocfg file.
    Requires routes to be prepared in a separate file. DOES NOT check their consistence with a net. 
    (Currently) DOES NOT support any other vehicle types.
    Generates traffic in a more realistic way than sumolib's randomTrips.py.
    
    :param routes_file: The routes file in SUMO format (xml with special tags)
    :param intensity: The traffic intensity from 1 to 5
    :param weights_file: The weights file for routes in xml format: <weights><weight route_id="my_route prob="0.1" />...</weights>
    :param target_route_id: Fixed route for the target (first) ship. Valuable for integration with OMNeT++ simulations
    :param end_time: Time when the simulation finishes in seconds
    :param output_file: The output xml file name.
    """
    def __init__(self, routes_file, intensity=3, weights_file=None,
                    target_route_id="pt_ferry_F1:0", end_time=10000.0, output_file="osm.ship.trips.xml"):
        if intensity in [1, 2, 3 , 4, 5]:
            self.intensity = intensity
        else:
            raise TypeError("please give intensity as an int from 1 to 5")
        self.target_route_id = target_route_id
        self.end_time = end_time
        self.outputfile = output_file
        
        self.vehicle_index = 0
        
        self.routes = self._get_routes(routes_file)
        self.weights = self._get_weights(weights_file)
        self._create_xml()
    
    def generate_traffic(self):
        time_left = 0.0
        # seconds between vehicles based on intensity and the distribution with [mean +- 0.2] with 0.5 step for more randomness
        time_step = 50 / self.intensity
        time_step_distr = [*numpy.arange(round(4 * time_step / 5), round(6 * time_step / 5) + 0.5, 0.5)]
        # # of trips on start, so that we have some traffic at 0.0 time
        vehicles_on_start = 3 * self.intensity + 1
        
        # generate starting traffic 
        # 1. for the target ship
        if self.target_route_id not in self.routes.keys():
            raise KeyError("Target route id {} has not been found in routes file".format(self.target_route_id))
        else:
            self._add_trip("ship" + str(self.vehicle_index), "pt_ferry", 0.0, "max", self.routes[self.target_route_id][0], self.routes[self.target_route_id][-1])
            self.vehicle_index += 1
        
        # 2. for other participants on start
        e_from_used_on_start = [self.routes[self.target_route_id][0]]
        while self.vehicle_index < vehicles_on_start:
            #route_id = random.choice(list(self.routes.keys()))
            route_id = numpy.random.choice(list(self.routes.keys()), 1, p=self.weights)[0] if self.weights is not None else random.choice(list(self.routes.keys()))
            e_from = random.choice(self.routes[route_id])
            e_to = random.choice(self.routes[route_id])
            # check this because we don't want trips from same edges at 0.0 time
            if e_from != e_to and e_from not in e_from_used_on_start:
                self._add_trip("ship" + str(self.vehicle_index), "pt_ferry", 0.0, "max", e_from, e_to)
                self.vehicle_index += 1
                e_from_used_on_start.append(e_from)
            else:
                continue
        
        # generate main traffic
        while time_left < self.end_time:
            time_step_rand = random.choice(time_step_distr)
            time_left += time_step_rand
            route_id = numpy.random.choice(list(self.routes.keys()), 1, p=self.weights)[0] if self.weights is not None else random.choice(list(self.routes.keys()))
            e_from = random.choice(self.routes[route_id])
            e_to = random.choice(self.routes[route_id])
            self._add_trip("ship" + str(self.vehicle_index), "pt_ferry", time_left, "max", e_from, e_to)
            self.vehicle_index += 1
        
        # save the resulting xml
        self._save_xml()
        
    
    def _add_trip(self, trip_id, trip_type, depart_time, depart_speed, trip_from, trip_to):
        trip_elem = self._doc.createElement('trip')
        trip_elem.setAttribute("id", str(trip_id))
        trip_elem.setAttribute("type", str(trip_type))
        trip_elem.setAttribute("depart", str(depart_time))
        trip_elem.setAttribute("departSpeed", str(depart_speed))
        trip_elem.setAttribute("from", str(trip_from))
        trip_elem.setAttribute("to", str(trip_to))
        self._root.appendChild(trip_elem)
    
    def _get_routes(self, routes_file):
        ids = [route.id for route in sumolib.xml.parse_fast(routes_file, 'route', ['id'])]
        edges_list = [route.edges.split() for route in sumolib.xml.parse_fast(routes_file, 'route', ['edges'])]
        return {id:edges for id, edges in zip(ids, edges_list)}
    
    def _get_weights(self, weights_file):
        if weights_file is not None:
            weights_route_ids = [weight.route_id for weight in sumolib.xml.parse_fast(weights_file, 'weight', ['route_id'])]
            weights_list = [float(weight.prob) for weight in sumolib.xml.parse_fast(weights_file, 'weight', ['prob'])]
            weights_dict = {id:prob for id, prob in zip(weights_route_ids, weights_list)}
            unsigned_probs_num = len(self.routes.keys()) - len(weights_route_ids)
            prob_average = (1 - sum(weights_list)) / unsigned_probs_num if unsigned_probs_num != 0 else 0
            return [
                weights_dict[route_id] if route_id in weights_route_ids else prob_average for route_id in self.routes.keys()
            ]
        else:
            return None
        
    def _create_xml(self):
        self._doc = xml.dom.minidom.Document()
        self._root = self._doc.createElement('routes')
        self._root.setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance")
        self._root.setAttribute('xsi:noNamespaceSchemaLocation', "http://sumo.dlr.de/xsd/routes_file.xsd")
        self._doc.appendChild(self._root)
        veh_elem = self._doc.createElement('vType') # FIXME: load vTypes from additional SUMO files
        veh_elem.setAttribute("id", "pt_ferry") 
        veh_elem.setAttribute("vClass", "ship")
        veh_elem.setAttribute("maxSpeed", "10") # TODO: add parameter for setting the speed
        self._root.appendChild(veh_elem)   
    
    def _save_xml(self):
        with open(self.outputfile, 'w') as out_xml:
            self._doc.writexml(out_xml, addindent='\t', newl='\n', encoding="utf-8")
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-r', dest='routes_file', type=str, required=True, help="routes file")
    parser.add_argument('-i', dest='intensity', type=int, choices=[1, 2, 3, 4, 5], default=3, help="traffic intensity: give int from 1 to 5")
    parser.add_argument('-w', '--weights-file', dest='weights_file', type=str, help="weights for routes file")
    parser.add_argument('-tr', '--target-route', dest='target_route_id', type=str, default="pt_ferry_F1:0", help="route for the target (first) ship")
    parser.add_argument('-et', '--end-time', dest='end_time', type=float, default=10000.0, help="simulation time in seconds")
    parser.add_argument('-o', dest='output_file', default="osm.ship.trips.xml", help="output file name")
    args = parser.parse_args()
    
    tg = TrafficGenerator(args.routes_file, intensity=args.intensity, 
        weights_file=args.weights_file, target_route_id=args.target_route_id, end_time=args.end_time, output_file=args.output_file)
    tg.generate_traffic()
    
    # an example of a console command:
    # $ python generateTraffic.py -r osm_pt.rou.xml -i 3 -w osm_pt.rou.weights.xml -o osm.ship.trips.xml
