// Meta-module generator for 3D-catom modular robots.
//
// Runs a genetic algorithm to discover the smallest meta-module shape
// satisfying the framework of Lamiroy, Bassil & Piranda
// (Connectivity, Exclusivity, Axis-contacts, Center existence) and
// admitting a Transfer operation in every ±basis direction. Then it
// boots the standard Catoms3D simulator so the shape can be visualised.

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "robots/catoms3D/catoms3DSimulator.h"

#include "metaModuleCore.h"
#include "metaModuleGenBlockCode.h"

using namespace Catoms3D;
using namespace metaModuleGen;

namespace {

void printPlan(const TransferPlan& p, const std::string& dir) {
    std::cout << "    " << dir
              << "  feasible=" << (p.feasible ? "yes" : "no")
              << "  bridges=" << p.nBridges
              << "  setup=" << p.nSetupBridgeMoves
              << "  perModule=" << p.nTransferMovesPerModule
              << "  clear=" << p.nClearBridgeMoves
              << "  total=" << p.totalMoves(1) << "\n";
}

// Tile the meta-module into an nA x nB x nC block of meta-cells and write a
// VisibleSim config. A <metaModuleInfo> element records the basis, the m=0
// meta-module's world cells + center, and the tiling counts so the interactive
// block code can identify meta-modules and plan operations.
bool writeStructureXml(const std::string& path, const MetaModule& mm, int tiles) {
    const int nA = tiles, nB = tiles, nC = tiles;  // tiled block of meta-modules
    // Symmetric margin: centers the structure in the grid (so the simulator's
    // default camera frames it) and leaves room for modules moved into the
    // empty neighbor slots during an operation.
    const int margin = 8;

    auto shifted = [&](const Cell3DPosition& c, int a, int b, int cc) {
        return Cell3DPosition(
            c[0] + a*mm.basis.X[0] + b*mm.basis.Y[0] + cc*mm.basis.Z[0],
            c[1] + a*mm.basis.X[1] + b*mm.basis.Y[1] + cc*mm.basis.Z[1],
            c[2] + a*mm.basis.X[2] + b*mm.basis.Y[2] + cc*mm.basis.Z[2]);
    };

    // Bounding box over the whole tiled block (to compute a positive offset).
    int mnx=INT_MAX,mny=INT_MAX,mnz=INT_MAX,mxx=INT_MIN,mxy=INT_MIN,mxz=INT_MIN;
    for (int a=0;a<nA;a++) for (int b=0;b<nB;b++) for (int cc=0;cc<nC;cc++)
        for (const auto& c : mm.cells) {
            Cell3DPosition p = shifted(c,a,b,cc);
            mnx=std::min<int>(mnx,p[0]); mny=std::min<int>(mny,p[1]); mnz=std::min<int>(mnz,p[2]);
            mxx=std::max<int>(mxx,p[0]); mxy=std::max<int>(mxy,p[1]); mxz=std::max<int>(mxz,p[2]);
        }
    int ox = margin - mnx, oy = margin - mny, oz = margin - mnz;
    // FCC adjacency is z-parity dependent (even/odd rows have different
    // neighbor sets), so the translation into the positive octant MUST keep
    // every cell's z-parity — otherwise a structure that is connected at the
    // origin renders as disconnected clumps. Keep all offsets even.
    if (ox & 1) ++ox;
    if (oy & 1) ++oy;
    if (oz & 1) ++oz;
    int gx = mxx+ox+margin+1, gy = mxy+oy+margin+1, gz = mxz+oz+margin+1;

    std::ofstream f(path);
    if (!f) return false;
    f << "<?xml version=\"1.0\" standalone=\"no\" ?>\n";
    f << "<world gridSize=\"" << gx << "," << gy << "," << gz
      << "\" windowSize=\"1280,720\">\n";
    // No <camera>/<spotlight>: let the simulator auto-frame the (centered)
    // structure in world coordinates. A hand-written camera target is in world
    // units (cells x gridScale), which is easy to get wrong and leaves the
    // structure off-screen.
    f << "  <blockList color=\"128,128,128\" blockSize=\"10,10,10\">\n";
    // A small palette so adjacent meta-modules are visually distinct.
    static const char* pal[4] = {"200,200,100","120,180,220","180,140,200","150,210,150"};
    for (int a=0;a<nA;a++) for (int b=0;b<nB;b++) for (int cc=0;cc<nC;cc++) {
        const char* col = pal[((a+b+cc)%4)];
        for (const auto& c : mm.cells) {
            Cell3DPosition p = shifted(c,a,b,cc);
            f << "    <block position=\"" << (p[0]+ox) << "," << (p[1]+oy) << ","
              << (p[2]+oz) << "\" orientation=\"0\" color=\"" << col << "\"/>\n";
        }
    }
    f << "  </blockList>\n";

    // Machine-readable meta-module description for the interactive block code.
    auto vec = [](const Cell3DPosition& v){ return std::to_string(v[0])+","+
        std::to_string(v[1])+","+std::to_string(v[2]); };
    f << "  <metaModuleInfo"
      << " X=\"" << vec(mm.basis.X) << "\""
      << " Y=\"" << vec(mm.basis.Y) << "\""
      << " Z=\"" << vec(mm.basis.Z) << "\""
      << " center=\"" << (mm.center[0]+ox) << "," << (mm.center[1]+oy) << ","
                      << (mm.center[2]+oz) << "\""
      << " nA=\"" << nA << "\" nB=\"" << nB << "\" nC=\"" << nC << "\""
      << " cells=\"";
    bool first = true;
    for (const auto& c : mm.cells) {
        if (!first) f << ";";
        f << (c[0]+ox) << "," << (c[1]+oy) << "," << (c[2]+oz);
        first = false;
    }
    f << "\"/>\n";
    f << "</world>\n";
    return static_cast<bool>(f);
}

struct CliArgs {
    bool runSim{true};
    int minSize{4};
    int maxSize{30};
    int boundingRadius{4};
    int population{120};
    int generations{200};
    uint32_t seed{42};
    int tiles{2};          // N: export an NxNxN block of meta-modules
    std::string outXml{"metaModule_result.xml"};
    std::string viewXml;   // if set: skip the search, just view this structure
};

CliArgs parseExtra(int argc, char** argv) {
    CliArgs a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        auto eat = [&](const char* k) -> const char* {
            size_t n = std::strlen(k);
            if (s.rfind(k, 0) == 0 && s.size() > n) return argv[i] + n;
            return nullptr;
        };
        if (s == "--no-sim")            a.runSim = false;
        else if (auto v = eat("--min-size=")) a.minSize = std::atoi(v);
        else if (auto v = eat("--max-size=")) a.maxSize = std::atoi(v);
        else if (auto v = eat("--radius="))   a.boundingRadius = std::atoi(v);
        else if (auto v = eat("--pop="))      a.population = std::atoi(v);
        else if (auto v = eat("--gens="))     a.generations = std::atoi(v);
        else if (auto v = eat("--seed="))     a.seed = static_cast<uint32_t>(std::atoll(v));
        else if (auto v = eat("--out-xml="))  a.outXml = v;
        else if (auto v = eat("--tiles="))    a.tiles = std::max(1, std::atoi(v));
        else if (auto v = eat("--view="))     a.viewXml = v;
    }
    return a;
}

