#include "csgParser.h"
#include <algorithm>
#include <cassert>

enum KEYWORDS {KW_UNION,KW_INTERSECTION,KW_DIFFERENCE,KW_TRANSLATE,KW_ROTATE,KW_SCALE,KW_CUBE,KW_CYLINDER,KW_SPHERE,KW_TORUS,KW_COLOR,KW_AFFECT,KW_MODULE};
const size_t CSGParser::keywordsCount = 13;
const string CSGParser::keywords[keywordsCount] = {"union","intersection","difference","translate","rotate","scale","cube","cylinder","sphere","torus","color","=","module"};
static const std::string whitespaces (" \t\f\v\n\r");

size_t CSGParser::readCubeParameters(const string &line, Vector3D &v,bool &center) {
    size_t endKeyword = line.length();
    size_t beginKeyword = readVector(line,0, endKeyword, v);
    beginKeyword = line.find_first_of(',', beginKeyword)+1;
    if (beginKeyword!=string::npos && beginKeyword<endKeyword) {
        center = (line.find("true",beginKeyword)!=string::npos);
    } else {
        center = false; // default value it note precised
    }
    return endKeyword;
}

size_t CSGParser::readCylinderParameters(const string &line, double &height, double &radiusBase, double &radiusTop, bool &center) {
    size_t endKeyword = line.length();
    size_t exprPos = line.find_first_of(',',0);
    string expr = line.substr(0, exprPos);
    height = readExpression(expr);
    size_t beginKeyword = exprPos+1;
    exprPos = line.find_first_of(',',beginKeyword);
    expr = line.substr(beginKeyword, exprPos-beginKeyword);
    radiusBase = readExpression(expr);
    beginKeyword = exprPos+1;
    exprPos = line.find_first_of(',',beginKeyword);
    expr = line.substr(beginKeyword, exprPos-beginKeyword);
    if (exprPos!=string::npos && exprPos<endKeyword) {
        radiusTop = readExpression(expr);
        beginKeyword = exprPos + 1;
        center = (line.find("true",beginKeyword)!=string::npos);
    } else { // default radiusTop=radiusBase
        radiusTop=radiusBase;
        center=false;
    }
    return endKeyword;
}

size_t CSGParser::readTorusParameters(const string &line, double &radius1, double &radius2) {
    size_t endKeyword = line.length();
    size_t exprPos = line.find_first_of(',',0);
    string expr = line.substr(0, exprPos);
    radius1 = readExpression(expr);
    size_t beginKeyword = exprPos+1;
    expr = line.substr(beginKeyword);
    radius2 = readExpression(expr);
    return endKeyword;
}


size_t CSGParser::readVector(const string &line, size_t pos1, size_t pos2, Vector3D &v) {
    size_t beginKeyword = line.find_first_of('[', pos1)+1;
    size_t endKeyword = line.find_last_of(']', pos2)-1;
    size_t res=endKeyword+1;
    assert(beginKeyword!=string::npos && endKeyword!=string::npos && endKeyword>beginKeyword);
    string parameters = line.substr(beginKeyword, endKeyword-beginKeyword+1);
    endKeyword = parameters.find_first_of(',');
    string expr = parameters.substr(0, endKeyword);
    v.pt[0] = readExpression(expr);
    beginKeyword = endKeyword+1;
    endKeyword = parameters.find_first_of(',',beginKeyword);
    expr = parameters.substr(beginKeyword, endKeyword-beginKeyword);
    v.pt[1] = readExpression(expr);
    beginKeyword = endKeyword+1;
    endKeyword = parameters.length();
    expr = parameters.substr(beginKeyword, endKeyword-beginKeyword);
    v.pt[2] = readExpression(expr);
    return res;
}

void CSGParser::updateVariable(const string &instr) {
    size_t posEqual = instr.find_first_of('=');
    size_t endInstr = instr.find_last_not_of(whitespaces,posEqual-1);
    string id = instr.substr(0,endInstr+1);
    string expr = instr.substr(posEqual+1);
    double val = readExpression(expr);

    if (variables.find(id)!=variables.end()) { // exists
        variables.find(id)->second = val;
    } else { // create
        variables.insert(pair<string,double>(id,val));
        mu_p.DefineVar(id, &(variables.find(id)->second));
    }
}

