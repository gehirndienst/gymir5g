# Created by: Nikita Smirnov, nsm@informatik.uni-kiel.de
# Created Date: 12.10.2022
# version = 1.1

import argparse
import enum
import itertools
import math

from coordsTransformer import CoordTransformer

X2_MAX_DISTANCE = 5000

class NetworkType(enum.Enum):
    DC_CLOSE = 1 # NSA 5G: DC where 4G and 5G bss almost at the same position
    DC_AWAY = 2 # NSA 5G: DC where 4G and 5G bss are distanced
    NR = 3 # SA 5G

class BaseStationsConfigurationScenario:
    """
    Serves as a secondary tool for base station configurator and stores all information about network type and distances between base stations.
    :param net_type: 5G NSA with close or distanced master/secondary stations generated automatically or 5G SA
    :param bs_distance: The distance between base 4G and secondary 5G station. 0 in case SA
    :param x2_distance: The maximum distance between master stations that allows adding an X2 link. 0 in case SA
    """
    def __init__(self, net_type, bs_distance=0, x2_distance=0) -> None:
        if net_type == "dc_close":
            self.net_type = NetworkType.DC_CLOSE
        elif net_type == "dc_away":
            self.net_type = NetworkType.DC_AWAY
        elif net_type == "nr":
            self.net_type = NetworkType.NR
        else:
            raise ValueError("network type for base station configuration is unknown")
        
        # 5 meters distance as a default one for CLOSE scenario
        self.bs_distance = 5 if self.net_type == NetworkType.DC_CLOSE else (bs_distance if self.net_type == NetworkType.DC_AWAY else 0)
        # 0 means we connect all bss with x2, in DC all LTE bss and all LTE-NR bss
        self.x2_distance = max(min(x2_distance, X2_MAX_DISTANCE), bs_distance) if x2_distance > 0 else 0

        
