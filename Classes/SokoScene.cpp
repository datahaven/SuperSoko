//
// Game In A Day Project - Super Soko
// Because the world needs another Sokoban clone ...
//
// by Adrian Dale 16/04/2012
// http://www.adriandale.co.uk
// 
#include "SokoScene.h"

CCScene* Soko::scene()
{
    CCScene * scene = NULL;
    do 
    {
        // 'scene' is an autorelease object
        scene = CCScene::node();
        CC_BREAK_IF(! scene);

        // 'layer' is an autorelease object
        Soko *layer = Soko::node();
        CC_BREAK_IF(! layer);

        // add layer as a child to scene
        scene->addChild(layer);
    } while (0);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Soko::init()
{
    bool bRet = false;
    do 
    {
        //////////////////////////////////////////////////////////////////////////
        // super init first
        //////////////////////////////////////////////////////////////////////////

        CC_BREAK_IF(! CCLayer::init());

        //////////////////////////////////////////////////////////////////////////
        // add your codes below...
        //////////////////////////////////////////////////////////////////////////

        // 1. Add a menu item with "X" image, which is clicked to quit the program.

        // Create a "close" menu item with close icon, it's an auto release object.
        CCMenuItemImage *pCloseItem = CCMenuItemImage::itemFromNormalImage(
            "CloseNormal.png",
            "CloseSelected.png",
            this,
            menu_selector(Soko::menuCloseCallback));
        CC_BREAK_IF(! pCloseItem);

        // Place the menu item bottom-right conner.
        pCloseItem->setPosition(ccp(CCDirector::sharedDirector()->getWinSize().width - 20, 20));

		
		CCMenuItemImage *pRestartItem = CCMenuItemImage::itemFromNormalImage("Soko_RestartButton.png",
				"Soko_RestartButton.png", this, menu_selector(Soko::menuRestartCallback));
		pRestartItem->setPosition(ccp(680,148));
        // Create a menu with the "close" menu item, it's an auto release object.
        CCMenu* pMenu = CCMenu::menuWithItems(pCloseItem, pRestartItem, NULL);
        pMenu->setPosition(CCPointZero);
        CC_BREAK_IF(! pMenu);

        // Add the menu to Soko layer as a child layer.
        this->addChild(pMenu, 1);
        bRet = true;
    } while (0);

	// My stuff goes here - will probably remove all of the above anyway as I won't want
	// the close button
	
	// Set projection to 2D so we don't get lines between our tiles
	CCDirector::sharedDirector()->setProjection(kCCDirectorProjection2D);

	mpBackground = CCSprite::spriteWithFile("Soko_Background.png");
	CCAssert(mpBackground != NULL, "Error - Soko_Background.png not found");
	CCSize winsize = CCDirector::sharedDirector()->getWinSize();
	mpBackground->setPosition(ccp(winsize.width/2, winsize.height/2));
	this->addChild(mpBackground, 0);

	SimpleAudioEngine::sharedEngine()->preloadEffect( CCFileUtils::fullPathFromRelativePath("Soko_Footsteps01.wav") );
	SimpleAudioEngine::sharedEngine()->preloadEffect( CCFileUtils::fullPathFromRelativePath("Soko_Footsteps02.wav") );
	SimpleAudioEngine::sharedEngine()->setEffectsVolume(0.25);

	// Load up our sprite sheet
	mpSpriteBatchNode = CCSpriteBatchNode::batchNodeWithFile("Soko_Sprites.png", 64);
	if (!mpSpriteBatchNode)
		return false;
	this->addChild(mpSpriteBatchNode);

	mpLevelCompleteLabel = CCLabelBMFont::labelWithString("Level Complete!", "konqa32.fnt");
	mpLevelCompleteLabel->setPosition(ccp(680, 225));
	// So it is hidden when game starts. Normally it fades slowly on a restart
	mpLevelCompleteLabel->setOpacity(0);
	this->addChild(mpLevelCompleteLabel, 1);
	
	InitialiseLevel("Soko.tmx");

	mpLogo = CCSprite::spriteWithFile("Soko_Logo.png");
	CCAssert(mpLogo != NULL, "Error - Soko_Logo.png not found");
	mpLogo->setPosition(ccp(668,475));
	this->addChild(mpLogo, 1);
	// For a laugh I'm going to make the logo grow then shrink every once in a while
	CCFiniteTimeAction *actionScaleUp = CCScaleTo::actionWithDuration( (ccTime)0.25, 1.10 );
	CCFiniteTimeAction *actionScaleDown = CCScaleTo::actionWithDuration( 0.25, 1.0 );
	CCDelayTime *actionDelay = CCDelayTime::actionWithDuration(10.0f);
	mpLogo->runAction( CCRepeatForever::actionWithAction(
		(CCSequence *)CCSequence::actions(actionScaleUp, actionScaleDown, actionDelay, NULL)) );

	// Add some text labels for information purposes
	mpLevelLabel = CCLabelBMFont::labelWithString("Level: 01", "konqa32.fnt");
	mpLevelLabel->setPosition(ccp(680, 400));
	this->addChild(mpLevelLabel, 1);

	mpMovesLabel = CCLabelBMFont::labelWithString("Moves: 0000", "konqa32.fnt");
	mpMovesLabel->setPosition(ccp(680, 350));
	this->addChild(mpMovesLabel, 1);

	mpPushesLabel = CCLabelBMFont::labelWithString("Pushes: 0000", "konqa32.fnt");
	mpPushesLabel->setPosition(ccp(680, 300));
	this->addChild(mpPushesLabel, 1);

	// Call update every frame
	this->schedule( schedule_selector(Soko::GameTick) );

    return bRet;
}

bool Soko::InitialiseLevel(char *LevelName)
{
	mLevel = 0;
	mGameRunning = true;

	// Neat way to remove the label when we restart
	mpLevelCompleteLabel->stopAllActions();
	if (mpLevelCompleteLabel->getOpacity() != 0)
		mpLevelCompleteLabel->runAction(CCFadeOut::actionWithDuration(0.5));

	// Load in tilemap
	this->setTileMap( CCTMXTiledMap::tiledMapWithTMXFile(LevelName) );
	
	this->getTileMap()->retain();
	this->getTileMap()->setScale(0.40);
	this->getTileMap()->setPosition(ccp(128+32,128));
	this->addChild(this->getTileMap());

	// Now place the boxes - they are stored in the map but as Objects,
	// rather than tiles, since I know they will move
	CCTMXObjectGroup *pMobiles = this->getTileMap()->objectGroupNamed("Mobiles");
	CCAssert(pMobiles != NULL, "Error no Mobiles layer in tile map");

	CCMutableArray< CCStringToStringDictionary * > *pObjs = pMobiles->getObjects();
	for( CCMutableArray< CCStringToStringDictionary * >::CCMutableArrayIterator it = pObjs->begin();
		it != pObjs->end(); ++it)
	{
		CCStringToStringDictionary *pDict = *it;
		if ( pDict->objectForKey("name")->toStdString().compare("Box") == 0 )
		{
			int BoxX = pDict->objectForKey("x")->toInt();
			int BoxY = pDict->objectForKey("y")->toInt();
			// Since meta layer is by pixel, rather than by tile
			// Grid coord needs to be divided by 64

			// TODO - Box texture should change when it is over a goal point.
			// Works fine, so long as box doesn't start on a goal
			CCSprite *pBoxSprite = CCSprite::spriteWithTexture(mpSpriteBatchNode->getTexture(),
															CCRectMake(64,0,64,64));
			// No scaling needed since it is a child of layer
			pBoxSprite->setTag(1);
			pBoxSprite->setPosition( ccp(BoxX+32, BoxY+32) );
			this->getTileMap()->addChild(pBoxSprite);
		}
	}

	// Position the player
	mpPlayer = new Player();
	mpPlayer->init(); // Should this happen automatically? It doesn't.
	// TODO - Should do this in player init() fn?
	mpPlayer->mpSprite = CCSprite::spriteWithTexture(mpSpriteBatchNode->getTexture(),
															CCRectMake(0,0,64,64));
	CCStringToStringDictionary *spawnPoint = pMobiles->objectNamed("PlayerStart");
	CCAssert(spawnPoint != NULL, "PlayerStart object not found in Mobiles layer");
	int PX = spawnPoint->objectForKey("x")->toInt();
	int PY = spawnPoint->objectForKey("y")->toInt();

	// I don't see why I can't just set player position - needs to be mpSprite position
	mpPlayer->mpSprite->setPosition( ccp(PX+32, PY+32) );

	this->getTileMap()->addChild(mpPlayer->mpSprite);
	mpPlayer->mIsMoving = false;
	return true;
}

Soko::~Soko()
{
	SimpleAudioEngine::sharedEngine()->end();
	CC_SAFE_RELEASE_NULL(mpSpriteBatchNode);
	CC_SAFE_RELEASE_NULL(mpTileMap);
	// Don't know if the sprites and labels need releasing?
}

void Soko::menuCloseCallback(CCObject* pSender)
{
    // "close" menu item clicked
    CCDirector::sharedDirector()->end();
}

void Soko::menuRestartCallback(CCObject* pSender)
{
	InitialiseLevel("Soko.tmx");
	// Updates the move count labels and boxes
	spriteMoveFinished(NULL);
}

// This will handle the keyboard processing
void Soko::GameTick(ccTime dt)
{
	// So we don't move while "Level Complete" is on screen
	if (mGameRunning == false)
		return;

	if (mpPlayer->mIsMoving == false)
	{
		// Player moves in steps, so only change his move destination
		// if the key is still down while he is no longer moving
		int dx = 0;
		int dy = 0;
		if (GetAsyncKeyState(VK_UP) & 0x8000)
			dy++;
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			dy--;

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			++dx;
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			--dx;
		
		MovePlayer(dx, dy);
	}
}

bool Soko::MovePlayer(int dx, int dy)
{
	// Only move if not a stationary move or a diagonal move
	if ( (dx!=0 && dy!=0)  || (dx==0 && dy==0) )
		return false;

	// Do some "collision detection" with the walls
	int oldX = mpPlayer->mpSprite->getPosition().x;
	int oldY = mpPlayer->mpSprite->getPosition().y;
	int oldCellX = oldX / 64;
	int oldCellY = oldY / 64;
	int mapYSize = this->getTileMap()->getMapSize().height;
	CCTMXLayer *pTMGround = this->getTileMap()->layerNamed("Ground");

	// Allow for y coords of map being back to front
	int newX = oldCellX+dx;
	int newY = mapYSize - 1 - (oldCellY+dy);
	int tileGid = pTMGround->tileGIDAt(ccp(newX, newY ) );
	if (tileGid == 1) // Trying to move into wall
		return false;

	// Now see if we're trying to push a box
	// - all box sprites have tag set to one
	CCSprite *pPushedBox = NULL;
	CCArray *pBoxes = this->getTileMap()->getChildren();
	for(int i=0; i<pBoxes->count(); ++i)
	{
		CCSprite *pBox = (CCSprite *)pBoxes->objectAtIndex(i);
		if (pBox->getTag() == 1)
		{
			int boxXCell = (pBox->getPosition().x-32) / 64;
			int boxYCell = mapYSize - 1 - ((pBox->getPosition().y-32) / 64);
			if (boxXCell==newX && boxYCell==newY)
			{
				// We're walking into a box - see if we can push it
				int nextBoxXCell = boxXCell+dx;
				int nextBoxYCell = boxYCell+(dy*-1); // Need to reverse the direction
				int nextBoxTile = pTMGround->tileGIDAt( ccp(nextBoxXCell, nextBoxYCell) );
				// - not if it is next to a wall
				if (nextBoxTile==1)
					return false;
				// See if it is next to another box - ie need to look to see if there is
				// a box in the cell we would be moving it to
				for(int j=0; j<pBoxes->count(); ++j)
				{
					CCSprite *pBox2 = (CCSprite *)pBoxes->objectAtIndex(j);
					if (pBox2->getTag() == 1)
					{
						int box2XCell = (pBox2->getPosition().x-32) / 64;
						int box2YCell = mapYSize - 1 - ((pBox2->getPosition().y-32) / 64);
						if (box2XCell == nextBoxXCell && box2YCell == nextBoxYCell)
							return false;
					}
				}

				// Push the box
				pPushedBox = pBox;
			}
		}
	}

	// Move the player (and box) and notify us when done
	++mpPlayer->mMoveCount;
	mpPlayer->mIsMoving = true;

	// Need to see which way we've moved to see if we need to spin player to face movement direction
	int newFacing = 0;
	if (dx==-1)
		newFacing = 2;
	if (dy==1)
		newFacing = 3;
	if (dy==-1)
		newFacing = 1;

	// TODO minor bug - it should really take twice as long to rotate 180 degrees as
	// it does to rotate 90
	CCFiniteTimeAction *playerRotateAction = NULL;
	float rotateDelay = 0.1f;
	if (newFacing != mpPlayer->mFacing)
	{
		playerRotateAction = CCRotateTo::actionWithDuration(rotateDelay, newFacing * 90);
	}
	else
		playerRotateAction = CCDelayTime::actionWithDuration(0.0f);

	// Slightly convoluted logic follows to get box to delay its move if player spins before
	// pushing.
	if (pPushedBox != NULL)
	{
		CCFiniteTimeAction *actionBoxMove = CCMoveTo::actionWithDuration( (ccTime)0.2,
							ccp(pPushedBox->getPosition().x+64*dx, pPushedBox->getPosition().y+64*dy) );
		if (newFacing != mpPlayer->mFacing)
			pPushedBox->runAction( CCSequence::actions( CCDelayTime::actionWithDuration(rotateDelay), actionBoxMove, NULL) );
		else
			pPushedBox->runAction( actionBoxMove );

		++mpPlayer->mPushCount;
	}

	CCFiniteTimeAction *actionMove = CCMoveTo::actionWithDuration( (ccTime)0.2,
		ccp(oldX+(dx*64), oldY+(dy*64)) );
	CCFiniteTimeAction *actionMoveDone = CCCallFuncN::actionWithTarget( this,
		callfuncN_selector(Soko::spriteMoveFinished));
	mpPlayer->mpSprite->runAction( CCSequence::actions(playerRotateAction, actionMove, actionMoveDone, NULL) );
	
	mpPlayer->mFacing = newFacing;	

	// Slightly heavier footstep if we're pushing
	if (pPushedBox == NULL)
		SimpleAudioEngine::sharedEngine()->playEffect( CCFileUtils::fullPathFromRelativePath("Soko_Footsteps01.wav") );
	else
		SimpleAudioEngine::sharedEngine()->playEffect( CCFileUtils::fullPathFromRelativePath("Soko_Footsteps02.wav") );

	return true;
}

void Soko::spriteMoveFinished(CCNode *sender)
{
	mpPlayer->mIsMoving = false;
	
	int mapYSize = this->getTileMap()->getMapSize().height;
	CCTMXLayer *pTMGround = this->getTileMap()->layerNamed("Ground");

	// Change the boxes to show whether they are on a goal spot or not
	// This code is a bit of a duplicate of the code that moves the boxes,
	// but I don't want to do the update until the move completes
	int GoalCount = 0;
	int BoxCount = 0;
	CCArray *pBoxes = this->getTileMap()->getChildren();
	for(int i=0; i<pBoxes->count(); ++i)
	{
		CCSprite *pBox = (CCSprite *)pBoxes->objectAtIndex(i);
		if (pBox->getTag() == 1)
		{
			// Need to actually count these as tile map has children other than
			// just the boxes, such as the player and any effects I might add
			++BoxCount;

			int boxXCell = (pBox->getPosition().x-32) / 64;
			int boxYCell = mapYSize - 1 - ((pBox->getPosition().y-32) / 64);

			if (pTMGround->tileGIDAt( ccp(boxXCell, boxYCell) ) == 2)
			{
				// On floor
				pBox->setTextureRect(CCRectMake(64,0,64,64));
			}
			else if (pTMGround->tileGIDAt( ccp(boxXCell, boxYCell) ) == 3)
			{
				// On goal
				++GoalCount;
				pBox->setTextureRect(CCRectMake(128,0,64,64));
			}
		}
	}

	ostringstream movesStr;
	movesStr << "Moves: " << setw(4) << setfill('0') << mpPlayer->mMoveCount;
	ostringstream pushesStr;
	pushesStr << "Pushes: " << setw(4) << setfill('0') << mpPlayer->mPushCount;
	// NB This crashed when I used TTFLabels
	mpMovesLabel->setString(movesStr.str().c_str());
	mpPushesLabel->setString(pushesStr.str().c_str());

	if (GoalCount == BoxCount)
	{
		// Win!
		mGameRunning = false;
		// Fade label in and bounce its size a little
		mpLevelCompleteLabel->runAction(CCFadeIn::actionWithDuration(0.5));
		mpLevelCompleteLabel->runAction( CCSequence::actions(
			CCScaleTo::actionWithDuration( (ccTime)0.25, 1.10 ),
			CCScaleTo::actionWithDuration( (ccTime)0.25, 1.00 ), NULL) );
	}
}

bool Player::init()
{
	mPushCount = 0;
	mMoveCount = 0;
	mFacing = 0;
	return true;
}