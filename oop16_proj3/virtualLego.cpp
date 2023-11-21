////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <math.h>

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f} };
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;

public:
	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		m_pSphereMesh = NULL;
	}
	~CSphere(void) {}

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false;
		return true;
	}

	void destroy(void)
	{
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release();
			m_pSphereMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{
		// Insert your code here.
		double dist = sqrt(pow(center_x - ball.getPos_X(), 2) + pow(center_z - ball.getPos_Z(), 2));
		if (dist < 2 * getRadius() - 0.0) return true;
		else return false;
	}

	void hitBy(CSphere& ball)
	{
		if (hasIntersected(ball))
		{
			// Calculate relative velocity
			float relVelX = ball.getVelocity_X() - getVelocity_X();
			float relVelZ = ball.getVelocity_Z() - getVelocity_Z();
			// Calculate the normal vector at the collision point
			D3DXVECTOR3 normal = ball.getCenter() - getCenter();
			D3DXVec3Normalize(&normal, &normal);
			// Calculate impulse based on the normal and relative velocity
			float impulse = relVelX * normal.x + relVelZ * normal.z;
			// Update velocities
			setPower(getVelocity_X() + impulse * normal.x, getVelocity_Z() + impulse * normal.z);
			ball.setPower(ball.getVelocity_X() - impulse * normal.x, ball.getVelocity_Z() - impulse * normal.z);
		}
	}

	void ballUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if (vx > 0.0001 || vz > 0.0001)
		{
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;*/

			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0, 0); }
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
		if (rate < 0)
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
	}

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }
	double getPos_X() { return this->center_x; }
	double getPos_Y() { return this->center_y; }
	double getPos_Z() { return this->center_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getRadius(void)  const { return (float)(M_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;

};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_width = iwidth;
		m_depth = idepth;

		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{
		auto ballPos = ball.getCenter();
		if (abs(ballPos.x - m_x) < (m_width / 2) + ball.getRadius() && abs(ballPos.z - m_z) < (m_depth / 2) + ball.getRadius()) {
			return true;
		}
		else return false;
	}

	void hitBy(CSphere& ball)
	{
		if (hasIntersected(ball)) {
			if (m_width > m_depth) {												// 가로방향 벽일 때
				if (ball.getVelocity_Z() * (m_z - ball.getCenter().z) > 0) {		// 공이 벽을 향해 움직이고 있을 때만 충돌 판정
					ball.setPower(0.7*ball.getVelocity_X(), -0.7*ball.getVelocity_Z());		// 공의 속도의 z성분 부호 반전
				}
			}
			else {																	// 세로방향 벽일 때
				if (ball.getVelocity_X() * (m_x - ball.getCenter().x) > 0) {		// 공이 벽을 향해 움직이고 있을 때만 충돌 판정
					ball.setPower(-0.7*ball.getVelocity_X(), 0.7*ball.getVelocity_Z());		// 공의 속도의 x성분 부호 반전
				}
			}
		}
	}

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }



private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

// 공이 움직일 경로 표시
class CPath {
private:
	class CDot {
	private:
		float					center_x, center_y, center_z;
		float                   m_radius;
		float					m_velocity_x;
		float					m_velocity_z;

	public:
		CDot(void)
		{
			D3DXMatrixIdentity(&m_mLocal);
			ZeroMemory(&m_mtrl, sizeof(m_mtrl));
			m_radius = 0;
			m_velocity_x = 0;
			m_velocity_z = 0;
			m_pSphereMesh = NULL;
		}
		~CDot(void) {}

	public:
		bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
		{
			if (NULL == pDevice)
				return false;

			m_mtrl.Ambient = color;
			m_mtrl.Diffuse = color;
			m_mtrl.Specular = color;
			m_mtrl.Emissive = d3d::BLACK;
			m_mtrl.Power = 5.0f;

			if (FAILED(D3DXCreateSphere(pDevice, 0.05, 50, 50, &m_pSphereMesh, NULL)))
				return false;
			return true;
		}

		void destroy(void)
		{
			if (m_pSphereMesh != NULL) {
				m_pSphereMesh->Release();
				m_pSphereMesh = NULL;
			}
		}

		void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
		{
			if (NULL == pDevice)
				return;
			pDevice->SetTransform(D3DTS_WORLD, &mWorld);
			pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
			pDevice->SetMaterial(&m_mtrl);
			m_pSphereMesh->DrawSubset(0);
		}

		void setCenter(float x, float y, float z)
		{
			D3DXMATRIX m;
			center_x = x;	center_y = y;	center_z = z;
			D3DXMatrixTranslation(&m, x, y, z);
			setLocalTransform(m);
		}
		const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
		void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
		D3DXVECTOR3 getCenter(void) const
		{
			D3DXVECTOR3 org(center_x, center_y, center_z);
			return org;
		}

	private:
		D3DXMATRIX              m_mLocal;
		D3DMATERIAL9            m_mtrl;
		ID3DXMesh* m_pSphereMesh;

	};

private:
	CDot dots[60];
	const float width = 9;
	const float depth = 6.24f;
public:
	// 공 생성
	void create(IDirect3DDevice9* pDevice) {
		if (NULL == pDevice)
			return;
		for (int i = 0; i < 60; i++) {
			dots[i].create(pDevice);
		}
	}
	// startPos에서 endPos 방향으로 벽까지 0.2 간격으로 공을 그림
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld, D3DXVECTOR3 startPos, D3DXVECTOR3 endPos) {
		auto direction = endPos - startPos;
		float length = sqrt((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z));
		direction /= length;
		direction *= 0.2f;
		int i = 0;
		auto pos = startPos + direction;
		while (i < 60 && pos.x > -width / 2 && pos.x < width / 2 && pos.z > -depth / 2 && pos.z < depth / 2) {
			dots[i].setCenter(pos.x, pos.y, pos.z);
			dots[i].draw(pDevice, mWorld);
			pos += direction;
			i++;
		}
	}
};

