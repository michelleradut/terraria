//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//	   such as player movement, some minor physics as well as rendering.
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

#ifndef _CPLAYER_H_
#define _CPLAYER_H_

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "Sprite.h"
#include "Bullet.h"
#include <vector>
#include "EnemyBullet.h"

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPlayer (Class)
// Desc : Player class handles all player manipulation, update and management.
//-----------------------------------------------------------------------------
class CPlayer
{
public:
	//-------------------------------------------------------------------------
	// Enumerators
	//-------------------------------------------------------------------------
	enum DIRECTION 
	{ 
		DIR_FORWARD	 = 1, 
		DIR_BACKWARD	= 2, 
		DIR_LEFT		= 4, 
		DIR_RIGHT	   = 8, 
	};

	enum ESpeedStates
	{
		SPEED_START,
		SPEED_STOP
	};

	//-------------------------------------------------------------------------
	// Constructors & Destructors for This Class.
	//-------------------------------------------------------------------------
			 CPlayer(const BackBuffer *pBackBuffer);
	virtual ~CPlayer();

	//-------------------------------------------------------------------------
	// Public Functions for This Class.
	//-------------------------------------------------------------------------
	void					Update( float dt );
	void					Draw();
	int						getHeight();
	int						getWidth();
	int						getLife();
	int						getScore();
	void					Move(ULONG ulDirection);
	Vec2&					Position();
	Vec2&					Velocity();
	void					ResetVelocity();

	void					Explode();
	bool					AdvanceExplosion();

	bool					GetShot(Bullet& bullet);
	bool					GetShotEnemy(EnemyBullet& bullet);
	bool					AreIntersecting(const RECT& aFirst, const RECT& aSecond);
	void					DecreaseLife();
	void					IncreaseLife();
	void					IncreaseScore();
	void					Rotate();
	DIRECTION				rotateDirection;
	void					ReadLifes(int lives);
	void					ReadScore(int score);




private:
	//-------------------------------------------------------------------------
	// Private Variables for This Class.
	//-------------------------------------------------------------------------
	Sprite*					m_pSprite;
	ESpeedStates			m_eSpeedState;
	float					m_fTimer;
	
	bool					m_bExplosion;
	AnimatedSprite*			m_pExplosionSprite;
	int						m_iExplosionFrame;
	const BackBuffer*		mBackBuffer;
	int						life = 3;
	int						score = 0;

};

#endif // _CPLAYER_H_