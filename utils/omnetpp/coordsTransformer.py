# Created by: Nikita Smirnov, nsm@informatik.uni-kiel.de
# Created Date: 11.10.2022
# version = 1.0

import argparse
import datetime
import json
import sumolib

class Coord:  
    def __init__(self, x, y):
        self.x = x
        self.y = y

class GeoCoord:  
    def __init__(self, lon, lat):
        self.lon = lon
        self.lat = lat
      
class CoordTransformer:
    """
    Transforms coordinates between three coordinate systems:
        1. geo: longitude and latitude
        2. traci: SUMO/Veins plane cs
        3. omnet: OMNeT++ plane cs
    
    :param net_file: The net file (SUMO)
    :param margin: A margin for traci-omnet conversion, default is 25, look it at VeinsInetManager.margin
    :param is_write: whether to save transformed coords as a file
    """
    def __init__(self, net_file, margin=25, is_write=True):
        self.net = sumolib.net.readNet(net_file)
        bounds = self.net.getBoundary()
        offset = self.net.getLocationOffset()
        self.offset = Coord(offset[0], offset[1])
        self.bottomleft = Coord(bounds[0], bounds[1])
        self.topright = Coord(bounds[2], bounds[3])
        self.dimensions = Coord(self.topright.x - self.bottomleft.x, self.topright.y - self.bottomleft.y)
        self.margin = margin
        self.is_write = is_write
    
    # you should supply here a json file of the following structure: {objects:[{ name: obj_name, lon: obj_lon, lat: obj_lat}, ...]}
    def geo2omnet_from_json(self, geo_json):
        omnet_coords = []
        geo_coords_json = json.load(open(geo_json))
        for geo_coord_dict in geo_coords_json["objects"]:
            omnet_coord = self.traci2omnet(self.geo2traci(GeoCoord(geo_coord_dict["lon"], geo_coord_dict["lat"])))
            omnet_coords.append(
                {
                    "name": geo_coord_dict["name"],
                    "x": omnet_coord.x,
                    "y": omnet_coord.y
                }
            )
        res_json = {"objects": omnet_coords}
        # dump as json if needed
        if self.is_write:
            json.dump(res_json, open(datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S") + "_omnet_coords.json", 'w'), indent='\t', separators=(',', ': '))
        return res_json
    
    # taken from sumolib
    def geo2traci(self, geoCoords):
        x, y = self.net.convertLonLat2XY(geoCoords.lon, geoCoords.lat)
        return Coord(x, y)

    def traci2geo(self, traciCoords):
        return GeoCoord(
            self.net.convertXY2LonLat(traciCoords.x, traciCoords.y)
        )

    # traci 2 omnet are copied from veins: veins/modules/mobility/traci/TraCICoordinateTransformation
    def traci2omnet(self, traciCoords):
        return Coord(
            traciCoords.x - self.bottomleft.x + self.margin, 
            self.dimensions.y - (traciCoords.y - self.bottomleft.y) + self.margin
        )

    def omnet2traci(self, omnetCoords):
        return Coord(
            omnetCoords.x + self.bottomleft.x - self.margin, 
            self.dimensions.y - (omnetCoords.y - self.bottomleft.y) + self.margin
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', dest='net_file', type=str, required=True, help="net file")
    parser.add_argument('-m', '--margin', dest='margin', type=float, default=25.0, help="traci margin")
    parser.add_argument('-g', '--geo-json', dest='geo_json', default=None, help="input of geo coords which need to be transformed to omnet ones")
    args = parser.parse_args()
    
    ct = CoordTransformer(args.net_file, args.margin, True)
    if args.geo_json is not None:
        ct.geo2omnet_from_json(args.geo_json)