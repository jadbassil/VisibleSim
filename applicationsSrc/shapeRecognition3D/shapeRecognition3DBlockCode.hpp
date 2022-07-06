#ifndef ShapeRecognition3DBlockCode_H_
#define ShapeRecognition3DBlockCode_H_

#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include <queue>
#include "messages.hpp"


static const int SAMPLE_MSG_ID = 1000;

using namespace BlinkyBlocks;

class Box {
public:
    Cell3DPosition base;
    Cell3DPosition corner;
    int w, h, d ;

    Box() {
        base = Cell3DPosition();
        w = h = d = -1;
    };
    Box(Cell3DPosition _base, int _w, int _h, int _d): base(_base), w(_w), h(_h), d(_d) {};
    Box(Cell3DPosition _base, Cell3DPosition _corner): base(_base), corner(_corner){};

    bool operator<(const Box& r) const {
        return base < r.base;
    }

    bool operator==(const Box& r) const {
        return r.base == base and d == r.d and w == r.w and h == r.h;
    }


    static bool isInBox(Box &box, Cell3DPosition &pos)  {
        if(pos.pt[0] < box.base.pt[0] or pos.pt[0] > box.corner.pt[0]) return  false;
        if(pos.pt[1] < box.base.pt[1] or pos.pt[1] > box.corner.pt[1] ) return false;
        if(pos.pt[2] < box.base.pt[2] or pos.pt[2] > box.corner.pt[2]) return  false;

        return  true;
    }

};



class ShapeRecognition3DBlockCode : public BlinkyBlocksBlockCode {
private:


public :
    BlinkyBlocksBlock *module;
    int w{-1}; //width
    int h{-1}; //height
    int d{-1}; //depth
    int rightD{-1}, leftD{-1};
    int rightH{-1}, leftH{-1};
    queue<P2PNetworkInterface*> waitingCheckD;
    int nbWaitingNotifyD{0};
    Box myBox;
    static int c;
    explicit ShapeRecognition3DBlockCode(BlinkyBlocksBlock *host);
    ~ShapeRecognition3DBlockCode() override {};

    /**
     * This function is called on startup of the blockCode, it can be used to perform initial
     *  configuration of the host or this instance of the program.
     * @note this can be thought of as the main function of the module
     **/
    void startup() override;

    void setMyBox();

    static void colorBox(Box &box);

    int sendHandleableMessage(HandleableMessage* msg, P2PNetworkInterface* dest);

    bool isFrontmost() const;
    bool isBackmost() const;
    bool isTopmost() const;
    bool isBottommost() const;
    bool isLeftmost() const;
    bool isRightmost() const;

    /**
     * @brief Handler for all events received by the host block
     * @param pev pointer to the received event
     */
    void processLocalEvent(EventPtr pev) override;


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
        return (new ShapeRecognition3DBlockCode((BlinkyBlocksBlock*)host));
    };
/*****************************************************************************/
};

#endif /* ShapeRecognition3DBlockCode_H_ */
