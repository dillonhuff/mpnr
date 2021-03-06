#include "coreir.h"
#include "coreir/libs/cgralib.h"
#include "coreir/libs/commonlib.h"

#include "pugixml.hpp"

#include <fstream>

#include "catch.hpp"

using namespace CoreIR;
using namespace std;

namespace mpnr {

  std::vector<std::string> readCSVLine(FILE* in) {
    char * linePtr = NULL;
    size_t len = 0;

    int res = getline(&linePtr, &len, in);
    assert(res != 0);

    std::string line(linePtr);
    vector<string> tokens;
    string currentToken = "";
    int i = 0;
    while (i < (int) line.size()) {
      if (line[i] != ',') {
        currentToken += line[i];
      } else {
        tokens.push_back(currentToken);
        currentToken = "";
      }
      
      i++;
    }
    
    return tokens;
  }
  
  void runTB(const std::string& tbFile) {
    int vcsCompileRes = system(("vcs -assert disable +nbaopt +rad +nospecify +notimingchecks -ld gcc-4.4 +vcs+lic+wait -licqueue +cli -sverilog -full64 +incdir+/hd/cad/synopsys/dc_shell/latest/packages/gtech/src_ver/ +incdir+/hd/cad/synopsys/dc_shell/latest/dw/sim_ver/ -y /hd/cad/synopsys/dc_shell/latest/dw/sim_ver/ -CFLAGS '-O3 -march=native' ./cgra_verilog/*.v ./cgra_verilog/*.sv " + tbFile + " -top test").c_str());
    assert(vcsCompileRes == 0);

    int vcsRunRes = system("./simv");
    assert(vcsRunRes == 0);
  }

  class TileCoordinates {
  public:
    int first;
    int second;
  };

  std::ostream& operator<<(std::ostream& out, const TileCoordinates a) {
    out << "(" << a.first << ", " << a.second << ")";
    return out;
  }

  bool operator==(const TileCoordinates a, const TileCoordinates b) {
    return (a.first == b.first) && (a.second == b.second);
  }

  bool operator<(const TileCoordinates a, const TileCoordinates b) {
    if (a.first < b.first) {
      return true;
    }

    // a.first >= b.first
    if (a.first == b.first) {
      return a.second < b.second;
    }

    // a.first > b.first
    return false;
  }

  enum CGRAWireType {
    CGRA_WIRE_GLOBAL,
    CGRA_WIRE_IO
  };

  class CGRAWire {
  public:
    TileCoordinates location;
    std::string name;
  };

  bool operator==(const CGRAWire& l, const CGRAWire& r) {
    return (l.location == r.location) && (l.name == r.name);
  }

  std::ostream& operator<<(std::ostream& out, const CGRAWire& wire) {
    out << wire.location << "." << wire.name;
    return out;
  }

  int getIOWireSide(const CGRAWire wire) {
    //cout << "IO wire name = " << wire.name << endl;

    if (wire.name[0] == 'o') {
      string name = wire.name;
      string side = name.substr(11, 1);
      //cout << "side = " << side << endl;
      return stoi(side);
    } else {
      assert(wire.name[0] == 'i');

      string name = wire.name;
      string side = name.substr(10, 1);
      //cout << "side = " << side << endl;
      return stoi(side);

    }
  }

  int getIOWireTrack(const CGRAWire wire) {
    //cout << "IO wire name = " << wire.name << endl;

    if (wire.name[0] == 'o') {
      string name = wire.name;
      string side = name.substr(14, 1);
      //cout << "track = " << side << endl;
      return stoi(side);
    } else {
      assert(wire.name[0] == 'i');

      string name = wire.name;
      string side = name.substr(13, 1);
      //cout << "track = " << side << endl;
      return stoi(side);

    }
  }

  int getGlobalWireSide(const CGRAWire wire) {
    //cout << "Wire name = " << wire.name << endl;
    if (wire.name[0] == 'o') {
      string name = wire.name;
      string side = name.substr(11, 1);
      //cout << "side = " << side << endl;
      return stoi(side);
    } else {
      assert(wire.name[0] == 'i');
      string name = wire.name;
      string side = name.substr(10, 1);
      //cout << "side = " << side << endl;
      return stoi(side);
    }
  }

  int getGlobalWireTrack(const CGRAWire wire) {
    //cout << "Wire name = " << wire.name << endl;
    if (wire.name[0] == 'o') {
      string name = wire.name;
      string side = name.substr(14, 1);
      //cout << "track = " << side << endl;
      return stoi(side);
    } else {
      assert(wire.name[0] == 'i');

      string name = wire.name;
      string side = name.substr(16, 1);
      //cout << "track = " << side << endl;
      return stoi(side);
    }
  }
  
