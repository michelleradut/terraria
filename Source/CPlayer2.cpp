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
#include "CPlayer2.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
CPlayer2::CPlayer2(const BackBuffer* pBackBuffer)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/plane2imgandmask.bmp", RGB(0xff, 0x00, 0xff));
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
CPlayer2::~CPlayer2()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
}

void CPlayer2::Update(float dt)
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
	switch (m_eSpeedState)
	{
	case SPEED_STOP:
		if (v > 35.0f)
		{
			m_eSpeedState = SPEED_START;
			PlaySound("data/jet-start.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		break;
	case SPEED_START:
		if (v < 25.0f)
		{
			m_eSpeedState = SPEED_STOP;
			PlaySound("data/jet-stop.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		else
			if (m_fTimer > 1.f)
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

void CPlayer2::Draw()
{
	if (!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}

int CPlayer2::getHeight()
{
	return m_pSprite->height();
}
int CPlayer2::getWidth()
{
	return m_pSprite->width();
}
int CPlayer2::getLife()
{
	return life;
}
void CPlayer2::Move(ULONG ulDirection)
{
	if (ulDirection & CPlayer2::DIR_LEFT)
		m_pSprite->mVelocity.x -= .1;
	if (m_pSprite->mPosition.x < 1 + m_pSprite->width() / 2)
		m_pSprite->mVelocity.x = 0;
	//adaug un pixel (1 + m_pSprite->width()/2) in loc de (0 + m_pSprite->width()/2) pentru a evita coliziunea cu marginea

	if (ulDirection & CPlayer2::DIR_RIGHT)
		m_pSprite->mVelocity.x += .1;
	if (m_pSprite->mPosition.x > 800 - m_pSprite->width() / 2)
	{
		m_pSprite->mPosition.x = 800 - m_pSprite->width() / 2;
		m_pSprite->mVelocity.x = 0;
	}

	if (ulDirection & CPlayer2::DIR_FORWARD)
		m_pSprite->mVelocity.y -= .1;
	if (m_pSprite->mPosition.y < 1 + m_pSprite->height() / 2)
		m_pSprite->mVelocity.y = 0;

	if (ulDirection & CPlayer2::DIR_BACKWARD)
		m_pSprite->mVelocity.y += .1;
	if (m_pSprite->mPosition.y > 600 - m_pSprite->height() / 2)
	{
		m_pSprite->mPosition.y = 600 - m_pSprite->height() / 2;
		m_pSprite->mVelocity.y = 0;
	}
	//daca as lasa doar velocity, se blocheaza in marginea de jos sau din dreapta, de ce?
}


Vec2& CPlayer2::Position()
{
	return m_pSprite->mPosition;
}

Vec2& CPlayer2::Velocity()
{
	return m_pSprite->mVelocity;
}

void CPlayer2::ResetVelocity()
{
	m_pSprite->mVelocity.x = 0;
	m_pSprite->mVelocity.y = 0;
}
void CPlayer2::DecreaseLife()
{
	life--;
}

void CPlayer2::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool CPlayer2::AdvanceExplosion()
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
//adaugat
bool CPlayer2::GetShot(Bullet& bullet)
{
	
	RECT bulletRect = bullet.GetRectangle();
	RECT playerRect = m_pSprite->GetRectangle();

	auto isShot = AreIntersecting(bulletRect, playerRect) &&
		m_pSprite->AreMasksOverlapping(*bullet.GetSpritePtr());

	if (isShot)
	{
		//Explode();
		//AdvanceExplosion();
		DecreaseLife();
		return true;
	}


	return false;
}

bool CPlayer2::AreIntersecting(const RECT& aFirst, const RECT& aSecond)
{
	if (aFirst.right < aSecond.left || aSecond.right < aFirst.left)
		return false;

	if (aFirst.bottom < aSecond.top || aSecond.bottom < aFirst.top)
		return false;

	return true;
}