std::size_t CSGParser::createModule(const string &line, size_t initialPos) {
    //cout << "createModule: '" << line.substr(initialPos,10) << "'" << endl;
    // extract moduleId
    size_t posInstr=line.find_first_not_of(whitespaces,initialPos);
    size_t endInstr=line.find_first_of('(',posInstr+1);
    string moduleId =line.substr(posInstr,endInstr-posInstr);
    //cout << "moduleId=" << moduleId << endl;
    // extract parameters
    string moduleParam;
    posInstr = extractParametersString(line,endInstr,moduleParam);
    //cout << "moduleParam=" << moduleParam << endl;
    // extract code
    string moduleCode;
    posInstr = extractChildrenString(line,posInstr,moduleCode);
    //cout << "moduleInstr=" << moduleCode << endl;
    // add module
    auto *pm = new CSGParserModule(moduleCode);
    // add parameters
    // first parameters
    size_t pos=0,posId=0,endId;
    while ((pos=moduleParam.find_first_of(',',pos))!=string::npos) {
        posId = moduleParam.find_first_not_of(whitespaces,posId);
        endId = moduleParam.find_last_not_of(whitespaces,pos-1);
        pm->addParamId(moduleParam.substr(posId,endId-posId+1));
        pos++;
    }
    // last parameters
    if ((pos=moduleParam.find_last_of(','))!=string::npos) {
        posId = moduleParam.find_first_not_of(whitespaces,pos+1);
        endId = moduleParam.find_last_not_of(whitespaces);
        pm->addParamId(moduleParam.substr(posId,endId-posId+1));
    }

    modules.insert(pair<string,CSGParserModule*>(moduleId,pm));
    return posInstr;
}

double CSGParser::readExpression(const string &expr) {
    try {
        mu_p.SetExpr(expr);
        return mu_p.Eval();
    } catch (mu::Parser::exception_type &e) {
        cout << e.GetMsg() << endl;
    }
    return 0;
}

std::size_t CSGParser::extractParametersString(const string &line, size_t pos, string &str) {
    pos = line.find_first_of('(', pos);
    size_t beginPart=pos+1;
    if (pos==string::npos) return string::npos;
    size_t n=1;
    while (n!=0 && pos!=string::npos) {
        pos = line.find_first_of("()", pos+1);
        if (pos!=string::npos) {
            if (line[pos]=='(') {
                n++;
            } else {
                n--;
            }
        }
    }
    if (n!=0) return string::npos;
    str = line.substr(beginPart,pos-beginPart);
    return pos+1;
}

std::size_t CSGParser::extractChildrenString(const string &line, size_t pos, string &str) {
    pos = line.find_first_of('{', pos);
    size_t beginPart=pos+1;
    if (pos==string::npos) return string::npos;
    size_t n=1;
    while (n!=0 && pos!=string::npos) {
        pos = line.find_first_of("{}", pos+1);
        if (pos!=string::npos) {
            if (line[pos]=='{') {
                n++;
            } else {
                n--;
            }
        }
    }
    if (n!=0) return string::npos;
    str = line.substr(beginPart,pos-beginPart-1);
    return pos+1;
}

size_t CSGParser::readKeyword(const string &line, size_t initialPos, size_t &keywordId) {
    //size_t beginKeyword = line.find_first_not_of(" );\t\n", initialPos);
    size_t beginKeyword = line.find_first_not_of(whitespaces, initialPos);
    if (beginKeyword == string::npos) return string::npos;
    //size_t endKeyword = line.find_first_of(" =(\t", beginKeyword);
    size_t endKeyword = line.find_first_of("=(", beginKeyword);
    if (line[endKeyword] == '=') { // special case of declaration and initialization of a variable
        keywordId = KW_AFFECT;
        return beginKeyword;
    }

    string keyword = line.substr(beginKeyword, endKeyword-beginKeyword);
    keywordId = 0;
    while (keywordId<keywordsCount && keyword.find(keywords[keywordId])== string::npos) {
        keywordId++;
    }
    if (keywordId==KW_MODULE) {  // special case Module
        endKeyword = line.find_first_of(whitespaces, beginKeyword);
        endKeyword = line.find_first_not_of(whitespaces, endKeyword);
    } else if (keywordId==keywordsCount) { // try module function call
        auto m_it = modules.begin();
        while (m_it!=modules.end() && keyword!=m_it->first){
            m_it++;
            keywordId++;
        }
        if (m_it==modules.end()) keywordId= reinterpret_cast<size_t>(numeric_limits<size_t>::max); // error keyword not found
    }
    return endKeyword;
}

