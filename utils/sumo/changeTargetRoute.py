import argparse
import xml.dom.minidom
from generateTraffic import TrafficGenerator

class TargetRouteSetter:
    """
    Changes target route in trips file without regenerating it. Useful for multiple run with the same traffic but different routes
    
    :param trips_file: The trips file genereated by TrafficGenerator
    :param routes_file: The routes file in SUMO format (xml with special tags)
    """
    def __init__(self, trips_file, routes_file):
        self._doc = xml.dom.minidom.parse(trips_file)
        self.trips_file = trips_file
        self.tg = TrafficGenerator(routes_file)
           
    def set_target_route(self, target_route):
        target_trips = [trip_elem for trip_elem in self._doc.getElementsByTagName("trip") if trip_elem.getAttribute("id") == "ship0"]
        if not target_trips:
            raise ValueError("Target route has not been set -- please remake the trips file")
        target_trip = target_trips[0]
        target_trip.setAttribute("from", self.tg.routes[target_route][0])
        target_trip.setAttribute("to", self.tg.routes[target_route][-1])
        with open(self.trips_file, 'w') as out_xml:
            self._doc.writexml(out_xml, encoding="utf-8")
            

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', dest='trips_file', type=str, required=True, help="trips file")
    parser.add_argument('-r', dest='routes_file', type=str, required=True, help="routes file")
    parser.add_argument('-tr', '--target-route', dest='target_route', type=str, default="pt_ferry_F2:0", help="route for the target (first) ship that you want to set")
    args = parser.parse_args()
    
    tg = TargetRouteSetter(args.trips_file, args.routes_file)
    tg.set_target_route(args.target_route)