// 당구채
class CStick {
private:
	float m_x;
	float m_y;
	float m_z;
	float m_length;
	float m_angle;
	float m_velocity_x;
	float m_velocity_z;
	bool isMoving;

public:
	CStick(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_pBoundMesh = NULL;
		isMoving = false;
	}
	~CStick(void) {}

	bool create(IDirect3DDevice9* pDevice, float radius1, float radius2, float length, D3DXCOLOR color = d3d::WHITE) {
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_length = length;
		// radius1이 radius2보다 크다면 두 값을 바꿈
		if (radius2 < radius1) {
			float temp = radius1;
			radius1 = radius2;
			radius2 = temp;
		}

		if (FAILED(D3DXCreateCylinder(pDevice, radius1, radius2, length, 8, 3, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}
	bool hasIntersected(CSphere& ball)
	{
		auto ballPos = ball.getCenter();
		float dist = (ballPos.x - m_x) * (ballPos.x - m_x);
		dist += (ballPos.z - m_z) * (ballPos.z - m_z);
		return dist < (m_length / 2) * (m_length / 2);
	}

	void hitBy(CSphere& ball)
	{
		// 공과 부딪히면 공을 움직이고 당구채는 멈춤
		if (hasIntersected(ball)) {
			ball.setPower(m_velocity_x, m_velocity_z);
			setPower(0, 0);
			isMoving = false;
		}
	}

	// 당구채의 위치, 각도 설정
	void setTransform(float x, float y, float z, float angle) {
		setRotation(angle);
		setPosition(x, y, z);
	}

	// 당구채 이동
	void stickUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if (vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			setTransform(tX, cord.y, tZ, m_angle);
		}
		else { this->setPower(0, 0); }
	}

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
		if (abs(vx) > 0.01 || abs(vz) > 0.01) {
			isMoving = true;
		}
	}
	D3DXVECTOR3 getCenter(void) const {
		D3DXVECTOR3 org(m_x, m_y, m_z);
		return org;
	}

	// 당구채가 움직이는 중이면 true
	bool isMove() const {
		return isMoving;
	}

	// startPos에서 endPos 방향으로 당구채의 각도와 위치 설정
	void setTarget(D3DXVECTOR3 startPos, D3DXVECTOR3 endPos) {
		auto direction = endPos - startPos;
		float length = sqrt((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z));
		float angle = acos(-direction.z / length);
		if (direction.x > 0) {
			angle = 2 * PI - angle;
		}
		direction /= -length;
		direction *= m_length * 0.5 + M_RADIUS + length * 0.5;
		this->setTransform(direction.x + startPos.x, direction.y + startPos.y, direction.z + startPos.z, angle);
	}

private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_y = y;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		m_mLocal *= m;
	}

