//-----------------------------------------------------------------------------
// File: CGameApp.cpp
//
// Desc: Game Application class, this is the central hub for all app processing
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CGameApp Specific Includes
//-----------------------------------------------------------------------------
#include "CGameApp.h"
#include <iostream>
#include <fstream>
using namespace std;

extern HINSTANCE g_hInst;

//-----------------------------------------------------------------------------
// CGameApp Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CGameApp () (Constructor)
// Desc : CGameApp Class Constructor
//-----------------------------------------------------------------------------
CGameApp::CGameApp()
{
	// Reset / Clear all required values
	m_hWnd			= NULL;
	m_hIcon			= NULL;
	m_hMenu			= NULL;
	m_pBBuffer		= NULL;
	m_pPlayer		= NULL;
	m_pPlayer2		= NULL; 
	m_LastFrameRate = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CGameApp () (Destructor)
// Desc : CGameApp Class Destructor
//-----------------------------------------------------------------------------
CGameApp::~CGameApp()
{
	// Shut the engine down
	ShutDown();
}

//-----------------------------------------------------------------------------
// Name : InitInstance ()
// Desc : Initialises the entire Engine here.
//-----------------------------------------------------------------------------
bool CGameApp::InitInstance( LPCTSTR lpCmdLine, int iCmdShow )
{
	// Create the primary display device
	if (!CreateDisplay()) { ShutDown(); return false; }

	// Build Objects
	if (!BuildObjects()) 
	{ 
		MessageBox( 0, _T("Failed to initialize properly. Reinstalling the application may solve this problem.\nIf the problem persists, please contact technical support."), _T("Fatal Error"), MB_OK | MB_ICONSTOP);
		ShutDown(); 
		return false; 
	}

	// Set up all required game states
	SetupGameState();

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CreateDisplay ()
// Desc : Create the display windows, devices etc, ready for rendering.
//-----------------------------------------------------------------------------
bool CGameApp::CreateDisplay()
{
	LPTSTR			WindowTitle		= _T("GameFramework");
	LPCSTR			WindowClass		= _T("GameFramework_Class");
	USHORT			Width			= 800;
	USHORT			Height			= 600;
	RECT			rc;
	WNDCLASSEX		wcex;


	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CGameApp::StaticWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= WindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

	if(RegisterClassEx(&wcex)==0)
		return false;

	// Retrieve the final client size of the window
	::GetClientRect( m_hWnd, &rc );
	m_nViewX		= rc.left;
	m_nViewY		= rc.top;
	m_nViewWidth	= rc.right - rc.left;
	m_nViewHeight	= rc.bottom - rc.top;

	m_hWnd = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, NULL, NULL, g_hInst, this);

	if (!m_hWnd)
		return false;

	// Show the window
	ShowWindow(m_hWnd, SW_SHOW); 
	//inlocuind cu SW_MAXIMIZE se va face fullscreen

	// Success!!
	return true;
}

//-----------------------------------------------------------------------------
// Name : BeginGame ()
// Desc : Signals the beginning of the physical post-initialisation stage.
//		From here on, the game engine has control over processing.
//-----------------------------------------------------------------------------
int CGameApp::BeginGame()
{
	MSG		msg;

	// Start main loop
	while(true) 
	{
		// Did we recieve a message, or are we idling ?
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) 
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		} 
		else 
		{
			// Advance Game Frame.
			FrameAdvance();

		} // End If messages waiting
	
	} // Until quit message is receieved

	return 0;
}

