# Created by: Nikita Smirnov, nsm@informatik.uni-kiel.de
# Created Date: 07.10.2022
# Last Version: 21.04.2023
# version = 1.1

import argparse
import shapely.geometry as sg
import sumolib
import xml.etree.ElementTree as ET

class PolyReducer:
    """
    Reduces SUMO poly file to match the net bounds to prevent surplus rendering. Saves passed entities in a new file.
    DOES NOT change "location" tag, one should copy-paste it manually from a net file.
    
    :param poly_file: The poly file (SUMO from OSM)
    :param net_file: The net file (SUMO)
    :param output_file: The output xml file name.
    """
    
    def __init__(self, poly_file, net_file, output_file="reduced.poly.xml"):
        # load the net file to get the boundaries
        net_boundaries_coords = sumolib.net.readNet(net_file).getBoundary()
        xmin = net_boundaries_coords[0]
        xmax = net_boundaries_coords[2]
        ymin = net_boundaries_coords[1]
        ymax = net_boundaries_coords[3]

        # parse the boundaries to a Polygon object
        self.net_boundaries_poly = sg.Polygon([(xmin, ymin), (xmin, ymax), (xmax, ymax), (xmax, ymin)])

        # load the poly file and parse polygons
        self.tree = ET.parse(poly_file)
        self.root = self.tree.getroot()
        
        # output
        self.output_file = output_file 
    
    def reduce(self):
        while True:
            root_changed = False

            # polygons
            for poly in self.root.iter('poly'):
                poly_shape = poly.attrib['shape']

                # parse the polygon shape to a Polygon object if it has at least 4 coordinates, parse it to LineString or Point otherwise
                shape_coords = [(float(coord.split(',')[0]), float(coord.split(',')[1])) for coord in poly_shape.split(' ')]
                if len(shape_coords) >= 4:
                    poly_shape = sg.Polygon(shape_coords)
                    if not poly_shape.is_valid:
                        poly_shape = poly_shape.buffer(0)
                elif 1 < len(shape_coords) < 4:
                    poly_shape = sg.LineString(shape_coords)
                else:
                    poly_shape = sg.Point(shape_coords)

                # check if a shape doesn't lay within a net polygon
                if not poly_shape.within(self.net_boundaries_poly):
                    root_changed = True
                    # check if the polygon intersects with the net boundaries
                    if not self.net_boundaries_poly.intersects(poly_shape):
                        # delete the polygon -- it lies completely outside the boundaries
                        self.root.remove(poly)
                    else:
                        # cut the polygon to fit the boundaries if it lies partially inside the boundaries
                        poly_shape = poly_shape.intersection(self.net_boundaries_poly)
                        # convex multipolygon
                        if isinstance(poly_shape, sg.MultiPolygon):
                            poly_shape = poly_shape.convex_hull
                        poly.attrib['shape'] = ' '.join([f"{coord[0]},{coord[1]}" for coord in (poly_shape.exterior.coords if isinstance(poly_shape, sg.Polygon) else poly_shape.coords)])
            
            # pois
            for poi in self.root.iter('poi'):
                x = poi.attrib['x']
                y = poi.attrib['y']
                point = sg.Point([x, y])
                if not point.within(self.net_boundaries_poly):
                    root_changed = True
                    self.root.remove(poi)

            if not root_changed:
                break
        
        if self.output_file:
            self.save_xml()

    def save_xml(self):
        self.tree.write(self.output_file)
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', dest='poly_file', type=str, required=True, help="poly file")
    parser.add_argument('-n', dest='net_file', type=str, required=True, help="net file")
    parser.add_argument('-o', dest='output_file', default="reduced.poly.xml", help="output file name")
    args = parser.parse_args()
    
    pr = PolyReducer(args.poly_file, args.net_file, args.output_file)
    pr.reduce()