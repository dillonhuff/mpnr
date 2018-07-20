#include "coreir.h"
#include "coreir/libs/cgralib.h"
#include "coreir/libs/commonlib.h"

#include "catch.hpp"

using namespace CoreIR;
using namespace std;

namespace mpnr {

  class CGRA {
    int pe_grid_len;

  public:
    CGRA(const int pe_grid_len_) : pe_grid_len(pe_grid_len_) {}

    int getPEGridLen() const {
      return pe_grid_len;
    }
  };


  typedef std::pair<int, int> TileCoordinates;

  std::map<Instance*, TileCoordinates> placeApplication(ModuleDef& def, CGRA& cgra) {
    return {};
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
    map<Instance*, pair<int, int> > tilePlacement =
      placeApplication(*def, cgra);

    REQUIRE(tilePlacement.size() == 2);

    deleteContext(c);
  }
}
