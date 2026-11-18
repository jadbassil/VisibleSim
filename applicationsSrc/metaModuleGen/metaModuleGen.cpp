// Meta-module generator for 3D-catom modular robots.
//
// Searches for the smallest meta-module shape satisfying the framework of
// Lamiroy, Bassil & Piranda (Connectivity, Exclusivity, Axis-contacts, Center
// existence) and admitting Disassembly, Transfer and Assembly in every
// ±basis direction. Three searches are available (--search=):
//   ga         genetic algorithm (default)
//   enum       exhaustive enumeration, proves the smallest feasible size
//   mapelites  MAP-Elites over a (size x basis) archive
// Then it boots the standard Catoms3D simulator so the shape can be visualised.

#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "robots/catoms3D/catoms3DSimulator.h"

#include "metaModuleCore.h"
#include "metaModuleSearch.h"
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
    std::string search{"ga"};  // ga | enum | mapelites
    int threads{0};            // 0 = all hardware threads
    long iters{20000};         // MAP-Elites children
    int initSamples{500};      // MAP-Elites random seeds
    int batch{64};             // MAP-Elites children per parallel step
    std::string archiveCsv{"mapElites_archive.csv"};
    std::string enumCsv{"enum_feasible.csv"};
    std::string bases;         // comma-separated basis indices (empty = all)
    std::string saveDir;       // if set: one XML per feasible design + index
};

// Flags handled here; they are stripped before argv reaches the simulator.
const char* const kOwnFlags[] = {
    "--no-sim", "--min-size=", "--max-size=", "--radius=", "--pop=",
    "--gens=", "--seed=", "--out-xml=", "--tiles=", "--view=", "--search=",
    "--threads=", "--iters=", "--init=", "--batch=", "--archive-csv=",
    "--enum-csv=", "--bases=", "--save-dir=",
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
        else if (auto v = eat("--search="))   a.search = v;
        else if (auto v = eat("--threads="))  a.threads = std::max(0, std::atoi(v));
        else if (auto v = eat("--iters="))    a.iters = std::max(0L, std::atol(v));
        else if (auto v = eat("--init="))     a.initSamples = std::max(1, std::atoi(v));
        else if (auto v = eat("--batch="))    a.batch = std::max(1, std::atoi(v));
        else if (auto v = eat("--archive-csv=")) a.archiveCsv = v;
        else if (auto v = eat("--enum-csv="))    a.enumCsv = v;
        else if (auto v = eat("--bases="))    a.bases = v;
        else if (auto v = eat("--save-dir=")) a.saveDir = v;
    }
    return a;
}

// Keep only the default bases whose indices appear in `spec` ("0,2,3").
// An empty spec keeps them all.
bool selectBases(const std::string& spec, std::vector<CandidateBasis>& out) {
    auto all = defaultCandidateBases();
    if (spec.empty()) { out = all; return true; }
    std::stringstream ss(spec);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        char* end = nullptr;
        long i = std::strtol(tok.c_str(), &end, 10);
        if (tok.empty() || *end != '\0' || i < 0 || i >= (long)all.size()) {
            std::cerr << "Bad --bases entry '" << tok << "'. Valid indices:\n";
            for (size_t k = 0; k < all.size(); ++k)
                std::cerr << "  " << k << " = " << all[k].label << "\n";
            return false;
        }
        out.push_back(all[i]);
    }
    return !out.empty();
}

std::string vec3(const Cell3DPosition& v, const char* sep = ",") {
    return std::to_string(v[0]) + sep + std::to_string(v[1]) + sep
         + std::to_string(v[2]);
}

// Basis labels may contain spaces or brackets; keep file names portable.
std::string fileSafe(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')
            out.push_back(c);
        else if (!out.empty() && out.back() != '-')
            out.push_back('-');
    }
    while (!out.empty() && out.back() == '-') out.pop_back();
    return out;
}

