//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "CPlayer.h"
#include "CGameApp.h"
extern CGameApp g_App;
//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
CPlayer::CPlayer(const BackBuffer *pBackBuffer) : rotateDirection(DIRECTION::DIR_FORWARD)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/planeimgandmask.bmp", RGB(0xff,0x00, 0xff));
	m_pSprite->setBackBuffer( pBackBuffer );
	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 128;
	r.bottom = 128;

	m_pExplosionSprite	= new AnimatedSprite("data/explosion.bmp", "data/explosionmask.bmp", r, 16);
	//se modifica in 16, pentru cele 16 frame-uri ale exploziei
	m_pExplosionSprite->setBackBuffer( pBackBuffer );
	m_bExplosion		= false;
	m_iExplosionFrame	= 0;
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
CPlayer::~CPlayer()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
}

void CPlayer::Update(float dt)
{
	// Update sprite
	m_pSprite->update(dt);


	// Get velocity
	double v = m_pSprite->mVelocity.Magnitude();

	// NOTE: for each async sound played Windows creates a thread for you
	// but only one, so you cannot play multiple sounds at once.
	// This creation/destruction of threads also leads to bad performance
	// so this method is not recommanded to be used in complex projects.

	// update internal time counter used in sound handling (not to overlap sounds)
	m_fTimer += dt;

	// A FSM is used for sound manager 
	switch(m_eSpeedState)
	{
	case SPEED_STOP:
		if(v > 35.0f)
		{
			m_eSpeedState = SPEED_START;
			PlaySound("data/jet-start.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		break;
	case SPEED_START:
		if(v < 25.0f)
		{
			m_eSpeedState = SPEED_STOP;
			PlaySound("data/jet-stop.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		else
			if(m_fTimer > 1.f)
			{
				PlaySound("data/jet-cabin.wav", NULL, SND_FILENAME | SND_ASYNC);
				m_fTimer = 0;
			}
		break;
	}

	// NOTE: For sound you also can use MIDI but it's Win32 API it is a bit hard
	// see msdn reference: http://msdn.microsoft.com/en-us/library/ms711640.aspx
	// In this case you can use a C++ wrapper for it. See the following article:
	// http://www.codeproject.com/KB/audio-video/midiwrapper.aspx (with code also)
}

void CPlayer::Draw()
{
	if(!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}

int CPlayer::getHeight()
{
	return m_pSprite->height();
}
int CPlayer::getWidth()
{
	return m_pSprite->width();
}
int CPlayer::getLife()
{
	return life;
}
void CPlayer::DecreaseLife()
{
	life--;
}
void CPlayer::IncreaseLife()
{
	life++;
}
void CPlayer::IncreaseScore()
{
	score = score + 50;
}
int CPlayer::getScore()
{
	return score;
}
void CPlayer::ReadLifes(int lives)
{
	this->life = lives;
}
void CPlayer::ReadScore(int score)
{
	this->score = score;
}


void CPlayer::Move(ULONG ulDirection)
{
	if( ulDirection & CPlayer::DIR_LEFT )
		m_pSprite->mVelocity.x -= .5;
	if (m_pSprite->mPosition.x < 1 + m_pSprite->width() / 2)
		m_pSprite->mVelocity.x = 0;
	//adaug un pixel (1 + m_pSprite->width()/2) in loc de (0 + m_pSprite->width()/2) pentru a evita coliziunea cu marginea

	if( ulDirection & CPlayer::DIR_RIGHT )
		m_pSprite->mVelocity.x += .5;
	if (m_pSprite->mPosition.x > 800 - m_pSprite->width() / 2)
	{
		m_pSprite->mPosition.x = 800 - m_pSprite->width() / 2;
		m_pSprite->mVelocity.x = 0;
	}

	if( ulDirection & CPlayer::DIR_FORWARD )
		m_pSprite->mVelocity.y -= .5;
	if (m_pSprite->mPosition.y < 1 + m_pSprite->height() / 2)
		m_pSprite->mVelocity.y = 0;

	if( ulDirection & CPlayer::DIR_BACKWARD )
		m_pSprite->mVelocity.y += .5;
	if (m_pSprite->mPosition.y > 600 - m_pSprite->height() / 2)
	{
		m_pSprite->mPosition.y = 600 - m_pSprite->height() / 2;
		m_pSprite->mVelocity.y = 0;
	}
	//daca as lasa doar velocity, se blocheaza in marginea de jos sau din dreapta, de ce?
}


Vec2& CPlayer::Position()
{
	return m_pSprite->mPosition;
}

Vec2& CPlayer::Velocity()
{
	return m_pSprite->mVelocity;
}
void CPlayer::ResetVelocity()
{
	m_pSprite->mVelocity.x = 0;
	m_pSprite->mVelocity.y = 0;
}

void CPlayer::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
	
	
}

bool CPlayer::AdvanceExplosion()
{
	if(m_bExplosion)
	{
		m_pExplosionSprite->SetFrame(m_iExplosionFrame++);
		if(m_iExplosionFrame==m_pExplosionSprite->GetFrameCount())
		{
			m_bExplosion = false;
			m_iExplosionFrame = 0;
			m_pSprite->mVelocity = Vec2(0,0);
			m_eSpeedState = SPEED_STOP;
			return false;
		}
	}

	return true;
}

bool CPlayer::GetShot(Bullet& bullet)
{
	
	RECT bulletRect = bullet.GetRectangle();
	RECT playerRect = m_pSprite->GetRectangle();

	auto isShot = AreIntersecting(bulletRect, playerRect) &&
			m_pSprite->AreMasksOverlapping(*bullet.GetSpritePtr());

	if (isShot)
	{
		//Explode();
		//AdvanceExplosion();
		life--;
		return true;
	}


	return false;
}

bool CPlayer::GetShotEnemy(EnemyBullet& bullet)
{

	RECT bulletRect = bullet.GetRectangle();
	RECT playerRect = m_pSprite->GetRectangle();

	auto isShot = AreIntersecting(bulletRect, playerRect) &&
		m_pSprite->AreMasksOverlapping(*bullet.GetSpritePtr());

	if (isShot)
	{
		//Explode();
		//AdvanceExplosion();
		life--;
		return true;
	}


	return false;
}

bool CPlayer::AreIntersecting(const RECT& aFirst, const RECT& aSecond)
{
	if (aFirst.right < aSecond.left || aSecond.right < aFirst.left)
		return false;

	if (aFirst.bottom < aSecond.top || aSecond.bottom < aFirst.top)
		return false;

	return true;
}

void CPlayer::Rotate()
{
	auto position = m_pSprite->mPosition;
	auto velocity = m_pSprite->mVelocity;

	switch (rotateDirection)
	{
	case CPlayer::DIR_FORWARD:
		m_pSprite = new Sprite("data/PlaneImgAndMaskLeft.bmp", RGB(0xff, 0x00, 0xff));

		rotateDirection = CPlayer::DIR_LEFT;
		break;
	case CPlayer::DIR_BACKWARD:
		m_pSprite = new Sprite("data/PlaneImgAndMaskRight.bmp", RGB(0xff, 0x00, 0xff));
		rotateDirection = CPlayer::DIR_RIGHT;
		break;
	case CPlayer::DIR_LEFT:
		rotateDirection = CPlayer::DIR_BACKWARD;
		m_pSprite = new Sprite("data/planeimgandmaskk.bmp", RGB(0xff, 0x00, 0xff));
		break;
	case CPlayer::DIR_RIGHT:
		m_pSprite = new Sprite("data/planeimgandmask.bmp", RGB(0xff, 0x00, 0xff));
		rotateDirection = CPlayer::DIR_FORWARD;
		break;
	}

	m_pSprite->mPosition = position;
	m_pSprite->mVelocity = velocity;
	m_pSprite->setBackBuffer(g_App.m_pBBuffer);
}