// Boot the Catoms3D viewer on an existing structure XML. Strips our custom
// flags so the simulator's getopt does not choke, feeds it -c <xml>, and forces
// a realtime scheduler if the user did not pass -r/-R.
void bootViewer(int argc, char** argv, const std::string& xmlPath) {
    std::cout << "Booting visualiser on " << xmlPath << " ...\n";
    std::vector<std::string> kept;
    kept.reserve(argc + 4);
    kept.emplace_back(argv[0]);
    bool hasSchedFlag = false;   // -r/-R/-t all select a scheduler mode
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s.rfind("--no-sim", 0) == 0 || s.rfind("--min-size=", 0) == 0
            || s.rfind("--max-size=", 0) == 0 || s.rfind("--radius=", 0) == 0
            || s.rfind("--pop=", 0) == 0 || s.rfind("--gens=", 0) == 0
            || s.rfind("--seed=", 0) == 0 || s.rfind("--out-xml=", 0) == 0
            || s.rfind("--view=", 0) == 0) continue;
        if (s == "-r" || s == "-R" || s == "-t") hasSchedFlag = true;
        kept.push_back(s);
    }
    kept.emplace_back("-c");
    kept.emplace_back(xmlPath);
    // Only force the realtime scheduler (so GLUT opens) if the user did not
    // already pick one; -t (terminal) must not be overridden with -r.
    if (!hasSchedFlag) kept.emplace_back("-r");

    std::vector<char*> argv2;
    argv2.reserve(kept.size());
    for (auto& s : kept) argv2.push_back(s.data());
    createSimulator(static_cast<int>(argv2.size()), argv2.data(),
                    MetaModuleGenBlockCode::buildNewBlockCode);
    deleteSimulator();
}

} // namespace