	void setRotation(float angle) {
		m_angle = angle;
		D3DXMATRIX m;
		D3DXMatrixRotationY(&m, angle);
		setLocalTransform(m);
	}

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

class CText {
private:
	float m_x;
	float m_y;
	float m_z;
	float m_angle;
	float m_scale;

public:
	CText(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_pBoundMesh = NULL;
	}
	~CText(void) {}

	bool create(IDirect3DDevice9* pDevice, const char* text, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		// 폰트 설정
		HDC hdc = CreateCompatibleDC(0);
		HFONT hFont;
		HFONT hFontOld;
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = 25;
		lf.lfWidth = 12;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = 500;
		lf.lfItalic = false;
		lf.lfUnderline = false;
		lf.lfStrikeOut = false;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = 0;
		lf.lfClipPrecision = 0;
		lf.lfQuality = 0;
		lf.lfPitchAndFamily = 0;
		lf.lfFaceName, TEXT("맑은고딕");
		hFont = CreateFontIndirect(&lf);
		hFontOld = (HFONT)SelectObject(hdc, hFont);

		bool ret = FAILED(D3DXCreateText(pDevice, hdc, text, 0.01f, 0.2f, &m_pBoundMesh, NULL, NULL));
		SelectObject(hdc, hFontOld);
		DeleteObject(hFont);
		DeleteObject(hdc);
		return ret;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	// 텍스트 위치, 각도, 크기 설정
	void setTransform(float x, float y, float z, float angle, float scale) {
		D3DXMATRIX m;
		m_scale = scale;
		D3DXMatrixScaling(&m, scale, scale, scale);
		setLocalTransform(m);
		setRotation(angle);
		setPosition(x, y, z);
	}

private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_y = y;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		m_mLocal *= m;
	}