//-----------------------------------------------------------------------------
// Name : ShutDown ()
// Desc : Shuts down the game engine, and frees up all resources.
//-----------------------------------------------------------------------------
bool CGameApp::ShutDown()
{
	// Release any previously built objects
	ReleaseObjects ( );
	
	// Destroy menu, it may not be attached
	if ( m_hMenu ) DestroyMenu( m_hMenu );
	m_hMenu		 = NULL;

	// Destroy the render window
	SetMenu( m_hWnd, NULL );
	if ( m_hWnd ) DestroyWindow( m_hWnd );
	m_hWnd		  = NULL;
	
	// Shutdown Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : StaticWndProc () (Static Callback)
// Desc : This is the main messge pump for ALL display devices, it captures
//		the appropriate messages, and routes them through to the application
//		class for which it was intended, therefore giving full class access.
// Note : It is VITALLY important that you should pass your 'this' pointer to
//		the lpParam parameter of the CreateWindow function if you wish to be
//		able to pass messages back to that app object.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CGameApp::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// If this is a create message, trap the 'this' pointer passed in and store it within the window.
	if ( Message == WM_CREATE ) SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

	// Obtain the correct destination for this message
	CGameApp *Destination = (CGameApp*)GetWindowLong( hWnd, GWL_USERDATA );
	
	// If the hWnd has a related class, pass it through
	if (Destination) return Destination->DisplayWndProc( hWnd, Message, wParam, lParam );
	
	// No destination found, defer to system...
	return DefWindowProc( hWnd, Message, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name : DisplayWndProc ()
// Desc : The display devices internal WndProc function. All messages being
//		passed to this function are relative to the window it owns.
//-----------------------------------------------------------------------------
LRESULT CGameApp::DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	static UINT			fTimer;	
	static TCHAR TitleBuffer[255];
	std::ofstream fout;
	std::ifstream fin;

	// Determine message type
	switch (Message)
	{
		case WM_CREATE:
			break;
		
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		case WM_SIZE:
			if ( wParam == SIZE_MINIMIZED )
			{
				// App is inactive
				m_bActive = false;
			
			} // App has been minimized
			else
			{
				// App is active
				m_bActive = true;

				// Store new viewport sizes
				m_nViewWidth  = LOWORD( lParam );
				m_nViewHeight = HIWORD( lParam );
		
			
			} // End if !Minimized

			break;

		case WM_LBUTTONDOWN:
			// Capture the mouse
			SetCapture( m_hWnd );
			GetCursorPos( &m_OldCursorPos );
			break;

		case WM_LBUTTONUP:
			// Release the mouse
			ReleaseCapture( );
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			case VK_RETURN:
				fTimer = SetTimer(m_hWnd, 1, 250, NULL);
				m_pPlayer->Explode();
				break;
			case 0x51://case for q?
				fTimer = SetTimer(m_hWnd, 2, 250, NULL);
				m_pPlayer2->Explode();
				break;
			case 0x4f: //O
				fout.open("Data/saveloadfile.txt");
				fout << m_pPlayer->Position().x << " " << m_pPlayer->Position().y << "\n ";
				fout << m_pPlayer2->Position().x << " " << m_pPlayer2->Position().y << "\n ";
				fout << m_pPlayer->getLife() << "\n ";
				fout<< m_pPlayer->getScore() << "\n ";
				sprintf_s(TitleBuffer, _T("Game: %s"), "Game saved");
				SetWindowText(m_hWnd, TitleBuffer);
				fout.close();
				break;
			case 0x4c: //L
				fin.open("Data/saveloadfile.txt");
				if (fin)
				{
					int lives;
					int score;
					fin >> m_pPlayer->Position().x >> m_pPlayer->Position().y;
					m_pPlayer->Velocity().x = 0;
					m_pPlayer->Velocity().y = 0;
					fin >> m_pPlayer2->Position().x >> m_pPlayer2->Position().y;
					m_pPlayer2->Velocity().x = 0;
					m_pPlayer2->Velocity().y = 0;
					fin >> lives;
					m_pPlayer->ReadLifes(lives);
					fin >> score;
					m_pPlayer->ReadScore(score);
					sprintf_s(TitleBuffer, _T("Game: %s"), "Game loaded");
					SetWindowText(m_hWnd, TitleBuffer);
				}
				else
				{
					sprintf_s(TitleBuffer, _T("Game: %s"), "Unable to open file");
					SetWindowText(m_hWnd, TitleBuffer);
				}
				fin.close();
				break;
			case 0x4e: //N
				m_pPlayer->Rotate();
				break;
			}
			//break;

		case WM_TIMER:
			switch(wParam)
			{
			case 1:
				if(!m_pPlayer->AdvanceExplosion())
					KillTimer(m_hWnd, 1);
			case 2:
				if (!m_pPlayer2->AdvanceExplosion())
					KillTimer(m_hWnd, 2);
			}
			break;

		case WM_COMMAND:
			break;

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);

	} // End Message Switch
	
	return 0;
}

