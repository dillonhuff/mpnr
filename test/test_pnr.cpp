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
    CGRAWireType tp;
    int width;
    int side;
    int track;
  };

  enum TileType {
    TILE_TYPE_IO_1,
    TILE_TYPE_IO_16,
    TILE_TYPE_PE,
    TILE_TYPE_MEM,
    TILE_TYPE_EMPTY
  };

  TileType tileType(const CoreIR::Instance& tile) {
    return TILE_TYPE_IO_16;
  }

  class CGRATile {
  public:
    TileType tp;
    int tileNumber;
    TileCoordinates coordinates;
  };

  CGRATile peTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_PE, tileNumber, loc};
  }

  CGRATile memTile(const int tileNumber, TileCoordinates loc) {
    return {TILE_TYPE_MEM, tileNumber, loc};
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

    std::vector<std::vector<CGRATile> > tileRows;
    std::set<TileCoordinates> occupiedTiles;

  public:
    CGRA(const int pe_grid_len_) : pe_grid_len(pe_grid_len_) {
      // Build the cgra grid
      int grid_len = pe_grid_len + 4;

      int start_tile_no = 2;
      vector<CGRATile> topIORow;
      topIORow.push_back(emptyTile(-1, {0, 0}));
      topIORow.push_back(emptyTile(-1, {0, 1}));

      for (; start_tile_no < pe_grid_len + 2; start_tile_no++) {
	topIORow.push_back(io1Tile(start_tile_no, {0, start_tile_no}));
      }

      topIORow.push_back(emptyTile(-1, {0, start_tile_no}));
      topIORow.push_back(emptyTile(-1, {0, start_tile_no}));
      
      tileRows.push_back(topIORow);

      assert(start_tile_no == 18);

      // Build next 16 IO row
      vector<CGRATile> sndTileRow;
      sndTileRow.push_back(emptyTile(-1, {1, 0}));
      sndTileRow.push_back(emptyTile(-1, {1, 1}));

      sndTileRow.push_back(io16Tile(start_tile_no, {1, 2}));
      start_tile_no++;
      for (int i = 0; i < pe_grid_len + 1; i++) {
	sndTileRow.push_back(emptyTile(-1, {1, i + 3}));
      }

      tileRows.push_back(sndTileRow);

      assert(start_tile_no == 19);

      // Construct 3rd row: 1bit, 16bit, <pe / mem grid>, 16 bit, 1bit
      vector<CGRATile> thirdRow;
      thirdRow.push_back(io1Tile(start_tile_no, {2, 0}));
      start_tile_no++;

      thirdRow.push_back(io16Tile(start_tile_no, {2, 1}));
      start_tile_no++;

      for (int i = 0; i < pe_grid_len; i++) {
	if ((i % 4) == 3) {
	  thirdRow.push_back(memTile(start_tile_no, {2, i + 2}));
	} else {
	   thirdRow.push_back(peTile(start_tile_no, {2, i + 2}));
	}
	start_tile_no++;
      }
      
      thirdRow.push_back(io16Tile(start_tile_no, {2, pe_grid_len + 2}));
      start_tile_no++;

      thirdRow.push_back(io1Tile(start_tile_no, {2, pe_grid_len + 3}));
      start_tile_no++;

      tileRows.push_back(thirdRow);

      for (int row = 3; row < pe_grid_len + 2; row++) {
      	vector<CGRATile> tileRow;
	tileRow.push_back(io1Tile(start_tile_no, {row, 0}));
	start_tile_no++;

	tileRow.push_back(io1Tile(-1, {row, 1}));

	for (int i = 0; i < pe_grid_len; i++) {
	  if ((i % 4) == 3) {
	    tileRow.push_back(memTile(start_tile_no, {row, i + 2}));
	  } else {
	    tileRow.push_back(peTile(start_tile_no, {row, i + 2}));
	  }
	  start_tile_no++;
	}

	tileRow.push_back(io1Tile(start_tile_no, {row, pe_grid_len + 2}));
	start_tile_no++;
	
	tileRow.push_back(emptyTile(-1, {row, pe_grid_len + 3}));

      	tileRows.push_back(tileRow);
      }
      
      printPlacement({});
      unsigned rowZeroSize = tileRows[0].size();
      for (auto r : tileRows) {
	assert(rowZeroSize == r.size());
      }

    }

    void setOccupied(const TileCoordinates coords) {
      occupiedTiles.insert(coords);
    }

    bool unoccupied(const int tileRow, const int tileCol) const {
      return !elem({tileRow, tileCol}, occupiedTiles);
    }

    std::vector<TileCoordinates> unplacedTilesOfType(const TileType& tp) const {
      vector<TileCoordinates> tiles;

      for (unsigned i = 0; i < tileRows.size(); i++) {
	auto& tileRow = tileRows[i];

	for (unsigned j = 0; j < tileRow.size(); j++) {

	  CGRATile tile = tileRow[j];
  	  if ((tile.tp == tp) && unoccupied(i, j)) {
  	    tiles.push_back(tile.coordinates);
  	  }

  	}
      }

      return tiles;
    }

    int getPEGridLen() const {
      return pe_grid_len;
    }

    CGRAWire wireFor(const std::map<CoreIR::Instance*, TileCoordinates>& placement, CoreIR::Select* val) {
      Wireable* w = val->getParent();
      assert(isa<Instance>(w));

      Instance* node = cast<Instance>(w);
      cout << "Source node = " << node->toString() << endl;
      TileCoordinates t = map_find(node, placement);

      cout << "coordinates = " << t << endl;

      CGRAWireType tp = CGRA_WIRE_GLOBAL;
      // Assumes we are dealing with an array
      int width = 0; //cast<ArrayType>(val->getType())->getWidth();
      int side = 0;
      int track = 0;
      return {t, tp, width, side, track};
    }

    void printPlacement(const std::map<CoreIR::Instance*, TileCoordinates>& placement) const {
      for (unsigned i = 0; i < tileRows.size(); i++) {
	auto& gridRow = tileRows[i];
	for (unsigned j = 0; j < gridRow.size(); j++) {
	  TileCoordinates coords = {(int) i, (int) j};
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
  };


  bool sameTile(const TileCoordinates& a, const TileCoordinates b) {
    return (a.first == b.first) && (a.second == b.second);
  }

  void routeApplication(ModuleDef& def, const std::map<Instance*, TileCoordinates>& placement, CGRA& cgra) {
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
      cout << "Source coords = " << srcWire.location << endl;
      CGRAWire dstWire = cgra.wireFor(placement, cast<Select>(dst));
      cout << "Dest coords   = " << dstWire.location << endl;
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
	cout << "Tile type = " << tileType << endl;
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

    CGRA cgra(16);
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
