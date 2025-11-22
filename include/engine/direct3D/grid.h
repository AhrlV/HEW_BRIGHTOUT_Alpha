/*=======================================================================================================


     ƒOƒŠƒbƒh‚Ì•`‰æ[grid.cpp]

                                                                              Author :  Ryosuke Kageyama
                                                                              Date   :  2025/10/21
========================================================================================================*/


#ifndef GRID_H
#define GRID_H

#include<d3d11.h>
#include"camera.h"

void Grid_Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int x_count, int z_count, float grid_edge_x, float grid_edge_z);
void Grid_UnInit();
void Grid_Draw();

#endif