CSGNode *CSGParser::parseCSG(const string &str, size_t &pos) {
    CSGNode *node = nullptr;
    size_t keywordId;

    //cout << "parseCSG(" << str << "," << pos << ")" << endl;
    while ((pos = readKeyword(str, pos, keywordId)) != string::npos) {
        //cout << "KeywordId=" << keywordId << "  pos=" << pos << "|" << str.substr(std::max(0,int(pos-2)),5) << endl;
        switch (keywordId) {
            case KW_UNION: { // union
                string subline;
                pos = extractParametersString(str, pos, subline);
                pos = extractChildrenString(str, pos, subline);
                node = new CSGUnion();
#ifdef DEBUG_CSG
                cout << "union() " << endl;
#endif
                size_t subpos = 0;
                CSGNode *child;
                while (subpos<subline.length()) {
                    child = parseCSG(subline, subpos);
                    if (child!=nullptr) node->addChild(child);
                }
                return node;
            }
                break;
            case KW_INTERSECTION: { // intersection
                string subline;
                pos = extractParametersString(str, pos, subline);
                pos = extractChildrenString(str, pos, subline);
                node = new CSGIntersection();
#ifdef DEBUG_CSG
                cout << "intersection() " << endl;
#endif
                size_t subpos = 0;
                CSGNode *child;
                while (subpos<subline.length()) {
                    child = parseCSG(subline, subpos);
                    if (child!=nullptr) node->addChild(child);
                }
                return node;
            }
                break;
            case KW_DIFFERENCE: { //difference
                string subline;
                pos = extractParametersString(str, pos, subline);
                pos = extractChildrenString(str, pos, subline);
                node = new CSGDifference();
#ifdef DEBUG_CSG
                cout << "difference() " << endl;
#endif
                size_t subpos = 0;
                CSGNode *child;
                while (subpos<subline.length()) {
                    child = parseCSG(subline, subpos);
                    if (child!=nullptr) node->addChild(child);
                }
                return node;
            }
                break;
            case KW_TRANSLATE: { //ŧranslate
                Vector3D v;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readVector(subline, 0, subline.length(), v);
#ifdef DEBUG_CSG
                cout << "translate([" << v[0] << "," << v[1] << "," << v[2] << "]) " << endl;
#endif
                node = new CSGTranslate(v);
                CSGNode *child = parseCSG(str, pos);
                node->addChild(child);
                return node;
            }
                break;
            case KW_ROTATE: { //rotate
                Vector3D v;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readVector(subline, 0, subline.length(), v);
#ifdef DEBUG_CSG
                cout << "rotate([" << v[0] << "," << v[1] << "," << v[2] << "]) " << endl;
#endif
                node = new CSGRotate(v);
                CSGNode *child = parseCSG(str, pos);
                node->addChild(child);
                return node;
            }
                break;
            case KW_SCALE: { //scale
                Vector3D v;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readVector(subline, 0, subline.length(), v);
#ifdef DEBUG_CSG
                cout << "scale([" << v[0] << "," << v[1] << "," << v[2] << "]) " << endl;
#endif
                node = new CSGScale(v);
                CSGNode *child = parseCSG(str, pos);
                node->addChild(child);
                return node;
            }
                break;
            case KW_CUBE: { //cube
                Vector3D v;
                bool center;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readCubeParameters(subline, v, center);
#ifdef DEBUG_CSG
                cout << "cube([" << v[0] << "," << v[1] << "," << v[2] << "]," << (center?"true":"false") << "); " << endl;
#endif
                node = new CSGCube(v, center);
                pos = str.find(';',pos);
                return node;
            }
                break;
            case KW_CYLINDER: { //cylinder
                double height, baseRadius, topRadius;
                bool center;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readCylinderParameters(subline, height, baseRadius, topRadius, center);
#ifdef DEBUG_CSG
                cout << "cylinder(" << height << "," << baseRadius << "," << topRadius << "," << (center?"true":"false") << "); " << endl;
#endif
                if (baseRadius == topRadius) {
                    node = new CSGCylinder(height, baseRadius, center);
                } else {
                    node = new CSGCone(height, baseRadius, topRadius, center);
                }
                pos = str.find(';',pos);
                return node;
            }
                break;
            case KW_SPHERE: { //sphere
                string subline;
                pos = extractParametersString(str, pos, subline);
                double radius = readExpression(subline);
#ifdef DEBUG_CSG
                cout << "sphere(r=" << radius << "); " << endl;
#endif
                node = new CSGSphere(radius);
                pos = str.find(';',pos);
                return node;
            }
                break;
            case KW_TORUS: { //sphere
                double radius1, radius2;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readTorusParameters(subline, radius1,radius2);
#ifdef DEBUG_CSG
                cout << "torus(" << radius1 << "," << radius2 << "); " << endl;
#endif
                node = new CSGTorus(radius1,radius2);
                pos = str.find(';',pos);
                return node;
            }
                break;
            case KW_COLOR: { // color
                Vector3D v;
                string subline;
                pos = extractParametersString(str, pos, subline);
                readVector(subline, 0, subline.length(), v);
#ifdef DEBUG_CSG
                cout << "color([" << v[0] << ',' << v[1] << ',' << v[2] << "])" << endl;
#endif
                node = new CSGColor(v);
                CSGNode *child = parseCSG(str, pos);
                node->addChild(child);
                return node;
            }
                break;
            case KW_AFFECT: { // = create id and affectation
                string subline;
                size_t endInstr = str.find_first_of(';',pos);
                subline = str.substr(pos,endInstr-pos);
#ifdef DEBUG_CSG
                cout << "affectation:" << subline << endl;
#endif
                updateVariable(subline);
                pos = endInstr+1;
            }
            break;
            case KW_MODULE: { // module
                pos = createModule(str,pos);
#ifdef DEBUG_CSG
                cout << "New module:" << pos << endl;
#endif

            }
            break;
            default: {
                if (keywordId!=reinterpret_cast<size_t>(numeric_limits<size_t>::max)) {
                    auto m_it = modules.begin();
                    size_t i=keywordId-keywordsCount;
                    while (m_it!=modules.end() && i) {
                        m_it++;
                        i--;
                    }
#ifdef DEBUG_CSG
                    cout << "Call module:" << m_it->first << endl;
#endif
                    // add parameter in variables ! and init
                    string moduleParam;
                    extractParametersString(str,pos,moduleParam);
                    //cout << "moduleParam:" << moduleParam << endl;
                    // first parameters
                    size_t posP=0,posId=0,endId;
                    i=0;
                    double v=0;
                    while ((posP=moduleParam.find_first_of(',',posP))!=string::npos) {
                        posId = moduleParam.find_first_not_of(whitespaces,posId);
                        endId = moduleParam.find_last_not_of(whitespaces,posP-1);
                        v = readExpression(moduleParam.substr(posId,endId-posId+1));
                        m_it->second->addVariable(i,v,mu_p);
                        i++;
                        posP++;
                    }
                    // last parameters
                    if ((posP=moduleParam.find_last_of(','))!=string::npos) {
                        posId = moduleParam.find_first_not_of(whitespaces,posP+1);
                        endId = moduleParam.find_last_not_of(whitespaces);
                        v = readExpression(moduleParam.substr(posId,endId-posId+1));
                        m_it->second->addVariable(i,v,mu_p);
                    }

                    node = parseCSG(m_it->second->moduleBody);
                    m_it->second->removeVariables(mu_p);
                    pos = str.find_first_of(';',pos)+1;
                    return node;
                } else {
#ifdef DEBUG_CSG
                    cout << "Default ?" << pos << ":" << str[pos] << endl;
#endif
                }
            }
        }
    }
    return nullptr;
}
