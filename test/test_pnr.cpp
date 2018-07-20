#include "coreir.h"
#include "coreir/libs/cgralib.h"
#include "coreir/libs/commonlib.h"

#include "catch.hpp"

using namespace CoreIR;
using namespace std;

namespace mpnr {

  class TileCoordinates {
  public:
    int first;
    int second;
  };

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

      printPlacement({});
      unsigned rowZeroSize = tileRows[0].size();
      for (auto r : tileRows) {
	assert(rowZeroSize == r.size());
      }
      
      // for (int row = 0; row < grid_len; row++) {
      // 	vector<CGRATile> tileRow;
      // 	for (int col = 0; col < grid_len; col++) {
      // 	  tileRow.push_back({TILE_TYPE_EMPTY, {row, col}});
      // 	}

      // 	tileRows.push_back(tileRow);
      // }
    }

    void setOccupied(const TileCoordinates coords) {
      occupiedTiles.insert(coords);
    }

    bool unoccupied(const int tileRow, const int tileCol) const {
      return !elem({tileRow, tileCol}, occupiedTiles);
    }

    std::vector<TileCoordinates> unplacedTilesOfType(const TileType& tp) const {
      vector<TileCoordinates> tiles;
      //for (auto row : tileRows) {
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

    

    deleteContext(c);
  }
}