class BaseStationsConfigurator:
    """
    Generates NED and INI files content for a given set of base stations in geo coordinates within json file.
    Adds declaration and upf and x2 connection links for master and secondary stations to the ned file. 
    Adds x2 connection addresses for master and secondary stations to ini file.
    Saves output in txt file.
    
    :param net_file: The SUMO network file
    :param geo_json: The json file with geo coordinates of base stations. Transform coordinates automatically based on net_file
    :param conf_scenario: Configuration scenario that defines type of network (SA, NSA) and distances for x2 links adding
    """
    def __init__(self, net_file, geo_json, conf_scenario: BaseStationsConfigurationScenario):
        ct = CoordTransformer(net_file=net_file, is_write=False)
        self.coords = ct.geo2omnet_from_json(geo_json)
        self.conf_scenario = conf_scenario
    
    def configure(self):
        names = []
        names_5g = []
        
        ned_declaration_master = ""
        ned_declaration_slave = ""
        ned_connections_upf = ""
        ned_connections_x2_master_to_master = ""
        ned_connections_x2_master_to_slave = ""
        ini_x2 = ""
        
        for bs in self.coords["objects"]:
            # check whether we have only eNBs, gNBs or dc. Assumed that value under "name" tag is eNB*
            name = bs["name"]
            names.append(name)
            name_5g = "g" + name[1:] if self.conf_scenario.net_type != NetworkType.NR else ""
            if name_5g:
                names_5g.append(name_5g)
            
            # NED declaration masters and slaves
            ned_type_4g = "eNodeB" if self.conf_scenario.net_type != NetworkType.NR else "gNodeB"
            ned_type_5g = "gNodeB" if self.conf_scenario.net_type != NetworkType.NR else ""
            ned_declaration_master += self._get_ned_declaration(name, bs["x"], bs["y"], ned_type_4g)
            ned_declaration_slave += self._get_ned_declaration(name_5g, 
                bs["x"] + conf_scenario.bs_distance, bs["y"] + conf_scenario.bs_distance, ned_type_5g)
            
            # NED upf: only for masters in any case
            upf_or_pgw = "pgw" if self.conf_scenario.net_type != NetworkType.NR else "upf"
            ned_connections_upf += self._get_ned_upf(name, upf_or_pgw)
        
        # NED x2 masters to masters
        names_comb = list(itertools.combinations(names, 2))
        for names_tuple in names_comb:
            ned_connections_x2_master_to_master += self._get_ned_x2_with_limit(names_tuple, names)
        
        # NED x2 masters to slaves
        if len(names) == len(names_5g):
            for i in range(0, len(names)):
                ned_connections_x2_master_to_slave += self._get_ned_x2((names[i], names_5g[i]))
            
        # INI
        ini_x2 = self._get_ini_x2(names, names_5g)
        
        # save the resulting string
        res_string = "# NED declaration masters: " + ned_declaration_master + "\n\n# NED declaration slaves: " + ned_declaration_slave \
                + "\n\n# NED upf: " + ned_connections_upf \
                + "\n\n# NED x2 master to master: " + ned_connections_x2_master_to_master \
                + "\n\n# NED x2 master to slave: " + ned_connections_x2_master_to_slave \
                + "\n\n# INI x2: " + ini_x2
        with open("output.txt", "w") as f:
            f.write(res_string)
    
    def _get_ned_declaration(self, bs_name, bs_x, bs_y, bs_ned_type):
        if not bs_ned_type:
            return ""
        else:
            display = f'@display("{"p=" + str(bs_x) + "," + str(bs_y) + ";is=s"}");'
            declaration = "\n" + bs_name + ": " + bs_ned_type +  "{\n\t" + display + "\n" + "}"
            return declaration + "\n"
    
    def _get_ned_upf(self, name, upf_or_pgw):
        return "\n" + upf_or_pgw + ".pppg++ <--> Eth10G <--> " + name + ".ppp;"
    
    def _get_ned_x2_with_limit(self, name_tuple, names):
        index_master_first = names.index(name_tuple[0])
        index_master_second = names.index(name_tuple[1])
        distance = self._get_bs_distance(index_master_first, index_master_second)
        return  "" \
            if 0 < self.conf_scenario.x2_distance < distance \
            else self._get_ned_x2(name_tuple)
    
    def _get_ned_x2(self, name_tuple):
        return "\n" + name_tuple[0] + ".x2++ <--> Eth10G <--> " + name_tuple[1] + ".x2++;"

    
    def _get_ini_x2(self, names_masters, names_slaves):
        x2 = "" if not names_slaves else "\n*.gNB*.numX2Apps = 1"
        x2_slaves = ""
        x2_masters = ""
        # INI x2
        for name_master in names_masters:
            this_master_to_masters = ""
            this_master_to_slave = ""
            slave_to_this_master = ""
            index_master = names_masters.index(name_master)
            index_x2 = 0
            # master to slave
            if names_slaves:
                name_slave = names_slaves[index_master]
                slave_to_this_master += "\n*." + name_slave + ".x2App[0].client.connectAddress = " + f'"{name_master + "%x2ppp0"}"'
                this_master_to_slave += "\n*." + name_master + ".x2App[0].client.connectAddress = " + f'"{name_slave + "%x2ppp0"}"'
                index_x2 += 1
            # master to master
            for name_sec in names_masters:
                index_sec = names_masters.index(name_sec)
                distance = self._get_bs_distance(index_master, index_sec)
                if name_sec == name_master or 0 < self.conf_scenario.x2_distance < distance:
                    continue
                index_end = index_master if index_sec > index_master else index_master - 1
                index_end = index_end + 1 if names_slaves else index_end
                this_master_to_masters += "\n*." + name_master + ".x2App[" + str(index_x2) + "].client.connectAddress = " + f'"{name_sec + "%x2ppp" + str(index_end)}"'
                index_x2 += 1

            # finally
            x2_slaves += slave_to_this_master
            x2_masters += "\n*." + name_master + ".numX2Apps = " + str(index_x2)
            x2_masters += this_master_to_slave + this_master_to_masters
        
        x2 += x2_slaves + x2_masters
        return x2

    def _get_bs_distance(self, index_first, index_sec):
        first_x = self.coords["objects"][index_first]["x"]
        sec_x = self.coords["objects"][index_sec]["x"]
        first_y = self.coords["objects"][index_first]["y"]
        sec_y = self.coords["objects"][index_sec]["y"]
        return math.sqrt(math.pow(first_x - sec_x, 2) + math.pow(first_y - sec_y, 2))

        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', dest='net_file', type=str, required=True, help="net file")
    parser.add_argument('-g', '--geo-json', dest='geo_json', required=True, help="input of geo coords which need to be transformed to omnet ones")
    parser.add_argument('--net-type', dest='net_type', choices=["dc_close", "dc_away", "nr", "lte"], default="dc_close", help="net type: select one of: dc_close, dc_away, nr, lte")
    parser.add_argument('--bs-distance', dest='bs_distance', type=float, default=5.0, help="distance between bs in case of dual connectivity scenario")
    parser.add_argument('--x2-distance', dest='x2_distance', type=float, default=0, help="a limit distance for opening an X2 connection; 0 means all bss are connected")
    args = parser.parse_args()
    
    conf_scenario = BaseStationsConfigurationScenario(args.net_type, args.bs_distance, args.x2_distance)
    conf = BaseStationsConfigurator(args.net_file, args.geo_json, conf_scenario)
    conf.configure()