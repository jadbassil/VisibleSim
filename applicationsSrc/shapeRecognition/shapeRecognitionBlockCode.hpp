#ifndef ShapeRecognitionBlockCode_H_
#define ShapeRecognitionBlockCode_H_

#include "robots/catoms3D/catoms3DWorld.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include <queue>
#include <set>

static const int SETL_MSG_ID = 1000;
static const int GETW_MSG_ID = 1001;
static const int CHECKLR_MSG_ID = 1002;
static const int SENDLR_MSG_ID = 1003;
static const int SETW_MSG_ID = 1004;
static const int BROADCASTRECT_MSG_ID = 1005;

using namespace Catoms3D;

class Rectangle {
public:
    Cell3DPosition base;
    int l, w;

    Rectangle() {
        base = Cell3DPosition();
        l = w = -1;
    };
    Rectangle(Cell3DPosition _base, int _l,int _w): base(_base), l(_l), w(_w) {};

    bool operator<(const Rectangle& r) const {
        return (base.pt[0] == r.base.pt[0]) ?  base.pt[1] < r.base.pt[1]:  base.pt[0] < r.base.pt[0];
    }

    bool operator==(const Rectangle& r) const {
        return r.base == base and l == r.l and w == r.w;
    }

    static bool isInRectangle(Rectangle &r, Cell3DPosition &pos)  {
        if(pos.pt[1] < r.base.pt[1] or pos.pt[1] > r.base.pt[1] + r.l - 1) {
            return false;
        }
        if (r.w >= 0) {
            if (pos.pt[0] >= r.base.pt[0] and pos.pt[0] <= r.base.pt[0] + r.w - 1) {
                return true;
            }
        } else {
            if (pos.pt[0] <= r.base.pt[0] and pos.pt[0] >= r.base.pt[0] + r.w + 1) {
                return true;
            }
        }
        return  false;
    }

};

class ShapeRecognitionBlockCode : public Catoms3DBlockCode {
private:
    int l{};
    Catoms3DBlock *module;
    queue<P2PNetworkInterface*> waiting;
    int nbWaitedCheckLR{0};
    int leftL{};
    int rightL{};
    Rectangle myRectangle;
    set<Rectangle> currentShape;
public :
    ShapeRecognitionBlockCode(Catoms3DBlock *host);
    ~ShapeRecognitionBlockCode() {};

    /**
     * This function is called on startup of the blockCode, it can be used to perform initial
     *  configuration of the host or this instance of the program.
     * @note this can be thought of as the main function of the module
     **/
    void startup() override;

    bool isBottom() const;
    bool isTop() const;
    static void colorRectangle(Rectangle &r) ;

    static int c;

    /**
     * @brief Handler for all events received by the host block
     * @param pev pointer to the received event
     */
    void processLocalEvent(EventPtr pev) override;

    /**
     * @brief Callback function executed whenever the module finishes a motion
     */
    void onMotionEnd() override;

    /**
     * @brief Sample message handler for this instance of the blockcode
     * @param _msg Pointer to the message received by the module, requires casting
     * @param sender Connector of the module that has received the message and that is connected to the sender */
    void handleSetLMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleGetWMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleCheckLRMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleSendLRMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleSetWMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleBroadcastRectWMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    bool isInshape(Cell3DPosition &pos);
    /// Advanced blockcode handlers below

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from it.
     * @param config : pointer to the TiXmlDocument representing the configuration file, all information related to VisibleSim's core have already been parsed
     *
     * Called from BuildingBlock constructor, only once.
     */
    void parseUserElements(TiXmlDocument *config) override {}

    /**
     * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from each block config. Has to be overriden in the child class.
     * @param config : pointer to the TiXmlElement representing the block configuration file, all information related to concerned block have already been parsed
     *
     */
    void parseUserBlockElements(TiXmlElement *config) override {}

    /**
     * User-implemented debug function that gets called when a module is selected in the GUI
     */
    void onBlockSelected() override;

    /**
     * User-implemented debug function that gets called when a VS_ASSERT is triggered
     * @note call is made from utils::assert_handler()
     */
    void onAssertTriggered() override;

    /**
     * User-implemented keyboard handler function that gets called when
     *  a key press event could not be caught by openglViewer
     * @param c key that was pressed (see openglViewer.cpp)
     * @param x location of the pointer on the x axis
     * @param y location of the pointer on the y axis
     * @note call is made from GlutContext::keyboardFunc (openglViewer.h)
     */
    void onUserKeyPressed(unsigned char c, int x, int y) override {}

    /**
     * Call by world during GL drawing phase, can be used by a user
     *  to draw custom Gl content into the simulated world
     * @note call is made from World::GlDraw
     */
    void onGlDraw() override {}

    /**
     * @brief This function is called when a module is tapped by the user. Prints a message to the console by default.
     Can be overloaded in the user blockCode
     * @param face face that has been tapped */
    void onTap(int face) override {}


    /**
     * User-implemented keyboard handler function that gets called when
     *  a key press event could not be caught by openglViewer
     * @note call is made from GlutContext::keyboardFunc (openglViewer.h)
     */
    bool parseUserCommandLineArgument(int& argc, char **argv[]) override;

    /**
     * Called by openglviewer during interface drawing phase, can be used by a user
     *  to draw a custom Gl string onto the bottom-left corner of the GUI
     * @note call is made from OpenGlViewer::drawFunc
     * @return a string (can be multi-line with `\n`) to display on the GUI
     */
    string onInterfaceDraw() override;

/*****************************************************************************/
/** needed to associate code to module                                      **/
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new ShapeRecognitionBlockCode((Catoms3DBlock*)host));
    };
/*****************************************************************************/
};

#endif /* ShapeRecognitionBlockCode_H_ */