//-----------------------------------------------------------------------------
// Name : BuildObjects ()
// Desc : Build our demonstration meshes, and the objects that instance them
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
	m_pBBuffer = new BackBuffer(m_hWnd, m_nViewWidth, m_nViewHeight);
	m_pPlayer = new CPlayer(m_pBBuffer);
	m_pPlayer2 = new CPlayer2(m_pBBuffer);
	m_pBullet = {};
	m_pCrate = {};
	m_pHeart = {};
	m_pEnemy = {};
	m_pEnemyBullet = {};


	if(!m_imgBackground.LoadBitmapFromFile("data/background.bmp", GetDC(m_hWnd)))
		return false;

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : SetupGameState ()
// Desc : Sets up all the initial states required by the game.
//-----------------------------------------------------------------------------
void CGameApp::SetupGameState()
{
	m_pPlayer->Position() = Vec2(100, 400);
	m_pPlayer2->Position() = Vec2(400, 400);
}

//-----------------------------------------------------------------------------
// Name : ReleaseObjects ()
// Desc : Releases our objects and their associated memory so that we can
//		rebuild them, if required, during our applications life-time.
//-----------------------------------------------------------------------------
void CGameApp::ReleaseObjects( )
{
	if(m_pPlayer != NULL)
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	if (m_pPlayer2 != NULL)
	{
		delete m_pPlayer2;
		m_pPlayer2 = NULL;
	}

	if(m_pBBuffer != NULL)
	{
		delete m_pBBuffer;
		m_pBBuffer = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
	static TCHAR FrameRate[ 50 ];
	static TCHAR TitleBuffer[ 255 ];

	// Advance the timer
	m_Timer.Tick( );

	// Skip if app is inactive
	if ( !m_bActive ) return;
	
	// Get / Display the framerate
	if ( m_LastFrameRate != m_Timer.GetFrameRate() )
	{
		m_LastFrameRate = m_Timer.GetFrameRate( FrameRate, 50 );
		sprintf_s( TitleBuffer, _T("Game : %s     Lives: %d      Score: %d"), FrameRate, m_pPlayer->getLife(), m_pPlayer->getScore());
		SetWindowText( m_hWnd, TitleBuffer );

	} // End if Frame Rate Altered

	// Poll & Process input devices
	ProcessInput();

	// Animate the game objects
	AnimateObjects();

	// Drawing the game objects
	DrawObjects();
}
void CGameApp::PlaneCollision()
{
	static UINT fTimer;
	double distance = m_pPlayer->Position().Distance(m_pPlayer2->Position());
	if (distance <= m_pPlayer->getWidth())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_pPlayer->Explode();
		m_pPlayer->Position() = Vec2(100, 400);
		m_pPlayer->ResetVelocity();
		m_pPlayer->DecreaseLife();
		fTimer = SetTimer(m_hWnd, 2, 250, NULL);
		m_pPlayer2->Explode();
		m_pPlayer2->Position() = Vec2(400, 400);
		m_pPlayer2->ResetVelocity();
		m_pPlayer2->DecreaseLife();
		if (m_pPlayer->getLife() == 0)
		{
			MessageBox(m_hWnd, "Game ended", "Player 1 died", MB_OK);
			PostQuitMessage(0);
		}
		/*if (m_pPlayer2->getLife() == 0)
		{
			MessageBox(m_hWnd, "Game ended", "Player 2 died", MB_OK);
			PostQuitMessage(0);
		}*/
	}
}
void CGameApp::CrateCollision()
{
	static UINT fTimer;
	for (int i = 0; i < m_pCrate.size(); i++)
	{
		double distance = m_pPlayer->Position().Distance(m_pCrate[i]->Position());
		if (distance <= m_pPlayer->getWidth())
		{
			fTimer = SetTimer(m_hWnd, 1, 250, NULL);
			m_pPlayer->Explode();
			m_pPlayer->Position() = Vec2(100, 400);
			m_pPlayer->ResetVelocity();
			m_pPlayer->DecreaseLife();
			

			fTimer = SetTimer(m_hWnd, 2, 250, NULL);
			m_pCrate[i]->Explode();
			m_pCrate[i]->AdvanceExplosion();
			delete m_pCrate[i];
			m_pCrate.erase(m_pCrate.begin() + i);
			
			if (m_pPlayer->getLife() == 0)
			{
				MessageBox(m_hWnd, "Game ended", "Player 1 died", MB_OK);
				PostQuitMessage(0);
			}
		}
	}
}

