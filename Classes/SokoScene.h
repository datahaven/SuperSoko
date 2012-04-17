//
// Game In A Day Project - Super Soko
// Because the world needs another Sokoban clone ...
//
// by Adrian Dale 16/04/2012
// http://www.adriandale.co.uk
//
#ifndef __Soko_SCENE_H__
#define __Soko_SCENE_H__

#include "cocos2d.h"
#include <sstream>
#include <iomanip>
#include "SimpleAudioEngine.h"

// Yes, probably not good putting namespace declarations in include file
using namespace cocos2d;
using namespace CocosDenshion;
using namespace std;

class Player : public CCNode
{
public:
	virtual bool init();

	// implement the "static node()" method manually
    LAYER_NODE_FUNC(Player);

	CCSprite *mpSprite;
	bool mIsMoving;
	int mPushCount;
	int mMoveCount;
	int mFacing;
};

class Soko : public CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

	~Soko();

    // there's no 'id' in cpp, so we recommand to return the exactly class pointer
    static CCScene* scene();
    
    // a selector callback
    virtual void menuCloseCallback(CCObject* pSender);
	virtual void menuRestartCallback(CCObject* pSender);

    // implement the "static node()" method manually
    LAYER_NODE_FUNC(Soko);

	void spriteMoveFinished(CCNode *sender); 

protected:
	// Give our scene a tile map layer
	CC_SYNTHESIZE(CCTMXTiledMap *, mpTileMap, TileMap);
	CCSpriteBatchNode *mpSpriteBatchNode;
	void GameTick(ccTime dt);
	bool InitialiseLevel(char *LevelName);
	Player *mpPlayer;
	bool MovePlayer(int dx, int dy);
	CCSprite *mpBackground;
	CCSprite *mpLogo;
	CCLabelBMFont* mpLevelLabel;
	CCLabelBMFont* mpMovesLabel;
	CCLabelBMFont* mpPushesLabel;
	CCLabelBMFont* mpLevelCompleteLabel;
	int mLevel;
	bool mGameRunning;
};

#endif  // __Soko_SCENE_H__