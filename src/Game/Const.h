#pragma once
//--------------------------------------------
// 定数
//--------------------------------------------

// HP
const float HP_MAX_    = 20.0f;   //	HPの最大値(メモ：100.0f)
const int   COLOR_MAX_ = 255;
const int   FRAME_     = 120;   //	2秒間のフレーム

//	Enemy
const int ENEMY_MAX_ = 10;   //	敵の最大値

//	Item
const int ITEM_MAX_ = 3;   //	アイテムの最大数

//	フレーム数
int ITEM_FRAME_   = 60;    //	アイテムの使用時間
int STOP_TIME_    = 180;   //	時間を止められる時間
int COOL_TIME_    = 120;   //	クールタイム
int GAMETIME_ADD_ = 5;     //	敵を倒した時の追加時間
