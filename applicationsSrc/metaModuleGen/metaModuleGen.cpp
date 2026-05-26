// Meta-module generator for 3D-catom modular robots.
//
// Runs a genetic algorithm to discover the smallest meta-module shape
// satisfying the framework of Lamiroy, Bassil & Piranda
// (Connectivity, Exclusivity, Axis-contacts, Center existence) and
// admitting a Transfer operation in every ±basis direction. Then it
// boots the standard Catoms3D simulator so the shape can be visualised.

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

bool writeShapeXml(const std::string& path, const MetaModule& mm) {
    std::ofstream f(path);
    if (!f) return false;
    // Translate cells so they sit in the positive octant of the grid
    short ox = 5, oy = 5, oz = 2;
    f << "<?xml version=\"1.0\" standalone=\"no\" ?>\n";
    f << "<world gridSize=\"20,20,20\" windowSize=\"1280,720\">\n";
    f << "  <camera target=\"15,15,10\" directionSpherical=\"-30,30,40\" "
         "angle=\"30\" near=\"1\" far=\"600\"/>\n";
    f << "  <blockList color=\"128,128,128\" blockSize=\"10,10,10\">\n";
    for (const auto& c : mm.cells) {
        f << "    <block position=\"" << (c[0] + ox) << ","
                                       << (c[1] + oy) << ","
                                       << (c[2] + oz)
          << "\" orientation=\"0\" color=\"200,200,100\"/>\n";
    }
    Cell3DPosition ctr = mm.center;
    f << "    <!-- center cell at "
      << (ctr[0] + ox) << "," << (ctr[1] + oy) << "," << (ctr[2] + oz)
      << " -->\n";
    f << "  </blockList>\n";
    f << "</world>\n";
    return static_cast<bool>(f);
}

struct CliArgs {
    bool runSim{true};
    int minSize{4};
    int maxSize{30};
    int boundingRadius{3};
    int population{120};
    int generations{200};
    uint32_t seed{42};
    std::string outXml{"metaModule_result.xml"};
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
    }
    return a;
}

} // namespace

int main(int argc, char** argv) {
    CliArgs cli = parseExtra(argc, argv);

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
    std::cout << "Transfer cost per direction:\n";
    static const char* labels[6] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    for (int i = 0; i < 6; ++i) printPlan(result.cost.perDirection[i], labels[i]);
    std::cout << "total transfer moves (sum over ±X, ±Y, ±Z): "
              << result.cost.totalTransferMoves << "\n";

    bool wrote = writeShapeXml(cli.outXml, result.best);
    std::cout << (wrote ? "result XML written to: " : "FAILED to write XML at: ")
              << cli.outXml << "\n";

    if (cli.runSim) {
        std::cout << "Booting visualiser on the discovered shape...\n";

        // Forward only flags the simulator understands; strip ours
        // (--no-sim, --min-size=…, etc.) so getopt does not abort.
        std::vector<std::string> kept;
        kept.reserve(argc + 4);
        kept.emplace_back(argv[0]);
        bool hasRunFlag = false;
        for (int i = 1; i < argc; ++i) {
            std::string s = argv[i];
            if (s.rfind("--no-sim", 0) == 0)    continue;
            if (s.rfind("--min-size=", 0) == 0) continue;
            if (s.rfind("--max-size=", 0) == 0) continue;
            if (s.rfind("--radius=", 0) == 0)   continue;
            if (s.rfind("--pop=", 0) == 0)      continue;
            if (s.rfind("--gens=", 0) == 0)     continue;
            if (s.rfind("--seed=", 0) == 0)     continue;
            if (s.rfind("--out-xml=", 0) == 0)  continue;
            if (s == "-r" || s == "-R") hasRunFlag = true;
            kept.push_back(s);
        }
        // Always feed the generated XML and force realtime scheduler so GLUT
        // gets a chance to open its window.
        kept.emplace_back("-c");
        kept.emplace_back(cli.outXml);
        if (!hasRunFlag) kept.emplace_back("-r");

        std::vector<char*> argv2;
        argv2.reserve(kept.size());
        for (auto& s : kept) argv2.push_back(s.data());
        createSimulator(static_cast<int>(argv2.size()), argv2.data(),
                        MetaModuleGenBlockCode::buildNewBlockCode);
        deleteSimulator();
    }
    return 0;
}