int main(int argc, char** argv) {
    CliArgs cli = parseExtra(argc, argv);

    // --view=PATH: skip the search entirely and just open the interactive
    // viewer on an existing structure XML (must contain <metaModuleInfo>).
    if (!cli.viewXml.empty()) {
        std::cout << "[--view] skipping search; opening " << cli.viewXml << "\n";
        bootViewer(argc, argv, cli.viewXml);
        return 0;
    }

    std::cout << "\033[1;33m"
              << "MetaModuleGen — searching for the smallest valid 3D-catom meta-module"
              << "\033[0m\n";

    GAConfig cfg;
    cfg.minSize         = cli.minSize;
    cfg.maxSize         = cli.maxSize;
    cfg.boundingRadius  = cli.boundingRadius;
    cfg.populationSize  = cli.population;
    cfg.generations     = cli.generations;
    cfg.seed            = cli.seed;

    auto bases = defaultCandidateBases();
    GenerationResult result = runGenerator(bases, cfg);

    if (!result.found) {
        std::cout << "\nNo valid meta-module was found in the search budget.\n";
        std::cout << "Try increasing --max-size, --radius, --pop, or --gens.\n";
        return 1;
    }

    std::cout << "\n\033[1;32m*** Best meta-module ***\033[0m\n";
    std::cout << "basis label : " << result.basisLabel << "\n";
    std::cout << result.best << "\n";
    static const char* labels[6] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    std::cout << "Transfer cost per direction:\n";
    for (int i = 0; i < 6; ++i) printPlan(result.cost.perDirection[i], labels[i]);
    std::cout << "Disassembly / Assembly moves per direction:\n";
    for (int i = 0; i < 6; ++i) {
        std::cout << "    " << labels[i]
                  << "  disasm feasible=" << (result.cost.disassembly[i].feasible ? "yes" : "no")
                  << " moves=" << result.cost.disassembly[i].nMoves
                  << "   asm feasible=" << (result.cost.assembly[i].feasible ? "yes" : "no")
                  << " moves=" << result.cost.assembly[i].nMoves << "\n";
    }
    std::cout << "totals (sum over ±X, ±Y, ±Z):"
              << "  transfer=" << result.cost.totalTransferMoves
              << "  disassembly=" << result.cost.totalDisassemblyMoves
              << "  assembly=" << result.cost.totalAssemblyMoves
              << "  ALL=" << result.cost.totalMoves << "\n";

    bool wrote = writeStructureXml(cli.outXml, result.best, cli.tiles);
    std::cout << (wrote ? "result XML written to: " : "FAILED to write XML at: ")
              << cli.outXml << "\n";

    if (cli.runSim)
        bootViewer(argc, argv, cli.outXml);
    return 0;
}
