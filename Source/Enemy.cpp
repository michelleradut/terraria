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
#include "Enemy.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
Enemy::Enemy(const BackBuffer* pBackBuffer)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/enemy.bmp", RGB(0xff, 0x00, 0xff));
	m_pSprite->setBackBuffer(pBackBuffer);
	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 128;
	r.bottom = 128;

	m_pExplosionSprite = new AnimatedSprite("data/explosion.bmp", "data/explosionmask.bmp", r, 16);
	//se modifica in 16, pentru cele 16 frame-uri ale exploziei
	m_pExplosionSprite->setBackBuffer(pBackBuffer);
	m_bExplosion = false;
	m_iExplosionFrame = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
Enemy::~Enemy()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
}

void Enemy::Update(float dt)
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

}

void Enemy::Draw()
{
	if (!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}
int Enemy::getHeight()
{
	return m_pSprite->height();
}
void Enemy::Move(ULONG ulDirection)
{
	//m_pSprite->mPosition.y = 60;
	//m_pSprite->mPosition.x = 60;

	//if (m_pSprite->mPosition.x > 700)
	{
		//m_pSprite->mVelocity.x -= .2;
	}
	if (m_pSprite->mPosition.x > 100)
	{
		m_pSprite->mVelocity.x -= .2;
	}
}
void Enemy::PositiveXVelocity()
{
	m_pSprite->mVelocity.x += .2;

}
void Enemy::NegativeXVelocity()
{
	m_pSprite->mVelocity.x -= .2;
}


Vec2& Enemy::Position()
{
	return m_pSprite->mPosition;
}

Vec2& Enemy::Velocity()
{
	return m_pSprite->mVelocity;
}

void Enemy::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool Enemy::AdvanceExplosion()
{
	if (m_bExplosion)
	{
		m_pExplosionSprite->SetFrame(m_iExplosionFrame++);
		if (m_iExplosionFrame == m_pExplosionSprite->GetFrameCount())
		{
			m_bExplosion = false;
			m_iExplosionFrame = 0;
			m_pSprite->mVelocity = Vec2(0, 0);
			m_eSpeedState = SPEED_STOP;
			return false;
		}
	}

	return true;
}

bool Enemy::GetShot(Bullet& bullet)
{

	RECT bulletRect = bullet.GetRectangle();
	RECT playerRect = m_pSprite->GetRectangle();

	auto isShot = AreIntersecting(bulletRect, playerRect) &&
		m_pSprite->AreMasksOverlapping(*bullet.GetSpritePtr());

	if (isShot)
	{
		//Explode();
		//AdvanceExplosion();
		return true;
	}


	return false;
}

bool Enemy::AreIntersecting(const RECT& aFirst, const RECT& aSecond)
{
	if (aFirst.right < aSecond.left || aSecond.right < aFirst.left)
		return false;

	if (aFirst.bottom < aSecond.top || aSecond.bottom < aFirst.top)
		return false;

	return true;
}