void CGameApp::HeartCollision()
{
	for (int i = 0; i < m_pHeart.size(); i++)
	{
		double distance = m_pPlayer->Position().Distance(m_pHeart[i]->Position());
		if (distance <= m_pPlayer->getWidth())
		{
			m_pPlayer->IncreaseLife();

			delete m_pHeart[i];
			m_pHeart.erase(m_pHeart.begin() + i);
		}
	}
}
void CGameApp::Spawn()
{
	__int64 m_Time = timeGetTime();

	if (m_Time - m_CrateShootTime >= 4000) //intervalul la care apare un crate
	{
		m_CrateShootTime = m_Time;
		m_pCrate.push_back(new Crate(m_pBBuffer));
		m_pCrate.back()->Position() = Vec2(rand() % 800, 32);
	}
	if (m_Time - m_HeartShootTime >= 4000) //intervalul la care apare o viata
	{
		m_HeartShootTime = m_Time;
		m_pHeart.push_back(new Heart(m_pBBuffer));
		m_pHeart.back()->Position() = Vec2(rand() % 800, 32);
	}
	if (m_Time - m_EnemySpawnTime >= 10000) //intervalul la care apare un inamic
	{
		m_EnemySpawnTime = m_Time;
		m_pEnemy.push_back(new Enemy(m_pBBuffer));
		m_pEnemy.back()->Position() = Vec2(100, 60);

	}
	if (m_Time - m_EnemyShootTime >= 4000) //pentru a nu se trage gloante continuu
	{
		m_EnemyShootTime = m_Time;
		m_pEnemyBullet.push_back(new EnemyBullet(m_pBBuffer));
		for (int i = 0; i < m_pEnemy.size(); i++)
			m_pEnemyBullet.back()->Position() = Vec2(m_pEnemy[i]->Position().x, m_pEnemy[i]->Position().y - m_pEnemy[i]->getHeight() / 2);

	}
}
void CGameApp::Delete()
{
	for (int i = 0; i < m_pBullet.size(); i++) //stergere la iesire din ecran
	{
		if (m_pBullet[i]->out)
		{
			delete m_pBullet[i];
			m_pBullet.erase(m_pBullet.begin() + i);
			continue;
		}
		m_pBullet[i]->Move(CPlayer::DIR_FORWARD);
	}
	for (int i = 0; i < m_pCrate.size(); i++) //stergere la iesire din ecran
	{
		if (m_pCrate[i]->out)
		{
			delete m_pCrate[i];
			m_pCrate.erase(m_pCrate.begin() + i);
			continue;
		}
		m_pCrate[i]->Move(Crate::DIR_BACKWARD);
	}
	for (int i = 0; i < m_pHeart.size(); i++) //stergere la iesire din ecran
	{
		if (m_pHeart[i]->out)
		{
			delete m_pHeart[i];
			m_pHeart.erase(m_pHeart.begin() + i);
			continue;
		}
		m_pHeart[i]->Move(Crate::DIR_BACKWARD);
	}
	for (int i = 0; i < m_pEnemyBullet.size(); i++) //stergere la iesire din ecran
	{
		if (m_pEnemyBullet[i]->out)
		{
			delete m_pEnemyBullet[i];
			m_pEnemyBullet.erase(m_pEnemyBullet.begin() + i);
			continue;
		}
		m_pEnemyBullet[i]->Move(CPlayer::DIR_FORWARD);
	}
	
}
//-----------------------------------------------------------------------------
// Name : ProcessInput () (Private)
// Desc : Simply polls the input devices and performs basic input operations
//-----------------------------------------------------------------------------
void CGameApp::ProcessInput( )
{
	static UCHAR pKeyBuffer[ 256 ];
	ULONG		Direction = 0;
	ULONG		Direction2 = 0;
	//ULONG		LaunchBullet = 0;
	POINT		CursorPos;
	float		X = 0.0f, Y = 0.0f;

	// Retrieve keyboard state
	if ( !GetKeyboardState( pKeyBuffer ) ) return;

	// Check the relevant keys
	
	if ( pKeyBuffer[ VK_UP	] & 0xF0 ) Direction |= CPlayer::DIR_FORWARD;
	if ( pKeyBuffer[ VK_DOWN  ] & 0xF0 ) Direction |= CPlayer::DIR_BACKWARD;
	if ( pKeyBuffer[ VK_LEFT  ] & 0xF0 ) Direction |= CPlayer::DIR_LEFT;
	if ( pKeyBuffer[ VK_RIGHT ] & 0xF0 ) Direction |= CPlayer::DIR_RIGHT;

	if (pKeyBuffer[0x57] & 0xF0) Direction2 |= CPlayer2::DIR_FORWARD;
	if (pKeyBuffer[0x53] & 0xF0) Direction2 |= CPlayer2::DIR_BACKWARD;
	if (pKeyBuffer[0x41] & 0xF0) Direction2 |= CPlayer2::DIR_LEFT;
	if (pKeyBuffer[0x44] & 0xF0) Direction2 |= CPlayer2::DIR_RIGHT;
	//adaugat wasd pentru cea de a doua nava si direction2 pentru a nu se misca impreuna
	
	//if (pKeyBuffer[0x4e] & 0xF0) //n
		//m_pPlayer->Rotate();
	//if (pKeyBuffer[0x4d] & 0xF0) Direction |= CPlayer::Right; //m

	// Move the player
	m_pPlayer->Move(Direction);
	m_pPlayer2->Move(Direction2);
	for (int i = 0; i < m_pEnemy.size(); i++)
	{
		if (m_pEnemy[i]->Position().x < 400)
			m_pEnemy[i]->PositiveXVelocity();
		if (m_pEnemy[i]->Position().x > 400)
			m_pEnemy[i]->NegativeXVelocity();
	}
	
	PlaneCollision();
	CrateCollision();
	HeartCollision();
	Spawn();
	Delete();


	__int64 m_Time = timeGetTime();
	if (pKeyBuffer[VK_SPACE] & 0xF0)
	{
		if (m_Time - m_BulletShootTime >= 200) //pentru a nu se trage gloante continuu
		{
			m_BulletShootTime = m_Time;
			m_pBullet.push_back(new Bullet(m_pBBuffer));
			m_pBullet.back()->Position() = Vec2(m_pPlayer->Position().x, m_pPlayer->Position().y - m_pPlayer->getHeight() /2); 

		}
	}
	




	


	for (int i = 0; i < m_pEnemyBullet.size(); i++)
	{
		if (m_pPlayer->GetShotEnemy(*m_pEnemyBullet[i]))
		{
			static UINT			fTimer;
			fTimer = SetTimer(m_hWnd, 1, 250, NULL);
			m_pPlayer->Explode();
			m_pPlayer->Position() = Vec2(200, 400);
			m_pPlayer->ResetVelocity();
			if (m_pPlayer->getLife() == 0)
			{
				MessageBox(m_hWnd, "Game ended", "Player 1 died", MB_OK);
				PostQuitMessage(0);
			}
		}
	}

	for (int i = 0; i < m_pBullet.size(); i++)
	{
		/*if (m_pPlayer->GetShot(*m_pBullet[i]))
		{

			m_pPlayer->Explode();

			m_pPlayer->Position() = Vec2(400, 400);
			if (m_pPlayer->getLife() == 0)
			{
				MessageBox(m_hWnd, "Game ended", "Player 1 died", MB_OK);
				PostQuitMessage(0);
			}
		}*/
		if (m_pPlayer2->GetShot(*m_pBullet[i]))
		{
			static UINT			fTimer;
			fTimer = SetTimer(m_hWnd, 2, 250, NULL);
			m_pPlayer2->Explode();
			m_pPlayer2->Position() = Vec2(400, 400);
			m_pPlayer2->ResetVelocity();
			if (m_pPlayer2->getLife() == 0)
			{
				MessageBox(m_hWnd, "Game ended", "Player 2 died", MB_OK);
				PostQuitMessage(0);
			}
		}
		
		for (int j = 0; j < m_pCrate.size(); j++)
			if (m_pCrate[j]->GetShot(*m_pBullet[i]))
			{
				static UINT			fTimer;
				fTimer = SetTimer(m_hWnd, 3, 250, NULL);
				m_pCrate[j]->Explode();
				m_pCrate[j]->AdvanceExplosion();
				delete m_pCrate[j];
				m_pPlayer->IncreaseScore();
				m_pCrate.erase(m_pCrate.begin() + j);
			}
		for (int j = 0; j < m_pEnemy.size(); j++)
			if (m_pEnemy[j]->GetShot(*m_pBullet[i]))
			{
				delete m_pEnemy[j];
				m_pPlayer->IncreaseScore();
				m_pEnemy.erase(m_pEnemy.begin() + j);
			}
	}



	// Now process the mouse (if the button is pressed)
	if ( GetCapture() == m_hWnd )
	{
		// Hide the mouse pointer
		SetCursor( NULL );

		// Retrieve the cursor position
		GetCursorPos( &CursorPos );

		// Reset our cursor position so we can keep going forever :)
		SetCursorPos( m_OldCursorPos.x, m_OldCursorPos.y );

	} // End if Captured
}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
	m_pPlayer->Update(m_Timer.GetTimeElapsed());
	m_pPlayer2->Update(m_Timer.GetTimeElapsed());
	for (int i = 0; i < m_pEnemy.size(); i++)
		m_pEnemy[i]->Update(m_Timer.GetTimeElapsed());
	for (int i = 0; i < m_pBullet.size(); i++)
		m_pBullet[i]->Update(m_Timer.GetTimeElapsed());
	for (int i = 0; i < m_pCrate.size(); i++)
		m_pCrate[i]->Update(m_Timer.GetTimeElapsed());
	for (int i = 0; i < m_pHeart.size(); i++)
		m_pHeart[i]->Update(m_Timer.GetTimeElapsed());
	for (int i = 0; i < m_pEnemyBullet.size(); i++)
		m_pEnemyBullet[i]->Update(m_Timer.GetTimeElapsed());
}