  int oppositeSide(const int side) {
    assert(side < 4);
    return (side + 2) % 4;
  }
  

  enum TileType {
    TILE_TYPE_IO_1,
    TILE_TYPE_IO_16,
    TILE_TYPE_PE,
    TILE_TYPE_MEM,
    TILE_TYPE_LOWER_MEM,
    TILE_TYPE_EMPTY
  };

  TileType tileType(const CoreIR::Instance& tile) {
    return TILE_TYPE_IO_16;
  }

  class CGRAMux {
  public:
    std::string snk;
    std::map<int, std::string> inputs;
  };

  class CGRAComponent {
  public:
    std::string componentType;

    int featureAddress;

    std::vector<CGRAMux> muxes;
  };

  class CGRATile {
  public:
    TileType tp;
    int tileNumber;

    TileCoordinates coordinates;

    std::vector<std::string> globalOutWires;
    std::vector<std::string> globalInWires;

    std::vector<CGRAComponent> components;
  };

  std::ostream& operator<<(std::ostream& out, const CGRATile& tile) {
    out << "TILE: " << tile.tp << ", " << tile.tileNumber << ", " << tile.coordinates << endl;
    return out;
  }

  CGRATile peTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_PE, tileNumber, loc};
  }

  CGRATile memTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_MEM, tileNumber, loc};
  }

  CGRATile lowerMemTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_LOWER_MEM, tileNumber, loc};
  }
  
  CGRATile emptyTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_EMPTY, tileNumber, loc};
  }

  CGRATile io1Tile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_IO_1, tileNumber, loc};
  }

  CGRATile io16Tile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_IO_16, tileNumber, loc};
  }
  
  class CGRA {
    int pe_grid_len;

    //std::vector<std::vector<CGRATile> > tileRows;
    std::map<TileCoordinates, CGRATile> tileMap;;
    std::set<TileCoordinates> occupiedTiles;

  public:
    CGRA(const std::string& str) {
      pugi::xml_document doc;
      auto result = doc.load_file("./cgra_verilog/cgra_info.txt");
      REQUIRE(result);

      int maxRow = 0;
      int maxCol = 0;
      for (auto cgra : doc.children("CGRA")) {
	for (auto tile : cgra.children("tile")) {
	  string tileType = tile.attribute("type").as_string();
	  //cout << "Tile type = " << tileType << endl;

	  int row = tile.attribute("row").as_int();
	  int col = tile.attribute("col").as_int();
	  int tileNumber = 0;

	  if (row > maxRow) {
	    maxRow = row;
	  }
	  if (col > maxCol) {
	    maxCol = col;
	  }

	  if (tileType == "pe_tile_new") {
	    tileMap[{row, col}] = peTile(tileNumber, {row, col});
	  } else if (tileType == "empty") {
	    tileMap[{row, col}] = emptyTile(tileNumber, {row, col});
	  } else if (tileType == "io1bit") {
	    tileMap[{row, col}] = io1Tile(tileNumber, {row, col});
	  } else if (tileType == "io16bit") {
	    tileMap[{row, col}] = io16Tile(tileNumber, {row, col});
	  } else if (tileType == "memory_tile") {
	    tileMap[{row, col}] = memTile(tileNumber, {row, col});
	    tileMap[{row + 1, col}] = lowerMemTile(tileNumber, {row + 1, col});
	  } else {
	    assert(tileType == "gst");
	  }

	  TileCoordinates coords = {row, col};
	  CGRATile& currentTile = tileMap[coords];

	  //cout << "out wires" << endl;
	  vector<string> ioOutWires;
	  for (auto out : tile.children("output")) {
	    for (auto c : out.children()) {
	      //cout << "\t" << c.value() << endl;
	      currentTile.globalOutWires.push_back(c.value());
	    }
	  }

	  vector<string> inWires;
	  for (auto out : tile.children("input")) {
	    for (auto c : out.children()) {
	      currentTile.globalInWires.push_back(c.value());
	    }
	  }

	  //cout << "Outgoing global wires from " << tileType << endl;
	  for (auto sb : tile.children("sb")) {
	    CGRAComponent sbComp;
	    sbComp.componentType = "sb";
	    sbComp.featureAddress = sb.attribute("feature_address").as_int();
	    
	    // Add sbComp.muxes
	    for (auto m : sb.children("mux")) {
	      string snk = m.attribute("snk").as_string();
	      //cout << "\tglobal wire = " << snk << endl;
	      map<int, string> srcs;
	      for (auto src : m.children("src")) {
		int srcNum = src.attribute("sel").as_int();
		string srcName = (*std::begin(src.children())).value();
		srcs.insert({srcNum, srcName});
	      }

	      sbComp.muxes.push_back({snk, srcs});

	    }


	    tileMap[coords].components.push_back(sbComp);
	  }

	}
      }

      cout << "maxRow = " << maxRow << endl;
      cout << "maxCol = " << maxCol << endl;
      assert(maxRow == maxCol);

      pe_grid_len = maxCol - 3;
    }

    void setOccupied(const TileCoordinates coords) {
      occupiedTiles.insert(coords);
    }

    bool unoccupied(const int tileRow, const int tileCol) const {
      return !elem({tileRow, tileCol}, occupiedTiles);
    }

    std::vector<TileCoordinates> unplacedTilesOfType(const TileType& tp) const {
      vector<TileCoordinates> tiles;

      for (auto p : tileMap) {
	TileCoordinates coords = p.first;
	auto type = p.second.tp;
	if ((type == tp) && (unoccupied(coords.first, coords.second))) {
	  tiles.push_back(coords);
	}
      }

      return tiles;
    }

    int getPEGridLen() const {
      return pe_grid_len;
    }

    CGRATile getTile(const TileCoordinates coords) const {
      return map_find(coords, tileMap);
    }

    CGRAWire wireFor(const std::map<CoreIR::Instance*, TileCoordinates>& placement,
		     CoreIR::Select* val) {
      Wireable* w = val->getParent();
      assert(isa<Instance>(w));

      Instance* node = cast<Instance>(w);
      cout << "Source node = " << node->toString() << endl;
      TileCoordinates t = map_find(node, placement);
      CGRATile tileFor = getTile(t);
      std::string portName = val->getSelStr();

      cout << "portName = " << portName << endl;
      cout << "coordinates = " << t << endl;
      
      if (tileFor.tp == TILE_TYPE_IO_16) {
	assert((portName == "in") || (portName == "out"));
	assert(tileFor.globalOutWires.size() > 0);
	assert(tileFor.globalInWires.size() > 0);

	if (portName == "out") {
	  return {t, tileFor.globalOutWires[0]};
	}

	return {t, tileFor.globalInWires[0]};
      }

      assert(false);
    }

    void printPlacement(const std::map<CoreIR::Instance*, TileCoordinates>& placement) const {
      for (int i = 0; i < pe_grid_len + 4; i++) {
      	//auto& gridRow = tileRows[i];
      	for (int j = 0; j < pe_grid_len + 4; j++) {
	  TileCoordinates coords = {(int) i, (int) j};
	  if (!contains_key(coords, tileMap)) {
	    cout << "coords = " << coords << " not in tilemap " << endl;
	  }
	  assert(contains_key(coords, tileMap));
	  
	  //CGRATile t = map_find(coords, tileMap);

      	  bool isPlaced = false;
      	  for (auto place : placement) {
      	    if (place.second == coords) {
      	      isPlaced = true;
      	      break;
      	    }
      	  }

      	  if (isPlaced) {
      	    cout << " P";
      	  } else {
      	    cout << " .";
      	  }
      	}
      	cout << " " << endl;
      }
    }

    std::vector<CGRAWire> getNet(const CGRAWire wire) const {
      CGRATile tile = getTile(wire.location);

      cout << "Getting net of " << wire << endl;

      // 16 bit IO tiles have one neighbor tile
      if (tile.tp == TILE_TYPE_IO_16) {
	int side = getIOWireSide(wire);
	int track = getIOWireTrack(wire);

	int outputSide = oppositeSide(side);
	if (outputSide == 3) {
	  CGRATile receiverTile = getTile({tile.coordinates.first + 1, tile.coordinates.second});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};

	}

	if (outputSide == 1) {
	  CGRATile receiverTile = getTile({tile.coordinates.first - 1, tile.coordinates.second});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}

	if (outputSide == 0) {
	  CGRATile receiverTile = getTile({tile.coordinates.first, tile.coordinates.second - 1});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}

	if (outputSide == 2) {
	  CGRATile receiverTile = getTile({tile.coordinates.first, tile.coordinates.second + 1});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}
	
	assert(false);
      } else if (tile.tp == TILE_TYPE_PE) {
	int side = getGlobalWireSide(wire);
	int track = getGlobalWireTrack(wire);

	int outputSide = oppositeSide(side);
	if (outputSide == 3) {
	  CGRATile receiverTile = getTile({tile.coordinates.first + 1, tile.coordinates.second});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}

	if (outputSide == 1) {
	  CGRATile receiverTile = getTile({tile.coordinates.first - 1, tile.coordinates.second});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}

	if (outputSide == 0) {
	  CGRATile receiverTile = getTile({tile.coordinates.first, tile.coordinates.second - 1});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}

	if (outputSide == 2) {
	  CGRATile receiverTile = getTile({tile.coordinates.first, tile.coordinates.second + 1});
	  cout << "Reciever tile location = " << receiverTile.coordinates << endl;

	  //TileCoordinates receiverLoc = receiverTile.coordinates;
	  std::string sameNetWire = "in_BUS16_S" + to_string(outputSide) + "_T" + to_string(track);
	  cout << "Same net wire in receiver tile = " << sameNetWire << endl;

	  return {wire, {receiverTile.coordinates, sameNetWire}};
	}
	
	assert(false);
	// cout << "PE tile wire" << endl;
	// assert(false);
	
      } else {
	assert(false);
      }

      assert(false);
    }

    std::vector<CGRAWire> receiverSinks(const CGRAWire wire) {
      vector<CGRAWire> possibleConnections;

      CGRATile receiverTile = getTile(wire.location);

      for (auto comp : receiverTile.components) {
	for (auto mux : comp.muxes) {
	  //cout << "\tmux inputs" << endl;
	  bool viableOutput = false;
	  for (auto input : mux.inputs) {
	    if (input.second == wire.name) {
	      viableOutput = true;
	      break;
	    }
	    //cout << "\t\t" << input.second << endl;
	  }

	  if (viableOutput) {
	    possibleConnections.push_back({receiverTile.coordinates, mux.snk});
	  }
	}
      }

      return possibleConnections;
    }
    
    std::vector<CGRAWire> possibleConnections(const CGRAWire wire) {
      vector<CGRAWire> possibleConnections;
      for (auto w : getNet(wire)) {
	concat(possibleConnections, receiverSinks(w));
      }

      return possibleConnections;
    }

    int rc_distance(const CGRAWire& a, const CGRAWire& b) const {
      // Need to compute the minimul distance between nets
      int min = pe_grid_len*200;
      for (auto aN : getNet(a)) {
	for (auto bN : getNet(b)) {

	  TileCoordinates aC = aN.location;
	  TileCoordinates bC = bN.location;
	  cout << "aN = " << aN << endl;
	  cout << "bN = " << bN << endl;
	  int d = fabs(aC.first - bC.first) + fabs(aC.second - bC.second);
	  cout << "d  = " << d << endl;

	  if (d < min) {
	    min = d;
	  }
	}
      }

      return min;
    }

  };

  bool sameTile(const TileCoordinates& a, const TileCoordinates b) {
    return (a.first == b.first) && (a.second == b.second);
  }

  typedef std::map<Instance*, TileCoordinates> Placement;

  bool findRoute(const CGRAWire& srcWire,
		 const CGRAWire& dstWire,
		 CGRA& cgra) {

    // If the destination is already in the same net as the source we are done
    for (auto w : cgra.getNet(srcWire)) {
      if (w == dstWire) {
	return true;
      }
    }

    std::vector<CGRAWire> nextSteps =
      cgra.possibleConnections(srcWire);
    cout << "Possible next steps in path:" << endl;
    for (auto next : nextSteps) {
      cout << "\t" << next << endl;
    }

    if (nextSteps.size() == 0) {
      // Unset route
      return false;
    }

    sort_lt(nextSteps, [dstWire, cgra](const CGRAWire& l) { return cgra.rc_distance(l, dstWire); });
    //for (auto possibleStep : nextSteps) {
    CGRAWire possibleStep = nextSteps[0];
    cout << "Trying path = " << possibleStep << endl;
    bool routed = findRoute(possibleStep, dstWire, cgra);
    if (routed) {
      cout << "Next step is " << possibleStep << endl;
      return true;
    }
    
    return false;
  }

  void routeApplication(ModuleDef& def, const Placement& placement, CGRA& cgra) {
    // Basically: For every connection:
    // 1. Convert the connection into some tile coordinate and wire and within the tile?
    // 2. Find route from source wire to dest wire?
    for (auto conn : def.getConnections()) {
      cout << conn.first->toString() << " <-> " << conn.second->toString() << endl;
      bool firstIsIn = conn.first->getType()->isInput();
      bool firstIsOut = conn.first->getType()->isOutput();

      Wireable* src = firstIsIn ? conn.second : conn.first;
      Wireable* dst = firstIsIn ? conn.first : conn.second;

      cout << src->toString() << " -> " << dst->toString() << endl;
      cout << "First is input  = " << firstIsIn << endl;
      cout << "First is output = " << firstIsOut << endl;

      CGRAWire srcWire = cgra.wireFor(placement, cast<Select>(src));
      cout << "Source coords = " << srcWire.location << ", Source wire name = " << srcWire.name << endl;
      CGRAWire dstWire = cgra.wireFor(placement, cast<Select>(dst));
      cout << "Dest coords   = " << dstWire.location << ", Dest wire name   = " << dstWire.name << endl;

      bool foundRoute = findRoute(srcWire, dstWire, cgra);
      assert(foundRoute);
    }

    assert(false);
  }

  std::map<Instance*, TileCoordinates> placeApplication(ModuleDef& def, CGRA& cgra) {
    map<Instance*, TileCoordinates> placement;

    for (auto instPair : def.getInstances()) {
      Instance* inst = instPair.second;
      std::string tpName = inst->getModuleRef()->getName();

      cout << "Type name = " << tpName << endl;

      auto possibleTiles = cgra.unplacedTilesOfType(tileType(*inst));

      assert(possibleTiles.size() > 0);

      placement[inst] = possibleTiles[0];
      cgra.setOccupied(possibleTiles[0]);
    }

    return placement;
  }

  void outputBitstream(const CGRA& cgra, const std::string& fileName) {
    ofstream out(fileName);
    out.close();
  }

  TEST_CASE("Parse CGRA info") {
    pugi::xml_document doc;
    auto result = doc.load_file("./cgra_verilog/cgra_info.txt");
    REQUIRE(result);

    for (auto cgra : doc.children("CGRA")) {
      for (auto tile : cgra.children("tile")) {
	string tileType = tile.attribute("type").as_string();
	//cout << "Tile type = " << tileType << endl;
	
      }
    }
  }
  
  TEST_CASE("Route input to output") {
    Context* c = newContext();
    CoreIRLoadLibrary_cgralib(c);
    CoreIRLoadLibrary_commonlib(c);

    Values w16 = {{"width", Const::make(c, 16)}};

    Module* Top = c->getGlobal()->newModuleDecl("Top",c->Record());
    ModuleDef* def = Top->newModuleDef();

    def->addInstance("io0","cgralib.IO",w16,{{"mode",Const::make(c,"i")}});
    def->addInstance("io1","cgralib.IO",w16,{{"mode",Const::make(c,"o")}});
    
    def->connect("io0.out","io1.in");

    //CGRA cgra(16);
    CGRA cgra("./cgra_verilog/cgra_info.txt");
    map<Instance*, TileCoordinates > tilePlacement =
      placeApplication(*def, cgra);

    SECTION("All tiles are placed") {
      REQUIRE(tilePlacement.size() == 2);
    }

    SECTION("All tiles are in distinct locations") {
      vector<TileCoordinates> coords;

      for (auto place : tilePlacement) {
  	TileCoordinates t = place.second;
  	for (auto c : coords) {
  	  REQUIRE(!sameTile(c, t));
  	}
  	coords.push_back(t);
      }

      cgra.printPlacement(tilePlacement);
    }

    SECTION("Route, output bitstream, and execute routed code") {

      // Route the application. This function should modify the CGRA
      // so that it can be output
      routeApplication(*def, tilePlacement, cgra);

      // Write the bitstream out to a file
      outputBitstream(cgra, "passthrough.bsa");

      string testFile = "verilog_tbs/test_passthrough_route.v";
      runTB(testFile);

      FILE* resFile = fopen("passthrough.txt", "r");

      while (!feof(resFile)) {
	auto l1Res = readCSVLine(resFile);

	if (l1Res.size() == 0) {
	  cout << "Found last line, done" << endl;
	  break;
	}

	cout << "Line" << endl;
	for (auto tok : l1Res) {
	  cout << "\t" << tok << endl;
	}

	BitVector side3Out = BitVector(16, l1Res[3]);

	REQUIRE(side3Out != BitVector(16, 0));
      }
      
      fclose(resFile);
      
    }

    deleteContext(c);
  }
}
