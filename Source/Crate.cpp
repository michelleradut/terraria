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
#include "Crate.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
Crate::Crate(const BackBuffer* pBackBuffer)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/crate.bmp", RGB(0xff, 0x00, 0xff));
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
Crate::~Crate()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
}

void Crate::Update(float dt)
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

void Crate::Draw()
{
	if (!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}

void Crate::Move(ULONG ulDirection)
{
	int height = 0, width = 0;
	RECT rect;
	if (GetClientRect(GetActiveWindow(), &rect))
	{
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	if (m_pSprite->mPosition.y - m_pSprite->height() / 2 >= 0)
		m_pSprite->mPosition.y += 0.1;
	else
		this->out = true;
}


Vec2& Crate::Position()
{
	return m_pSprite->mPosition;
}

Vec2& Crate::Velocity()
{
	return m_pSprite->mVelocity;
}

void Crate::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool Crate::AdvanceExplosion()
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

bool Crate::GetShot(Bullet& bullet)
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

bool Crate::AreIntersecting(const RECT& aFirst, const RECT& aSecond)
{
	if (aFirst.right < aSecond.left || aSecond.right < aFirst.left)
		return false;

	if (aFirst.bottom < aSecond.top || aSecond.bottom < aFirst.top)
		return false;

	return true;
}