//-----------------------------------------------------------------------------
// Name : DrawObjects () (Private)
// Desc : Draws the game objects
//-----------------------------------------------------------------------------
void CGameApp::DrawObjects()
{

	m_pBBuffer->reset();

	//m_imgBackground.Paint(m_pBBuffer->getDC(), 0, 0);
	DrawBackground();

	m_pPlayer->Draw();

	m_pPlayer2->Draw();

	for (int i = 0; i < m_pEnemy.size(); i++)
		m_pEnemy[i]->Draw();
	for (int i = 0; i < m_pBullet.size(); i++)
		m_pBullet[i]->Draw();
	for (int i = 0; i < m_pCrate.size(); i++)
		m_pCrate[i]->Draw();
	for (int i = 0; i < m_pHeart.size(); i++)
		m_pHeart[i]->Draw();
	for (int i = 0; i < m_pEnemyBullet.size(); i++)
		m_pEnemyBullet[i]->Draw();

	m_pBBuffer->present();


}
void CGameApp::DrawBackground()
{
	static int currentY = m_imgBackground.Height();

	static size_t lastTime = ::GetTickCount();
	size_t currentTime = ::GetTickCount();

	if (currentTime - lastTime > 100) // Scrolling background
	{
		lastTime = currentTime;
		currentY -= 10;
		if (currentY < 0)
			currentY = m_imgBackground.Height();
	}

	m_imgBackground.Paint(m_pBBuffer->getDC(), 0, currentY);
	//m_imgBackground.Paint(m_pBBuffer->getDC(), currentY, 0);

}
