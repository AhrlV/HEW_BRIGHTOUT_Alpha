/*=======================================================================================================


     カメラインタフェース [camera.h]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/28
========================================================================================================*/


#ifndef CAMERA_H
#define CAMERA_H

#include <directXMath.h>



class Camera
{
public:
	virtual ~Camera() = default;

	virtual void HandleInput() {}
	virtual void SetMatrix() const = 0;
};

/*
struct Camera
{

	DirectX::XMVECTOR eye;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR up;
	float FovAngleY;
	float NearZ;
	float FarZ;

	float width;
	float height;

	Camera()
		: eye({}), target({}), up({}), FovAngleY(0.0f), NearZ(0.0f), FarZ(0.0f), width(0.0f), height(0.0f)
	{}
};
*/




#endif