// Write every feasible design as its own structure XML, plus designs.csv (an
// index with costs, centers and cells) and run.json (the settings used).
bool saveDesigns(const CliArgs& cli, const std::vector<CandidateBasis>& bases,
                 const GenerationResult& result) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::create_directories(cli.saveDir, ec);
    if (ec) {
        std::cerr << "Cannot create --save-dir " << cli.saveDir << ": "
                  << ec.message() << "\n";
        return false;
    }
    std::vector<FoundDesign> designs = result.designs;
    if (designs.empty() && result.found)
        designs.push_back({result.best, result.cost, result.basisLabel});

    const fs::path dir(cli.saveDir);
    std::ofstream index(dir / "designs.csv");
    index << "file,size,basis,total_moves,transfer,disassembly,assembly,center,cells\n";
    const int width = designs.size() >= 100 ? 3 : 2;
    bool ok = static_cast<bool>(index);
    for (size_t i = 0; i < designs.size(); ++i) {
        const FoundDesign& d = designs[i];
        std::string rank = std::to_string(i + 1);
        rank.insert(0, width - std::min<int>(width, rank.size()), '0');
        const std::string name = rank + "_size" + std::to_string(d.mm.size()) + "_"
            + fileSafe(d.basisLabel) + "_" + std::to_string(d.cost.totalMoves)
            + "moves.xml";
        ok &= writeStructureXml((dir / name).string(), d.mm, cli.tiles);
        std::string cells;
        for (const auto& c : d.mm.cells) {
            if (!cells.empty()) cells += ";";
            cells += vec3(c, " ");
        }
        index << name << "," << d.mm.size() << ",\"" << d.basisLabel << "\","
              << d.cost.totalMoves << "," << d.cost.totalTransferMoves << ","
              << d.cost.totalDisassemblyMoves << "," << d.cost.totalAssemblyMoves
              << ",\"" << vec3(d.mm.center, " ") << "\",\"" << cells << "\"\n";
    }

    std::ofstream run(dir / "run.json");
    run << "{\n"
        << "  \"search\": \"" << cli.search << "\",\n"
        << "  \"min_size\": " << cli.minSize << ",\n"
        << "  \"max_size\": " << cli.maxSize << ",\n"
        << "  \"radius\": " << cli.boundingRadius << ",\n"
        << "  \"max_extent\": " << (2 * cli.boundingRadius + 1) << ",\n"
        << "  \"seed\": " << cli.seed << ",\n"
        << "  \"ga_population\": " << cli.population << ",\n"
        << "  \"ga_generations\": " << cli.generations << ",\n"
        << "  \"mapelites_iters\": " << cli.iters << ",\n"
        << "  \"mapelites_init\": " << cli.initSamples << ",\n"
        << "  \"mapelites_batch\": " << cli.batch << ",\n"
        << "  \"threads\": " << cli.threads << ",\n"
        << "  \"tiles\": " << cli.tiles << ",\n"
        << "  \"bases\": [\n";
    for (size_t i = 0; i < bases.size(); ++i) {
        const Basis& b = bases[i].basis;
        run << "    {\"label\": \"" << bases[i].label << "\", \"X\": [" << vec3(b.X)
            << "], \"Y\": [" << vec3(b.Y) << "], \"Z\": [" << vec3(b.Z) << "]}"
            << (i + 1 < bases.size() ? "," : "") << "\n";
    }
    run << "  ],\n"
        << "  \"designs_saved\": " << designs.size() << "\n"
        << "}\n";
    ok &= static_cast<bool>(index) && static_cast<bool>(run);

    std::cout << (ok ? "saved " : "FAILED to save all of ") << designs.size()
              << " design(s) to " << cli.saveDir << "/ (XML per design,"
              << " designs.csv, run.json)\n";
    return ok;
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
        bool own = false;
        for (const char* f : kOwnFlags)
            if (s.rfind(f, 0) == 0) { own = true; break; }
        if (own) continue;
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

    std::vector<CandidateBasis> bases;
    if (!selectBases(cli.bases, bases)) return 2;
    const int maxExtent = 2 * cli.boundingRadius + 1;

    GenerationResult result;
    if (cli.search == "ga") {
        GAConfig cfg;
        cfg.minSize         = cli.minSize;
        cfg.maxSize         = cli.maxSize;
        cfg.boundingRadius  = cli.boundingRadius;
        cfg.populationSize  = cli.population;
        cfg.generations     = cli.generations;
        cfg.seed            = cli.seed;
        result = runGenerator(bases, cfg);
    } else if (cli.search == "enum") {
        EnumConfig cfg;
        cfg.minSize   = cli.minSize;
        cfg.maxSize   = cli.maxSize;
        cfg.maxExtent = maxExtent;
        cfg.threads   = cli.threads;
        cfg.csvPath   = cli.enumCsv;
        result = runEnumeration(bases, cfg);
        if (!cli.enumCsv.empty())
            std::cout << "feasible shapes CSV written to: " << cli.enumCsv << "\n";
    } else if (cli.search == "mapelites") {
        MapElitesConfig cfg;
        cfg.minSize     = cli.minSize;
        cfg.maxSize     = cli.maxSize;
        cfg.maxExtent   = maxExtent;
        cfg.initSamples = cli.initSamples;
        cfg.iterations  = cli.iters;
        cfg.batchSize   = cli.batch;
        cfg.threads     = cli.threads;
        cfg.seed        = cli.seed;
        cfg.archiveCsv  = cli.archiveCsv;
        result = runMapElites(bases, cfg);
    } else {
        std::cerr << "Unknown --search=" << cli.search
                  << " (expected ga, enum or mapelites)\n";
        return 2;
    }

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

    if (!cli.saveDir.empty()) saveDesigns(cli, bases, result);

    if (cli.runSim)
        bootViewer(argc, argv, cli.outXml);
    return 0;
}