	void setRotation(float angle) {
		D3DXMATRIX m;
		D3DXMatrixRotationX(&m, angle);
		m_mLocal *= m;
	}

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};


// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CSphere	g_sphere[4];
CSphere	g_target_blueball;
CLight	g_light;

CPath path;				// 공이 움직일 경로
bool isTarget = false;	// 마우스 우클릭 여부
CStick stick;			// 당구채
CText text1;			// 플레이어1 텍스트
CText text2;			// 플레이어2 텍스트
CText scoreText1;		// 플레이어1 점수 텍스트
CText scoreText2;		// 점수 텍스트
int score1 = 50;		// 플레이어1 점수 저장, 50점에서 시작
int score2 = 50;		// 플레이어2 점수 저장, 50점에서 시작
bool isNewTurn = false;								// 게임의 턴이 새로 돌아왔는지 저장
bool isHit[4] = { false, false, false, false };		// 각 공의 충돌 여부 저장
bool isStopped[4] = { false, false, false, false };	// 각 공의 정지 여부 저장
int currentPlayer = 1; //처음 시작은 Player1(흰공)
int currentBall = 3; //시작 흰공(3), 다음 노란공(2)

double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f);

	// create four balls and set the position
	for (i = 0; i < 4; i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	g_target_blueball.setCenter(.0f, (float)M_RADIUS, .0f);

	// 경로 생성
	path.create(Device);
	// 당구채 생성
	stick.create(Device, 0.05f, 0.1f, 7);
	// 텍스트 생성
	text1.create(Device, "Score(Player1) : "); //플레이어1
	text1.setTransform(-2, 0.2f, 3.7f, PI / 2, 0.5f);
	text2.create(Device, "Score(Player2) : "); //플레이어2
	text2.setTransform(-2, 0.2f, 3.2f, PI / 2, 0.5f);

	// 점수 텍스트 생성
	scoreText1.create(Device, std::to_string(score1).c_str()); //플레이어1
	scoreText1.setTransform(2, 0.2f, 3.7f, PI / 2, 0.5f);
	scoreText2.create(Device, std::to_string(score2).c_str()); //플레이어2
	scoreText2.setTransform(2, 0.2f, 3.2f, PI / 2, 0.5f);

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;


	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		if (!isNewTurn && abs(g_sphere[currentBall].getVelocity_X()) > 0.01 && abs(g_sphere[currentBall].getVelocity_Z()) > 0.01) {
			if (currentPlayer == 1) {
				currentPlayer = 2;
				currentBall = 2; //노란 공
			} else {
				currentPlayer = 1;
				currentBall = 3; //흰 공
			}
			isNewTurn = true;
		}

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < 4; i++) {
			g_sphere[i].ballUpdate(timeDelta);
			for (j = 0; j < 4; j++) { g_legowall[i].hitBy(g_sphere[j]); }
		}

		// check whether any two balls hit together and update the direction of balls
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (i >= j) { continue; }
				g_sphere[i].hitBy(g_sphere[j]);
			}
		}

		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (i == j) { continue; }

				if (!isHit[i] && g_sphere[i].hasIntersected(g_sphere[j]))
					isHit[i] = true;
				if (abs(g_sphere[i].getVelocity_X()) < 0.01 && abs(g_sphere[i].getVelocity_Z()) < 0.01)
					isStopped[i] = true;
			}
		}

		if (isStopped[0] && isStopped[1] && isStopped[2] && isStopped[3]) {
			if(currentPlayer != 1)//하얀공이면(이전의 플레이어를 계산하는 방식)
			{	
				if (!isHit[0] && !isHit[1] && !isHit[2] && isNewTurn)
					score1 -= 10;				// 아무 공에도 맞지 않으면 점수 -10
				else if (isHit[2])
					score1 -= 10;				// 노란 공에 맞으면 점수 -10
				else {
					if (isHit[0] && isHit[1]) {
						score1 += 10;			// 빨간 공 2개에 연달아 맞으면 점수 +10
						if (score1 < 0) score1 = 0;
					}
					if ((isHit[0] && !isHit[1]) || (!isHit[0] && isHit[1])) {
						score1 += 0;				// 빨간 공 하나만 맞으면 점수 +0
					}
				}
			}
			else if(currentPlayer != 2){
				if (!isHit[0] && !isHit[1] && !isHit[3] && isNewTurn)
					score2 -= 10;				// 아무 공에도 맞지 않으면 점수 -10
				else if (isHit[3])
					score2 -= 10;				// 노란 공에 맞으면 점수 -10
				else {
					if (isHit[0] && isHit[1]) {
						score2 += 10;			// 빨간 공 2개에 연달아 맞으면 점수 +10
						if (score1 < 0) score1 = 0;
					}
					if ((isHit[0] && !isHit[1]) || (!isHit[0] && isHit[1])) {
						score2 += 0;				// 빨간 공 하나만 맞으면 점수 +0
					}
				}
			}
			isNewTurn = false;
			memset(isHit, false, 4);
			memset(isStopped, false, 4);
			
		}

		scoreText1.destroy();						// 기존 점수 텍스트 제거
		scoreText1.create(Device, std::to_string(score1).c_str()); // 점수 텍스트 업데이트
		scoreText2.destroy();						// 기존 점수 텍스트 제거
		scoreText2.create(Device, std::to_string(score2).c_str()); // 점수 텍스트 업데이트

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < 4; i++) {
			g_legowall[i].draw(Device, g_mWorld);
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_target_blueball.draw(Device, g_mWorld);
		g_light.draw(Device);

		if (stick.isMove()) {	// 당구채가 움직이는 중이라면
			stick.stickUpdate(timeDelta);	// 당구채 이동
			stick.hitBy(g_sphere[currentBall]);		// 흰 공과 충돌 검사
			stick.draw(Device, g_mWorld);	// 당구채 그리기
		}
		else if (isTarget) {		// 당구채가 움직이지 않고 마우스 우클릭 중이라면
			path.draw(Device, g_mWorld, g_sphere[currentBall].getCenter(), g_target_blueball.getCenter()); // 경로 그리기
			stick.setTarget(g_sphere[currentBall].getCenter(), g_target_blueball.getCenter());		// 흰 공과 파란 공에 맞춰 당구채 위치, 각도 설정
			stick.draw(Device, g_mWorld);				// 당구채 그리기
		}

		text1.draw(Device, g_mWorld);		// 텍스트 그리기
		text2.draw(Device, g_mWorld);		// 텍스트 그리기
		scoreText1.draw(Device, g_mWorld);	// 점수 텍스트 그리기
		scoreText2.draw(Device, g_mWorld);	// 점수 텍스트 그리기

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:
			// 마우스 우클릭 + 흰 공이 멈춰있을 때만
			if (isTarget && abs(g_sphere[currentBall].getVelocity_X()) < 0.01 && abs(g_sphere[currentBall].getVelocity_Z()) < 0.01) {
				isTarget = false; // 마우스 우클릭 해제

				D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
				D3DXVECTOR3	whitepos = g_sphere[currentBall].getCenter();
				double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
				if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { theta = PI + theta; } // 3 사분면
				double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				//g_sphere[3].setPower(distance * cos(theta), distance * sin(theta));

				stick.setPower(distance * cos(theta), distance * sin(theta));		// 당구채 움직임

				for (int i = 0; i < 4; i++) {		// 공을 칠 때마다 isHit 배열 초기화
					isHit[i] = false;
				}
				
			}
			break;

		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		if (LOWORD(wParam) & MK_LBUTTON) {

			isTarget = false;		// 마우스 우클릭 해제

			if (isReset) {
				isReset = false;
			}
			else {
				D3DXVECTOR3 vDist;
				D3DXVECTOR3 vTrans;
				D3DXMATRIX mTrans;
				D3DXMATRIX mX;
				D3DXMATRIX mY;

				switch (move) {
				case WORLD_MOVE:
					dx = (old_x - new_x) * 0.01f;
					dy = (old_y - new_y) * 0.01f;
					D3DXMatrixRotationY(&mX, dx);
					D3DXMatrixRotationX(&mY, dy);
					g_mWorld = g_mWorld * mX * mY;

					break;
				}
			}

			old_x = new_x;
			old_y = new_y;

		}
		else {
			isReset = true;

			isTarget = false;		// 마우스 우클릭 해제
			if (LOWORD(wParam) & MK_RBUTTON) {
				if (abs(g_sphere[currentBall].getVelocity_X()) < 0.01 && abs(g_sphere[currentBall].getVelocity_Z()) < 0.01 && !stick.isMove()) {
					isTarget = true;	// 흰 공과 당구채가 멈춰있을 때만 true
				}

				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;

				D3DXVECTOR3 coord3d = g_target_blueball.getCenter();
				g_target_blueball.setCenter(coord3d.x + dx * (-0.007f), coord3d.y, coord3d.z + dy * 0.007f);
			}
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}